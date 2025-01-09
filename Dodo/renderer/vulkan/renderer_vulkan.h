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

        Ref<RenderContextVulkan> _context = nullptr;
        VkPhysicalDevice _physical_device = nullptr;
        std::vector<VkQueueFamilyProperties> _queue_families = {};
        std::map<std::string, bool> _requested_extensions = {};
        std::vector<const char*> _enabled_extensions = {};
        VkDevice _device = nullptr;
        std::vector<std::vector<VkQueue>> _queues = {};
        Functions _functions = {};

    public:
        CommandQueueFamilyHandle command_queue_family_get(const std::bitset<CommandQueueFamilyBits::count>& cmd_queue_family_bits, SurfaceHandle surface_handle) override;
        CommandQueueHandle command_queue_create(CommandQueueFamilyHandle cmd_queue_family_handle) override;
        void command_queue_destroy(CommandQueueHandle cmd_queue_handle) override;

    private:
        struct CommandQueue {
            uint32_t queue_family_index = 0;
            uint32_t queue_index = 0;

        };

    public:
        SwapChainHandle swap_chain_create(SurfaceHandle surface_handle) override;
        void swap_chain_resize(CommandQueueHandle cmd_queue_handle, SwapChainHandle swap_chain_handle, uint32_t desired_framebuffer_count = 3) override;

        void swapchain_begin_frame(SwapChainHandle swap_chain_handle) override;
        void swapchain_present(SwapChainHandle swap_chain_handle) override;
        void swapchain_on_resize(SwapChainHandle swap_chain_handle, uint32_t width, uint32_t height) override;
        void swap_chain_destroy(SwapChainHandle swap_chain_handle) override;

    private:
        struct SwapChain {
            SurfaceHandle surface_handle = nullptr;
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            VkSwapchainKHR swap_chain_vk = nullptr;
            std::vector<VkImage> images = {};
            std::vector<VkImageView> image_views = {};
            VkRenderPass render_pass = nullptr;
            std::vector<VkFramebuffer> framebuffers = {};
            std::vector<CommandQueue*> cmd_queues = {};
        };

        void _swap_chain_release(SwapChain* swap_chain);

    public:
        BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) override;
        void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) override;
        void buffer_destroy(BufferHandle buffer_handle) override;

    private:
    };

}

#endif
