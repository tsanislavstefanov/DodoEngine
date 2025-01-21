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
            draw,
            compute,
            copy
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

        struct SubmitSpecifications {
            CommandQueueHandle command_queue = {};
            std::vector<CommandBufferHandle> command_buffers = {};
            SwapChainHandle swap_chain = {};
        };

        RenderDevice() = default;
        virtual ~RenderDevice() = default;

        virtual void initialize(size_t index) = 0;
        virtual CommandQueueFamilyHandle command_queue_family_get(CommandQueueFamilyType command_queue_family_type, SurfaceHandle surface) = 0;
        virtual CommandQueueHandle command_queue_create(CommandQueueFamilyHandle command_queue_family) = 0;
        virtual void command_queue_execute_and_present(const SubmitSpecifications& submit_specs) = 0;
        virtual void command_queue_destroy(CommandQueueHandle command_queue) = 0;
        virtual CommandPoolHandle command_pool_create(CommandQueueFamilyHandle command_queue_family) = 0;
        virtual void command_pool_destroy(CommandPoolHandle command_pool) = 0;
        virtual CommandBufferHandle command_buffer_create(CommandPoolHandle command_pool, CommandBufferType command_buffer_type) = 0;
        virtual void command_buffer_begin(CommandBufferHandle command_buffer) = 0;
        virtual void command_buffer_end(CommandBufferHandle command_buffer) = 0;
        virtual FenceHandle fence_create() = 0;
        virtual void fence_wait(FenceHandle fence) = 0;
        virtual void fence_destroy(FenceHandle fence) = 0;
        virtual SemaphoreHandle semaphore_create() = 0;
        virtual void semaphore_destroy(SemaphoreHandle semaphore) = 0;
        virtual SwapChainHandle swap_chain_create(SurfaceHandle surface) = 0;
        virtual FramebufferHandle swap_chain_acquire_next_framebuffer(CommandQueueHandle command_queue, SwapChainHandle swap_chain, SwapChainStatus& swap_chain_status) = 0;
        virtual void swap_chain_recreate_or_resize(CommandQueueHandle command_queue, SwapChainHandle swap_chain, uint32_t desired_framebuffer_count = 3) = 0;
        virtual void swap_chain_destroy(SwapChainHandle swap_chain) = 0;
    };

}
