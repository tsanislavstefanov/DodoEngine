#pragma once

#include "render_context.h"
#include "render_handle.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(CommandQueueFamily);
    DODO_DEFINE_RENDER_HANDLE(CommandQueue);
    DODO_DEFINE_RENDER_HANDLE(CommandPool);
    DODO_DEFINE_RENDER_HANDLE(CommandList);
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

        enum class CommandQueueFamilyType {
            draw,
            compute,
            copy
        };

        virtual CommandQueueFamilyHandle command_queue_family_get(CommandQueueFamilyType command_queue_family_type, SurfaceHandle surface) = 0;
        virtual CommandQueueHandle command_queue_create(CommandQueueFamilyHandle command_queue_family) = 0;
        virtual void command_queue_execute_and_present(CommandQueueHandle command_queue, SemaphoreHandle wait_semaphore, const std::vector<CommandListHandle>& command_lists, SemaphoreHandle signal_semaphore, FenceHandle fence, SwapChainHandle swap_chain) = 0;
        virtual void command_queue_destroy(CommandQueueHandle command_queue) = 0;

        enum class CommandListType {
            primary,
            secondary
        };

        virtual CommandPoolHandle command_pool_create(CommandQueueFamilyHandle command_queue_family, CommandListType command_list_type) = 0;
        virtual void command_pool_destroy(CommandPoolHandle command_pool) = 0;

        virtual CommandListHandle command_list_create(CommandPoolHandle command_pool) = 0;
        virtual void command_list_begin(CommandListHandle command_list) = 0;
        virtual void command_list_end(CommandListHandle command_list) = 0;

        virtual FenceHandle fence_create() = 0;
        virtual void fence_wait(FenceHandle fence) = 0;
        virtual void fence_destroy(FenceHandle fence) = 0;

        virtual SemaphoreHandle semaphore_create() = 0;
        virtual void semaphore_destroy(SemaphoreHandle semaphore) = 0;

        enum class SwapChainStatus {
            ok,
            out_of_date,
            error
        };

        virtual SwapChainHandle swap_chain_create(SurfaceHandle surface) = 0;
        virtual FramebufferHandle swap_chain_acquire_next_framebuffer(SemaphoreHandle semaphore, SwapChainHandle swap_chain, FenceHandle fence, SwapChainStatus& swap_chain_status) = 0;
        virtual void swap_chain_resize(CommandQueueHandle cmd_queue, SwapChainHandle swap_chain, uint32_t desired_framebuffer_count = 3) = 0;
        virtual void swap_chain_destroy(SwapChainHandle swap_chain) = 0;

        virtual BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) = 0;
        virtual void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) = 0;
        virtual void buffer_destroy(BufferHandle buffer_handle) = 0;
    };

}
