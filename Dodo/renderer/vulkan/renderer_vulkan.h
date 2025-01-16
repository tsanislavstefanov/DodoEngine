#pragma once

#ifdef DODO_VULKAN

#include "vulkan_utils.h"
#include "renderer/renderer.h"
#include "renderer/render_handle_owner.h"

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
        CommandQueueFamilyHandle command_queue_family_get(CommandQueueFamilyType command_queue_family_type, SurfaceHandle surface) override;
        CommandQueueHandle command_queue_create(CommandQueueFamilyHandle command_queue_family) override;
        void command_queue_destroy(CommandQueueHandle command_queue) override;

    private:
        struct CommandQueueInfo {
            uint32_t queue_family_index = 0;
            uint32_t queue_index = 0;
        };

        RenderHandleOwner<CommandQueueHandle, CommandQueueInfo> _command_queue_owner = {};

    public:
        CommandPoolHandle command_pool_create(CommandQueueFamilyHandle command_queue_family) override;
        void command_pool_destroy(CommandPoolHandle command_pool) override;

    private:
        struct CommandPoolInfo {
            VkCommandPool vk_command_pool = VK_NULL_HANDLE;
        };

        RenderHandleOwner<CommandPoolHandle, CommandPoolInfo> _command_pool_owner = {};

    public:
        CommandBufferHandle command_buffer_create(CommandPoolHandle command_pool, CommandBufferType command_buffer_type) override;
        void command_buffer_begin(CommandBufferHandle command_buffer) override;
        void command_buffer_end(CommandBufferHandle command_buffer) override;

    private:
        struct CommandBufferInfo {
            CommandBufferType command_buffer_type = COMMAND_BUFFER_TYPE_PRIMARY;
            VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
        };

        RenderHandleOwner<CommandBufferHandle, CommandBufferInfo> _command_buffer_owner = {};

    public:
        FenceHandle fence_create() override;
        void fence_wait(FenceHandle fence) override;
        void fence_destroy(FenceHandle fence) override;

    private:
        struct FenceInfo {
            VkFence vk_fence = VK_NULL_HANDLE;
        };

        RenderHandleOwner<FenceHandle, FenceInfo> _fence_owner = {};

    public:
        SemaphoreHandle semaphore_create() override;
        void semaphore_destroy(SemaphoreHandle semaphore) override;

    private:
        struct SemaphoreInfo {
            VkSemaphore vk_semaphore = VK_NULL_HANDLE;
        };

        RenderHandleOwner<SemaphoreHandle, SemaphoreInfo> _semaphore_owner = {};

    public:
        SwapChainHandle swap_chain_create(SurfaceHandle surface) override;
        FramebufferHandle swap_chain_acquire_next_framebuffer(CommandQueueHandle command_queue, SwapChainHandle swap_chain, SwapChainStatus& swap_chain_status) override;
        void swap_chain_resize(CommandQueueHandle command_queue, SwapChainHandle swap_chain, uint32_t desired_framebuffer_count = 3) override;
        void swap_chain_destroy(SwapChainHandle swap_chain) override;

    private:
        struct SwapChainInfo {
            SurfaceHandle surface = {};
            VkFormat vk_format = VK_FORMAT_UNDEFINED;
            VkColorSpaceKHR vk_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            VkRenderPass vk_render_pass = nullptr;
            VkSwapchainKHR vk_swap_chain = nullptr;
            std::vector<VkImage> vk_images = {};
            std::vector<VkImageView> vk_image_views = {};
            std::vector<VkFramebuffer> framebuffers = {};
            uint32_t image_index = 0;
        };

        void _swap_chain_release(SwapChainInfo* swap_chain_info) const;
        FramebufferHandle _framebuffer_create();

    public:
        BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) override;
        void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) override;
        void buffer_destroy(BufferHandle buffer_handle) override;

    private:
    };

}

#endif
