#include <SDL3/SDL_vulkan.h>
#include <core/SkCanvas.h>
#include <core/SkColor.h>
#include <core/SkColorSpace.h>
#include <core/SkFont.h>
#include <core/SkFontMgr.h>
#include <core/SkRRect.h>
#include <core/SkSurface.h>
#include <core/SkTypeface.h>
#include <gpu/ganesh/GrBackendSurface.h>
#include <gpu/ganesh/GrDirectContext.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <gpu/ganesh/vk/GrVkBackendSemaphore.h>
#include <gpu/ganesh/vk/GrVkBackendSurface.h>
#include <gpu/ganesh/vk/GrVkDirectContext.h>
#include <gpu/ganesh/vk/GrVkTypes.h>
#include <gpu/vk/VulkanBackendContext.h>
#include <gpu/vk/VulkanExtensions.h>
#include <gpu/vk/VulkanMutableTextureState.h>
#include <ports/SkFontMgr_directory.h>
#include <toolkit/log.h>
#include <toolkit/render/renderer.h>

using namespace core;

namespace toolkit {

bool init_vulkan(Renderer::VulkanContext& ctx, const Window& window);
bool create_swapchain(Renderer::VulkanContext& ctx, uint32_t w, uint32_t h);
void destroy_vulkan(Renderer::VulkanContext& ctx);

Result<Unit, Error> Renderer::init(const Window& window) {
    if (!init_vulkan(m_ctx, window)) {
        return Error("Failed to initialize Vulkan");
    }

    skgpu::VulkanGetProc get_proc = [](const char* name,
                                       VkInstance inst,
                                       VkDevice dev) -> PFN_vkVoidFunction {
        if (dev)
            return vkGetDeviceProcAddr(dev, name);
        return vkGetInstanceProcAddr(inst, name);
    };

    skgpu::VulkanExtensions skia_vk_exts;
    skia_vk_exts.init(
        get_proc,
        m_ctx.instance,
        m_ctx.physical_device,
        static_cast<uint32_t>(m_ctx.enabled_instance_extensions.size()),
        m_ctx.enabled_instance_extensions.data(),
        static_cast<uint32_t>(m_ctx.enabled_device_extensions.size()),
        m_ctx.enabled_device_extensions.data()
    );

    skgpu::VulkanBackendContext vk_backend = {};
    vk_backend.fInstance = m_ctx.instance;
    vk_backend.fPhysicalDevice = m_ctx.physical_device;
    vk_backend.fDevice = m_ctx.device;
    vk_backend.fQueue = m_ctx.queue;
    vk_backend.fGraphicsQueueIndex = m_ctx.queue_family;
    vk_backend.fMaxAPIVersion = VK_API_VERSION_1_1;
    vk_backend.fVkExtensions = &skia_vk_exts;
    vk_backend.fDeviceFeatures2 = &m_ctx.enabled_features2;
    vk_backend.fGetProc = get_proc;

    m_gr_context = GrDirectContexts::MakeVulkan(vk_backend);

    if (!m_gr_context) {
        return Error("Failed to create Skia Vulkan GrDirectContext");
    }

    log::info(
        "Skia GrDirectContext created OK, backend={}",
        static_cast<int>(m_gr_context->backend())
    );

    log::info("Swapchain format: {}", static_cast<int>(m_ctx.swapchain_format));

    m_sk_color_type = (m_ctx.swapchain_format == VK_FORMAT_B8G8R8A8_UNORM
                       || m_ctx.swapchain_format == VK_FORMAT_B8G8R8A8_SRGB)
        ? kBGRA_8888_SkColorType
        : kRGBA_8888_SkColorType;

    return Unit {};
}

void Renderer::resize(int w, int h) {
    vkDeviceWaitIdle(m_ctx.device);

    if (!create_swapchain(
            m_ctx,
            static_cast<uint32_t>(w),
            static_cast<uint32_t>(h)
        )) {
        log::error("Failed to recreate swapchain during resize");
    }
}

struct DrawCommandVisitor {
    SkCanvas* canvas;

