#pragma once

#ifdef DODO_VULKAN

#include "vulkan_utils.h"
#include "renderer/render_device.h"

namespace Dodo {

    class RenderBackendVulkan;

    class RenderDeviceVulkan : public RenderDevice
    {
    public:
        RenderDeviceVulkan(Ref<RenderBackendVulkan> p_backend);

        void initialize(size_t index) override;

    private:
        struct Functions {
            PFN_vkCreateSwapchainKHR CreateSwapchainKHR = nullptr;
            PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = nullptr;
            PFN_vkAcquireNextImageKHR AcquireNextImageKHR = nullptr;
            PFN_vkQueuePresentKHR QueuePresentKHR = nullptr;
            PFN_vkDestroySwapchainKHR DestroySwapchainKHR = nullptr;
        };

        struct Queue {
            uint32_t queue_family_index = 0;
            uint32_t queue_index = 0;
            VkQueue queue = VK_NULL_HANDLE;
            uint32_t virtual_count = 0;
        };

        void _add_queue_create_infos(std::vector<VkDeviceQueueCreateInfo>& queue_create_infos);
        void _initialize_device(std::vector<VkDeviceQueueCreateInfo>& queue_create_infos);
        void _request_extension(const std::string& name, bool is_required);

        Ref<RenderBackendVulkan> _backend = nullptr;
        VkPhysicalDevice _physical_device = nullptr;
        std::vector<VkQueueFamilyProperties> _queue_families = {};
        std::map<std::string, bool> _requested_extensions = {};
        std::vector<const char*> _enabled_extensions = {};
        VkDevice _device = nullptr;
        std::vector<std::vector<Queue>> _queues = {};
        Functions _functions = {};

    public:
        // ---- COMMAND QUEUE ----
        CommandQueueFamilyHandle command_queue_family_get(CommandQueueFamilyType command_queue_family_type, SurfaceHandle surface) override;
        CommandQueueHandle command_queue_create(CommandQueueFamilyHandle command_queue_family) override;
        bool command_queue_execute_and_present(CommandQueueHandle p_command_queue, const std::vector<SemaphoreHandle>& p_wait_semaphores, const std::vector<CommandBufferHandle>& p_command_buffers, const std::vector<SemaphoreHandle>& p_signal_semaphores, FenceHandle p_fence, SwapChainHandle p_swap_chain) override;
        void command_queue_destroy(CommandQueueHandle command_queue) override;

    private:
        struct Fence;
        struct CommandQueue {
            uint32_t queue_family_index = 0;
            uint32_t queue_index = 0;
            std::vector<VkSemaphore> image_semaphores = {};
            std::vector<uint32_t> free_image_semaphores = {};
            std::vector<uint32_t> pending_image_semaphores = {};
            std::vector<uint32_t> pending_image_semaphores_for_fences = {};
            std::vector<std::pair<Fence*, uint32_t>> image_semaphores_for_fences = {};
            std::vector<VkSemaphore> present_semaphores = {};
            uint32_t present_semaphore_index = 0;
        };

        RenderHandlePool<CommandQueueHandle, CommandQueue> _command_queue_pool = {};

    public:
        // ---- COMMAND POOL ----
        CommandPoolHandle command_pool_create(CommandQueueFamilyHandle command_queue_family) override;
        void command_pool_destroy(CommandPoolHandle command_pool) override;

    private:
        RenderHandlePool<CommandPoolHandle, VkCommandPool> _command_pool_owner = {};

    public:
        // ---- COMMAND BUFFER ----
        CommandBufferHandle command_buffer_create(CommandPoolHandle command_pool, CommandBufferType command_buffer_type) override;
        void command_buffer_begin(CommandBufferHandle command_buffer) override;
        void command_buffer_end(CommandBufferHandle command_buffer) override;

    private:
        struct CommandBuffer {
            CommandBufferType command_buffer_type = CommandBufferType::primary;
            VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
        };

        RenderHandlePool<CommandBufferHandle, CommandBuffer> _command_buffer_pool = {};

    public:
        // ---- FENCE ----
        FenceHandle fence_create() override;
        void fence_wait(FenceHandle fence) override;
        void fence_destroy(FenceHandle fence) override;

    private:
        struct Fence {
            VkFence vk_fence = VK_NULL_HANDLE;
            CommandQueue* command_queue_to_signal = nullptr;
        };

        RenderHandlePool<FenceHandle, Fence> _fence_owner = {};

    public:
        // ---- SEMAPHORE ----
        SemaphoreHandle semaphore_create() override;
        void semaphore_destroy(SemaphoreHandle semaphore) override;

    private:
        RenderHandlePool<SemaphoreHandle, VkSemaphore> _semaphore_owner = {};

    public:
        // ---- SWAP CHAIN ----
        SwapChainHandle swap_chain_create(SurfaceHandle p_surface) override;
        FramebufferHandle swap_chain_acquire_next_framebuffer(CommandQueueHandle p_command_queue, SwapChainHandle p_swap_chain, SwapChainStatus& r_swap_chain_status) override;
        void swap_chain_resize(CommandQueueHandle p_command_queue, SwapChainHandle p_swap_chain, uint32_t p_desired_framebuffer_count = 3) override;
        void swap_chain_destroy(SwapChainHandle p_swap_chain) override;

    private:
        struct SwapChain {
            SurfaceHandle surface = {};
            VkFormat format = VK_FORMAT_UNDEFINED;
            VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            VkRenderPass render_pass = nullptr;
            uint32_t framebuffer_count = 0;
            VkSwapchainKHR vk_swap_chain = nullptr;
            std::vector<VkImage> images = {};
            std::vector<VkImageView> image_views = {};
            std::vector<VkFramebuffer> framebuffers = {};
            uint32_t image_index = 0;
        };

        void _swap_chain_release(SwapChain* r_swap_chain) const;

        RenderHandlePool<SwapChainHandle, SwapChain> _swap_chain_owner = {};
    };

}

#endif
