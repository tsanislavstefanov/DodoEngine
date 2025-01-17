#pragma once

#include "render_context.h"
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

    class Renderer : public RefCounted {
    public:
        enum class BufferUsage {
            vertex_buffer,
            auto_count,
            none
        };

        virtual ~Renderer() = default;

        virtual void initialize(size_t device_index) = 0;

        enum CommandQueueFamilyType {
            COMMAND_QUEUE_FAMILY_TYPE_GRAPHICS,
            COMMAND_QUEUE_FAMILY_TYPE_COMPUTE ,
            COMMAND_QUEUE_FAMILY_TYPE_TRANSFER
        };

        virtual CommandQueueFamilyHandle command_queue_family_get(CommandQueueFamilyType command_queue_family_type, SurfaceHandle surface) = 0;
        virtual CommandQueueHandle command_queue_create(CommandQueueFamilyHandle command_queue_family) = 0;
        virtual void command_queue_destroy(CommandQueueHandle command_queue) = 0;

        virtual CommandPoolHandle command_pool_create(CommandQueueFamilyHandle command_queue_family) = 0;
        virtual void command_pool_destroy(CommandPoolHandle command_pool) = 0;

        enum CommandBufferType {
            COMMAND_BUFFER_TYPE_PRIMARY,
            COMMAND_BUFFER_TYPE_SECONDARY
        };

        virtual CommandBufferHandle command_buffer_create(CommandPoolHandle command_pool, CommandBufferType command_buffer_type) = 0;
        virtual void command_buffer_begin(CommandBufferHandle command_buffer) = 0;
        virtual void command_buffer_end(CommandBufferHandle command_buffer) = 0;

        virtual FenceHandle fence_create() = 0;
        virtual void fence_wait(FenceHandle fence) = 0;
        virtual void fence_destroy(FenceHandle fence) = 0;

        virtual SemaphoreHandle semaphore_create() = 0;
        virtual void semaphore_destroy(SemaphoreHandle semaphore) = 0;

        enum SwapChainStatus {
            SWAP_CHAIN_STATUS_OK,
            SWAP_CHAIN_STATUS_OUT_OF_DATE,
            SWAP_CHAIN_STATUS_ERROR
        };

        virtual SwapChainHandle swap_chain_create(SurfaceHandle surface) = 0;
        virtual FramebufferHandle swap_chain_acquire_next_framebuffer(SemaphoreHandle signal_semaphore, SwapChainHandle swap_chain, SwapChainStatus& swap_chain_status) = 0;
        virtual void swap_chain_resize(CommandQueueHandle cmd_queue, SwapChainHandle swap_chain, uint32_t desired_framebuffer_count = 3) = 0;
        virtual void swap_chain_destroy(SwapChainHandle swap_chain) = 0;

        virtual BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) = 0;
        virtual void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) = 0;
        virtual void buffer_destroy(BufferHandle buffer_handle) = 0;
    };

}