    void operator()(const DrawRectCommand& cmd) const {
        auto rect = SkRect::MakeXYWH(
            cmd.origin.x, cmd.origin.y, cmd.size.width, cmd.size.height
        );

        SkPaint fill_paint;
        fill_paint.setColor(SkColorSetARGB(
            static_cast<uint8_t>(cmd.color.a * 255),
            static_cast<uint8_t>(cmd.color.r * 255),
            static_cast<uint8_t>(cmd.color.g * 255),
            static_cast<uint8_t>(cmd.color.b * 255)
        ));
        fill_paint.setAntiAlias(cmd.anti_aliasing);

        if (cmd.corner_radius > 0.f) {
            SkRRect rrect;
            rrect.setRectXY(rect, cmd.corner_radius, cmd.corner_radius);
            canvas->drawRRect(rrect, fill_paint);

            if (cmd.border.has_border() && cmd.border.is_uniform()) {
                SkPaint stroke_paint;
                stroke_paint.setColor(SkColorSetARGB(
                    static_cast<uint8_t>(cmd.border_color.a * 255),
                    static_cast<uint8_t>(cmd.border_color.r * 255),
                    static_cast<uint8_t>(cmd.border_color.g * 255),
                    static_cast<uint8_t>(cmd.border_color.b * 255)
                ));
                stroke_paint.setAntiAlias(cmd.anti_aliasing);
                stroke_paint.setStyle(SkPaint::kStroke_Style);
                stroke_paint.setStrokeWidth(cmd.border.left);
                canvas->drawRRect(rrect, stroke_paint);
            }
        } else {
            canvas->drawRect(rect, fill_paint);

            if (cmd.border.has_border()) {
                draw_borders(cmd, rect);
            }
        }
    }

    void draw_borders(const DrawRectCommand& cmd, const SkRect& rect) const {
        SkPaint border_paint;
        border_paint.setColor(SkColorSetARGB(
            static_cast<uint8_t>(cmd.border_color.a * 255),
            static_cast<uint8_t>(cmd.border_color.r * 255),
            static_cast<uint8_t>(cmd.border_color.g * 255),
            static_cast<uint8_t>(cmd.border_color.b * 255)
        ));
        border_paint.setAntiAlias(cmd.anti_aliasing);

        if (cmd.border.is_uniform()) {
            border_paint.setStyle(SkPaint::kStroke_Style);
            border_paint.setStrokeWidth(cmd.border.left);
            canvas->drawRect(rect, border_paint);
            return;
        }

        // Draw each side as a filled rect
        float x = rect.fLeft;
        float y = rect.fTop;
        float w = rect.width();
        float h = rect.height();

        if (cmd.border.top > 0.f) {
            canvas->drawRect(
                SkRect::MakeXYWH(x, y, w, cmd.border.top), border_paint
            );
        }
        if (cmd.border.bottom > 0.f) {
            canvas->drawRect(
                SkRect::MakeXYWH(
                    x, y + h - cmd.border.bottom, w, cmd.border.bottom
                ),
                border_paint
            );
        }
        if (cmd.border.left > 0.f) {
            canvas->drawRect(
                SkRect::MakeXYWH(
                    x, y + cmd.border.top, cmd.border.left,
                    h - cmd.border.top - cmd.border.bottom
                ),
                border_paint
            );
        }
        if (cmd.border.right > 0.f) {
            canvas->drawRect(
                SkRect::MakeXYWH(
                    x + w - cmd.border.right, y + cmd.border.top,
                    cmd.border.right,
                    h - cmd.border.top - cmd.border.bottom
                ),
                border_paint
            );
        }
    }
};

void Renderer::render() {
    uint32_t image_index;
    VkResult acquire_result = vkAcquireNextImageKHR(
        m_ctx.device,
        m_ctx.swapchain,
        UINT64_MAX,
        m_ctx.image_available,
        VK_NULL_HANDLE,
        &image_index
    );
    if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR
        || acquire_result == VK_SUBOPTIMAL_KHR) {
        return;
    }
    if (acquire_result != VK_SUCCESS) {
        log::error(
            "vkAcquireNextImageKHR failed: {}",
            static_cast<int>(acquire_result)
        );
        return;
    }

    GrVkImageInfo image_info = {};
    image_info.fImage = m_ctx.swapchain_images[image_index];
    image_info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.fImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    image_info.fFormat = m_ctx.swapchain_format;
    image_info.fImageUsageFlags = m_ctx.swapchain_usage;
    image_info.fSampleCount = 1;
    image_info.fLevelCount = 1;
    image_info.fCurrentQueueFamily = m_ctx.queue_family;
    image_info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto backend_rt = GrBackendRenderTargets::MakeVk(
        static_cast<int>(m_ctx.width),
        static_cast<int>(m_ctx.height),
        image_info
    );

