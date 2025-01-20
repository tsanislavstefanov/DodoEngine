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
        enum class CommandQueueFamilyType {
            graphics,
            compute ,
            transfer
        };

        enum class CommandBufferType {
            primary,
            secondary
        };

        enum class SwapChainStatus {
            success,
            out_of_date,
            error
        };

        virtual ~RenderDevice() = default;

        virtual void initialize(size_t index) = 0;

        virtual CommandQueueFamilyHandle command_queue_family_get(CommandQueueFamilyType command_queue_family_type, SurfaceHandle surface) = 0;
        virtual CommandQueueHandle command_queue_create(CommandQueueFamilyHandle command_queue_family) = 0;
        virtual bool command_queue_execute_and_present(CommandQueueHandle command_queue, const std::vector<SemaphoreHandle>& wait_semaphores, const std::vector<CommandBufferHandle>& command_buffers, const std::vector<SemaphoreHandle>& command_semaphores, FenceHandle fence, SwapChainHandle swap_chain) = 0;
        virtual void command_queue_destroy(CommandQueueHandle command_queue) = 0;

        virtual CommandPoolHandle command_pool_create(CommandQueueFamilyHandle p_command_queue_family) = 0;
        virtual void command_pool_destroy(CommandPoolHandle p_command_pool) = 0;

        virtual CommandBufferHandle command_buffer_create(CommandPoolHandle p_command_pool, CommandBufferType p_command_buffer_type) = 0;
        virtual void command_buffer_begin(CommandBufferHandle p_command_buffer) = 0;
        virtual void command_buffer_end(CommandBufferHandle p_command_buffer) = 0;

        virtual FenceHandle fence_create() = 0;
        virtual void fence_wait(FenceHandle fence) = 0;
        virtual void fence_destroy(FenceHandle fence) = 0;

        virtual SemaphoreHandle semaphore_create() = 0;
        virtual void semaphore_destroy(SemaphoreHandle semaphore) = 0;

        virtual SwapChainHandle swap_chain_create(SurfaceHandle surface) = 0;
        virtual FramebufferHandle swap_chain_acquire_next_framebuffer(CommandQueueHandle command_queue, SwapChainHandle swap_chain, SwapChainStatus& swap_chain_status) = 0;
        virtual void swap_chain_resize(CommandQueueHandle command_queue, SwapChainHandle swap_chain, uint32_t desired_framebuffer_count = 3) = 0;
        virtual void swap_chain_destroy(SwapChainHandle swap_chain) = 0;
    };

}