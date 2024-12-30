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

        SwapchainHandle swapchain_create(SurfaceHandle surface_handle, uint32_t desired_framebuffer_count = 3) override;
        void swapchain_begin_frame(SwapchainHandle swapchain_handle) override;
        void swapchain_present(SwapchainHandle swapchain_handle) override;
        void swapchain_on_resize(SwapchainHandle swapchain_handle, uint32_t width, uint32_t height) override;
        void swapchain_destroy(SwapchainHandle swapchain_handle) override;

        BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) override;
        void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) override;
        void buffer_destroy(BufferHandle buffer_handle) override;

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

        struct SwapchainVulkan {
            SurfaceHandle surface_handle = nullptr;
            uint32_t framebuffer_count = 0;
            uint32_t queue_family_index = 0;
            uint32_t queue_index = 0;
            VkSwapchainKHR swapchain = nullptr;
            std::vector<VkImage> images = {};
            std::vector<VkImageView> image_views = {};
            VkRenderPass render_pass = nullptr;
            std::vector<VkFramebuffer> framebuffers = {};
        };

        void _add_queue_create_infos();
        void _initialize_device();
        void _request_extension(const std::string& name, bool is_required);
        void _create_swapchain();

        Ref<RenderContextVulkan> _context = nullptr;
        VkPhysicalDevice _physical_device = nullptr;
        std::vector<VkQueueFamilyProperties> _queue_families = {};
        std::vector<std::vector<Queue>> _queues = {};
        std::vector<VkDeviceQueueCreateInfo> _queue_create_infos = {};
        std::map<std::string, bool> _requested_extensions = {};
        std::vector<const char*> _enabled_extensions = {};
        VkDevice _device = nullptr;
        Functions _functions = {};
        SwapchainHandle _main_swapchain_handle = nullptr;
    };

}

#endif