    auto surface = SkSurfaces::WrapBackendRenderTarget(
        m_gr_context.get(),
        backend_rt,
        kTopLeft_GrSurfaceOrigin,
        m_sk_color_type,
        SkColorSpace::MakeSRGB(),
        nullptr
    );

    if (!surface) {
        log::error("Failed to wrap swapchain image as Skia surface");
        return;
    }

    // Tell Skia to wait on the image-available semaphore before drawing
    GrBackendSemaphore wait_sem =
        GrBackendSemaphores::MakeVk(m_ctx.image_available);
    surface->wait(1, &wait_sem, false);

    SkCanvas* canvas = surface->getCanvas();

    // -- Draw --
    canvas->clear(SK_ColorDKGRAY);

    DrawCommandVisitor visitor {canvas};
    for (const auto& painter_ref : m_painters) {
        for (const auto& cmd : painter_ref.get().commands()) {
            std::visit(visitor, cmd);
        }
    }

    // Flush Skia — signal render_finished semaphore and transition to PRESENT_SRC
    GrBackendSemaphore signal_sem =
        GrBackendSemaphores::MakeVk(m_ctx.render_finished);
    auto present_state = skgpu::MutableTextureStates::MakeVulkan(
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        m_ctx.queue_family
    );

    GrFlushInfo flush_info = {};
    flush_info.fNumSemaphores = 1;
    flush_info.fSignalSemaphores = &signal_sem;

    m_gr_context->flush(surface.get(), flush_info, &present_state);
    m_gr_context->submit(GrSyncCpu::kNo);

    // Present
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_ctx.render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_ctx.swapchain;
    present_info.pImageIndices = &image_index;

    VkResult present_result = vkQueuePresentKHR(m_ctx.queue, &present_info);
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR
        || present_result == VK_SUBOPTIMAL_KHR) {
        // needs_resize = true;
    } else if (present_result != VK_SUCCESS) {
        log::error(
            "vkQueuePresentKHR failed: {}",
            static_cast<int>(present_result)
        );
    }

    m_painters.clear();
}

/**
 * Vulkan Free Functions
 */

bool init_vulkan(Renderer::VulkanContext& ctx, const Window& window) {
    // Instance
    Uint32 sdl_ext_count = 0;
    const char* const* sdl_exts =
        SDL_Vulkan_GetInstanceExtensions(&sdl_ext_count);
    if (!sdl_exts || sdl_ext_count == 0)
        return false;
    ctx.enabled_instance_extensions.assign(sdl_exts, sdl_exts + sdl_ext_count);

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "toki";
    app_info.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo inst_ci = {};
    inst_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_ci.pApplicationInfo = &app_info;
    inst_ci.enabledExtensionCount =
        static_cast<uint32_t>(ctx.enabled_instance_extensions.size());
    inst_ci.ppEnabledExtensionNames = ctx.enabled_instance_extensions.data();

    if (vkCreateInstance(&inst_ci, nullptr, &ctx.instance) != VK_SUCCESS)
        return false;

    SDL_Window* sdl_window = const_cast<SDL_Window*>(window.sdl_window());

    // Surface (required to pick a present-capable queue family)
    if (!SDL_Vulkan_CreateSurface(
            sdl_window,
            ctx.instance,
            nullptr,
            &ctx.surface
        ))
        return false;

    // Physical device + queue family
    uint32_t dev_count = 0;
    vkEnumeratePhysicalDevices(ctx.instance, &dev_count, nullptr);
    if (dev_count == 0)
        return false;
    std::vector<VkPhysicalDevice> devices(dev_count);
    vkEnumeratePhysicalDevices(ctx.instance, &dev_count, devices.data());

    bool found_device_and_queue = false;
    for (VkPhysicalDevice device : devices) {
        uint32_t qf_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &qf_count, nullptr);
        std::vector<VkQueueFamilyProperties> qf_props(qf_count);
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &qf_count,
            qf_props.data()
        );

        for (uint32_t i = 0; i < qf_count; i++) {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                device,
                i,
                ctx.surface,
                &present_support
            );
            if ((qf_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                && present_support) {
                ctx.physical_device = device;
                ctx.queue_family = i;
                found_device_and_queue = true;
                break;
            }
        }
        if (found_device_and_queue)
            break;
    }
    if (!found_device_and_queue)
        return false;

    // Logical device
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_ci = {};
    queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_ci.queueFamilyIndex = ctx.queue_family;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = &priority;

    ctx.enabled_device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo dev_ci = {};
    dev_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_ci.pNext = &ctx.enabled_features2;
    dev_ci.queueCreateInfoCount = 1;
    dev_ci.pQueueCreateInfos = &queue_ci;
    dev_ci.enabledExtensionCount =
        static_cast<uint32_t>(ctx.enabled_device_extensions.size());
    dev_ci.ppEnabledExtensionNames = ctx.enabled_device_extensions.data();

    if (vkCreateDevice(ctx.physical_device, &dev_ci, nullptr, &ctx.device)
        != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(ctx.device, ctx.queue_family, 0, &ctx.queue);

    // Pick swapchain format
    uint32_t fmt_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        ctx.physical_device,
        ctx.surface,
        &fmt_count,
        nullptr
    );
    std::vector<VkSurfaceFormatKHR> formats(fmt_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        ctx.physical_device,
        ctx.surface,
        &fmt_count,
        formats.data()
    );
    ctx.swapchain_format = formats[0].format;
    for (auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM) {
            ctx.swapchain_format = f.format;
            break;
        }
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB) {
            ctx.swapchain_format = f.format;
        }
    }

    // Swapchain
    int w, h;
    SDL_GetWindowSizeInPixels(sdl_window, &w, &h);
    if (!create_swapchain(
            ctx,
            static_cast<uint32_t>(w),
            static_cast<uint32_t>(h)
        ))
        return false;

    // Sync objects
    VkSemaphoreCreateInfo sem_ci = {};
    sem_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(ctx.device, &sem_ci, nullptr, &ctx.image_available);
    vkCreateSemaphore(ctx.device, &sem_ci, nullptr, &ctx.render_finished);

    return true;
}

