#include <SDL3/SDL.h>
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
#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

static constexpr int kWidth = 800;
static constexpr int kHeight = 600;

// All Vulkan state needed for rendering
struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t queue_family = 0;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchain_format = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags swapchain_usage = 0;
    std::vector<VkImage> swapchain_images;
    VkSemaphore image_available = VK_NULL_HANDLE;
    VkSemaphore render_finished = VK_NULL_HANDLE;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<const char*> enabled_instance_extensions;
    std::vector<const char*> enabled_device_extensions;
    VkPhysicalDeviceFeatures2 enabled_features2 = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
    };
};

static bool create_swapchain(VulkanContext& ctx, uint32_t w, uint32_t h) {
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

static bool init_vulkan(VulkanContext& ctx, SDL_Window* window) {
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

    // Surface (required to pick a present-capable queue family)
    if (!SDL_Vulkan_CreateSurface(window, ctx.instance, nullptr, &ctx.surface))
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
    SDL_GetWindowSizeInPixels(window, &w, &h);
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

static void destroy_vulkan(VulkanContext& ctx) {
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

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "toki",
        kWidth,
        kHeight,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return 1;
    }

    VulkanContext vk;
    if (!init_vulkan(vk, window)) {
        SDL_Log("Failed to initialize Vulkan");
        return 1;
    }

    // Skia Vulkan context
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
        vk.instance,
        vk.physical_device,
        static_cast<uint32_t>(vk.enabled_instance_extensions.size()),
        vk.enabled_instance_extensions.data(),
        static_cast<uint32_t>(vk.enabled_device_extensions.size()),
        vk.enabled_device_extensions.data()
    );

    skgpu::VulkanBackendContext vk_backend = {};
    vk_backend.fInstance = vk.instance;
    vk_backend.fPhysicalDevice = vk.physical_device;
    vk_backend.fDevice = vk.device;
    vk_backend.fQueue = vk.queue;
    vk_backend.fGraphicsQueueIndex = vk.queue_family;
    vk_backend.fMaxAPIVersion = VK_API_VERSION_1_1;
    vk_backend.fVkExtensions = &skia_vk_exts;
    vk_backend.fDeviceFeatures2 = &vk.enabled_features2;
    vk_backend.fGetProc = get_proc;

    auto gr_context = GrDirectContexts::MakeVulkan(vk_backend);
    if (!gr_context) {
        SDL_Log("Failed to create Skia Vulkan GrDirectContext");
        return 1;
    }
    SDL_Log(
        "Skia GrDirectContext created OK, backend=%d",
        gr_context->backend()
    );
    SDL_Log("Swapchain format: %d", vk.swapchain_format);

    // Load a system font
    auto font_mgr = SkFontMgr_New_Custom_Directory("/usr/share/fonts/TTF/");
    sk_sp<SkTypeface> typeface =
        font_mgr->makeFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf");
    if (!typeface) {
        SDL_Log("Failed to load typeface from file");
        return 1;
    }
    SkFont font(typeface, 36);

    SkColorType sk_color_type =
        (vk.swapchain_format == VK_FORMAT_B8G8R8A8_UNORM
         || vk.swapchain_format == VK_FORMAT_B8G8R8A8_SRGB)
        ? kBGRA_8888_SkColorType
        : kRGBA_8888_SkColorType;

    bool running = true;
    bool needs_resize = false;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
            if (event.type == SDL_EVENT_KEY_DOWN
                && event.key.key == SDLK_ESCAPE)
                running = false;
            if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                needs_resize = true;
            }
        }

        if (needs_resize) {
            int w, h;
            SDL_GetWindowSizeInPixels(window, &w, &h);
            if (w > 0 && h > 0) {
                vkDeviceWaitIdle(vk.device);
                if (!create_swapchain(
                        vk,
                        static_cast<uint32_t>(w),
                        static_cast<uint32_t>(h)
                    )) {
                    SDL_Log("Failed to recreate swapchain during resize");
                    break;
                }
                needs_resize = false;
            } else {
                SDL_Delay(10);
                continue;
            }
        }

        // Acquire swapchain image
        uint32_t image_index;
        VkResult acquire_result = vkAcquireNextImageKHR(
            vk.device,
            vk.swapchain,
            UINT64_MAX,
            vk.image_available,
            VK_NULL_HANDLE,
            &image_index
        );
        if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR
            || acquire_result == VK_SUBOPTIMAL_KHR) {
            int w, h;
            SDL_GetWindowSizeInPixels(window, &w, &h);
            if (w > 0 && h > 0) {
                vkDeviceWaitIdle(vk.device);
                if (!create_swapchain(
                        vk,
                        static_cast<uint32_t>(w),
                        static_cast<uint32_t>(h)
                    )) {
                    SDL_Log("Failed to recreate swapchain after acquire");
                    break;
                }
            }
            needs_resize = false;
            continue;
        }
        if (acquire_result != VK_SUCCESS) {
            SDL_Log("vkAcquireNextImageKHR failed: %d", acquire_result);
            break;
        }

        GrVkImageInfo image_info = {};
        image_info.fImage = vk.swapchain_images[image_index];
        image_info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.fImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        image_info.fFormat = vk.swapchain_format;
        image_info.fImageUsageFlags = vk.swapchain_usage;
        image_info.fSampleCount = 1;
        image_info.fLevelCount = 1;
        image_info.fCurrentQueueFamily = vk.queue_family;
        image_info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto backend_rt = GrBackendRenderTargets::MakeVk(
            static_cast<int>(vk.width),
            static_cast<int>(vk.height),
            image_info
        );

        auto surface = SkSurfaces::WrapBackendRenderTarget(
            gr_context.get(),
            backend_rt,
            kTopLeft_GrSurfaceOrigin,
            sk_color_type,
            SkColorSpace::MakeSRGB(),
            nullptr
        );

        if (!surface) {
            SDL_Log("Failed to wrap swapchain image as Skia surface");
            break;
        }

        // Tell Skia to wait on the image-available semaphore before drawing
        GrBackendSemaphore wait_sem =
            GrBackendSemaphores::MakeVk(vk.image_available);
        surface->wait(1, &wait_sem, false);

        SkCanvas* canvas = surface->getCanvas();

        // -- Draw --
        canvas->clear(SK_ColorDKGRAY);

        SkPaint rect_paint;
        rect_paint.setColor(SkColorSetRGB(0x42, 0xA5, 0xF5));
        rect_paint.setAntiAlias(true);
        SkRRect rrect;
        rrect.setRectXY(SkRect::MakeXYWH(100, 100, 300, 200), 20, 20);
        canvas->drawRRect(rrect, rect_paint);

        SkPaint circle_paint;
        circle_paint.setColor(SkColorSetRGB(0xEF, 0x53, 0x50));
        circle_paint.setAntiAlias(true);
        canvas->drawCircle(550, 300, 80, circle_paint);

        SkPaint text_paint;
        text_paint.setColor(SK_ColorWHITE);
        text_paint.setAntiAlias(true);
        canvas->drawString("Hello, Skia!", 250, 450, font, text_paint);

        // Flush Skia — signal render_finished semaphore and transition to PRESENT_SRC
        GrBackendSemaphore signal_sem =
            GrBackendSemaphores::MakeVk(vk.render_finished);
        auto present_state = skgpu::MutableTextureStates::MakeVulkan(
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            vk.queue_family
        );

        GrFlushInfo flush_info = {};
        flush_info.fNumSemaphores = 1;
        flush_info.fSignalSemaphores = &signal_sem;

        gr_context->flush(surface.get(), flush_info, &present_state);
        gr_context->submit(GrSyncCpu::kNo);

        // Present
        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &vk.render_finished;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &vk.swapchain;
        present_info.pImageIndices = &image_index;

        VkResult present_result = vkQueuePresentKHR(vk.queue, &present_info);
        if (present_result == VK_ERROR_OUT_OF_DATE_KHR
            || present_result == VK_SUBOPTIMAL_KHR) {
            needs_resize = true;
        } else if (present_result != VK_SUCCESS) {
            SDL_Log("vkQueuePresentKHR failed: %d", present_result);
            break;
        }
    }

    gr_context->abandonContext();
    destroy_vulkan(vk);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
