#pragma once

#ifdef DODO_VULKAN

#include "vulkan_utils.h"
#include "renderer/renderer.h"

namespace Dodo {

    class RenderContextVulkan;

    class RendererVulkan : public Renderer
    {
    public:
        RendererVulkan(Ref<RenderContextVulkan> context);

        void initialize(size_t device_index) override;

    private:
        struct Queue {
            VkQueue queue = nullptr;
            uint32_t virtual_count = 0;
        };

        struct Functions {
            PFN_vkCreateSwapchainKHR CreateSwapchainKHR = nullptr;
            PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = nullptr;
            PFN_vkAcquireNextImageKHR AcquireNextImageKHR = nullptr;
            PFN_vkQueuePresentKHR QueuePresentKHR = nullptr;
            PFN_vkDestroySwapchainKHR DestroySwapchainKHR = nullptr;
        };

        void _add_queue_create_infos(std::vector<VkDeviceQueueCreateInfo>& queue_create_infos);
        void _initialize_device(std::vector<VkDeviceQueueCreateInfo>& queue_create_infos);
        void _request_extension(const std::string& name, bool is_required);
        void _initialize_main_queue();

        Ref<RenderContextVulkan> _context = nullptr;
        VkPhysicalDevice _physical_device = nullptr;
        std::vector<VkQueueFamilyProperties> _queue_families = {};
        std::map<std::string, bool> _requested_extensions = {};
        std::vector<const char*> _enabled_extensions = {};
        VkDevice _device = nullptr;
        std::vector<std::vector<Queue>> _queues = {};
        Functions _functions = {};
        uint32_t _main_queue_family_index = 0;
        uint32_t _main_queue_index = 0;

    public:
        SwapchainHandle swapchain_create(SurfaceHandle surface_handle, uint32_t desired_frame_count = 3) override;
        void swapchain_begin_frame(SwapchainHandle swapchain_handle) override;
        void swapchain_present(SwapchainHandle swapchain_handle) override;
        void swapchain_on_resize(SwapchainHandle swapchain_handle, uint32_t width, uint32_t height) override;
        void swapchain_destroy(SwapchainHandle swapchain_handle) override;

    private:
        struct Swapchain {
            SurfaceHandle surface_handle = nullptr;
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            VkSwapchainKHR swapchain = nullptr;
            std::vector<VkImage> images = {};
            std::vector<VkImageView> image_views = {};
            VkRenderPass render_pass = nullptr;
            std::vector<VkFramebuffer> framebuffers = {};
            uint32_t present_queue_family_index = 0;
            uint32_t present_queue_index = 0;
        };

        void _swapchain_invalidate(SwapchainHandle swapchain_handle, uint32_t desired_frame_count);
        void _swapchain_select_format_and_color_space(SwapchainHandle swapchain_handle, VkFormat desired_format = VK_FORMAT_B8G8R8A8_SRGB, VkColorSpaceKHR desired_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
        void _swapchain_release(SwapchainHandle swapchain_handle);

    public:
        BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) override;
        void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) override;
        void buffer_destroy(BufferHandle buffer_handle) override;

    private:
    };

}

#endif
