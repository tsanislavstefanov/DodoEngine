#pragma once

#include "render_backend.h"
#include "render_handle.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(CommandQueueFamily);
    DODO_DEFINE_RENDER_HANDLE(CommandQueue);
    DODO_DEFINE_RENDER_HANDLE(CommandPool);
    DODO_DEFINE_RENDER_HANDLE(CommandBuffer);
    DODO_DEFINE_RENDER_HANDLE(Fence);
    DODO_DEFINE_RENDER_HANDLE(Semaphore);
    DODO_DEFINE_RENDER_HANDLE(SwapChain);
    DODO_DEFINE_RENDER_HANDLE(Framebuffer);
    DODO_DEFINE_RENDER_HANDLE(Buffer);

    class RenderDevice : public RefCounted {
    public:
        virtual ~RenderDevice() = default;

        virtual void initialize(size_t p_index) = 0;

        enum class CommandQueueFamilyType {
            graphics,
            compute ,
            transfer
        };

        virtual CommandQueueFamilyHandle command_queue_family_get(CommandQueueFamilyType p_command_queue_family_type, SurfaceHandle p_surface) = 0;
        virtual CommandQueueHandle command_queue_create(CommandQueueFamilyHandle p_command_queue_family) = 0;
        virtual bool command_queue_execute_and_present(CommandQueueHandle p_command_queue, const std::vector<SemaphoreHandle>& p_wait_semaphores, const std::vector<CommandBufferHandle>& p_command_buffers, const std::vector<SemaphoreHandle>& p_signal_semaphores, FenceHandle p_fence, SwapChainHandle p_swap_chain) = 0;
        virtual void command_queue_destroy(CommandQueueHandle p_command_queue) = 0;

        virtual CommandPoolHandle command_pool_create(CommandQueueFamilyHandle p_command_queue_family) = 0;
        virtual void command_pool_destroy(CommandPoolHandle p_command_pool) = 0;

        enum class CommandBufferType {
            primary,
            secondary
        };

        virtual CommandBufferHandle command_buffer_create(CommandPoolHandle p_command_pool, CommandBufferType p_command_buffer_type) = 0;
        virtual void command_buffer_begin(CommandBufferHandle p_command_buffer) = 0;
        virtual void command_buffer_end(CommandBufferHandle p_command_buffer) = 0;

        virtual FenceHandle fence_create() = 0;
        virtual void fence_wait(FenceHandle p_fence) = 0;
        virtual void fence_destroy(FenceHandle p_fence) = 0;

        virtual SemaphoreHandle semaphore_create() = 0;
        virtual void semaphore_destroy(SemaphoreHandle p_semaphore) = 0;

        enum class SwapChainStatus {
            success,
            out_of_date,
            error
        };

        virtual SwapChainHandle swap_chain_create(SurfaceHandle p_surface) = 0;
        virtual FramebufferHandle swap_chain_acquire_next_framebuffer(CommandQueueHandle p_command_queue, SwapChainHandle p_swap_chain, SwapChainStatus& r_swap_chain_status) = 0;
        virtual void swap_chain_resize(CommandQueueHandle p_command_queue, SwapChainHandle p_swap_chain, uint32_t p_desired_framebuffer_count = 3) = 0;
        virtual void swap_chain_destroy(SwapChainHandle p_swap_chain) = 0;
    };

}
