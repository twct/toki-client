#pragma once

#include <core/SkColor.h>
#include <core/error.h>
#include <core/result.h>
#include <gpu/ganesh/GrDirectContext.h>
#include <toolkit/window.h>
#include <vulkan/vulkan.h>

#include <vector>

namespace toolkit {

class Renderer {
  public:
    core::Result<core::Unit, core::Error> init(const Window& window);

    void resize(int width, int height);
    void render();

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

  private:
    VulkanContext m_ctx;
    SkColorType m_sk_color_type;
    sk_sp<GrDirectContext> m_gr_context;
};

}  // namespace toolkit