bool create_swapchain(Renderer::VulkanContext& ctx, uint32_t w, uint32_t h) {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        ctx.physical_device,
        ctx.surface,
        &caps
    );

    VkExtent2D extent = {w, h};
    extent.width = std::max(
        caps.minImageExtent.width,
        std::min(caps.maxImageExtent.width, extent.width)
    );
    extent.height = std::max(
        caps.minImageExtent.height,
        std::min(caps.maxImageExtent.height, extent.height)
    );

    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
        image_count = caps.maxImageCount;

    VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (caps.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    VkSwapchainCreateInfoKHR ci = {};
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = ctx.surface;
    ci.minImageCount = image_count;
    ci.imageFormat = ctx.swapchain_format;
    ci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    ci.imageExtent = extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = usage;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform = caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    ci.clipped = VK_TRUE;
    ci.oldSwapchain = ctx.swapchain;

    VkSwapchainKHR new_swapchain;
    if (vkCreateSwapchainKHR(ctx.device, &ci, nullptr, &new_swapchain)
        != VK_SUCCESS)
        return false;

    if (ctx.swapchain)
        vkDestroySwapchainKHR(ctx.device, ctx.swapchain, nullptr);
    ctx.swapchain = new_swapchain;
    ctx.width = extent.width;
    ctx.height = extent.height;
    ctx.swapchain_usage = usage;

    uint32_t count = 0;
    vkGetSwapchainImagesKHR(ctx.device, ctx.swapchain, &count, nullptr);
    ctx.swapchain_images.resize(count);
    vkGetSwapchainImagesKHR(
        ctx.device,
        ctx.swapchain,
        &count,
        ctx.swapchain_images.data()
    );

    return true;
}

void destroy_vulkan(Renderer::VulkanContext& ctx) {
    if (ctx.device) {
        vkDeviceWaitIdle(ctx.device);
        vkDestroySemaphore(ctx.device, ctx.render_finished, nullptr);
        vkDestroySemaphore(ctx.device, ctx.image_available, nullptr);
        vkDestroySwapchainKHR(ctx.device, ctx.swapchain, nullptr);
        vkDestroyDevice(ctx.device, nullptr);
    }
    if (ctx.instance) {
        vkDestroySurfaceKHR(ctx.instance, ctx.surface, nullptr);
        vkDestroyInstance(ctx.instance, nullptr);
    }
}

}  // namespace toolkit
