#pragma once

#include "render_context.h"
#include "render_handle.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(CommandQueueFamily);
    DODO_DEFINE_RENDER_HANDLE(CommandQueue);
    DODO_DEFINE_RENDER_HANDLE(CommandListPool);
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

        virtual CommandQueueFamilyHandle command_queue_family_get(CommandQueueFamilyType cmd_queue_family_type, SurfaceHandle surface_handle) = 0;
        virtual CommandQueueHandle command_queue_create(CommandQueueFamilyHandle cmd_queue_family_handle) = 0;
        virtual void command_queue_execute_and_present(CommandQueueHandle cmd_queue_handle, SemaphoreHandle wait_semaphore_handle, const std::vector<CommandListHandle>& cmd_list_handles, SemaphoreHandle signal_semaphore_handle, FenceHandle fence_handle, SwapChainHandle swap_chain_handle) = 0;
        virtual void command_queue_destroy(CommandQueueHandle cmd_queue_handle) = 0;

        enum class CommandListType {
            primary,
            secondary
        };

        virtual CommandListPoolHandle command_list_pool_create(CommandQueueFamilyHandle cmd_queue_family_handle, CommandListType cmd_list_type) = 0;
        virtual void command_list_pool_destroy(CommandListPoolHandle cmd_list_pool_handle) = 0;

        virtual CommandListHandle command_list_create(CommandListPoolHandle cmd_list_pool_handle) = 0;
        virtual void command_list_begin(CommandListHandle cmd_list_handle) = 0;
        virtual void command_list_end(CommandListHandle cmd_list_handle) = 0;

        virtual FenceHandle fence_create() = 0;
        virtual void fence_wait(FenceHandle fence_handle) = 0;
        virtual void fence_destroy(FenceHandle fence_handle) = 0;
        
        virtual SemaphoreHandle semaphore_create() = 0;
        virtual void semaphore_destroy(SemaphoreHandle semaphore_handle) = 0;

        virtual SwapChainHandle swap_chain_create(SurfaceHandle surface_handle) = 0;
        virtual FramebufferHandle swap_chain_acquire_next_framebuffer(SemaphoreHandle semaphore_handle, SwapChainHandle swap_chain_handle, bool& needs_resize, FenceHandle fence_handle) = 0;
        virtual void swap_chain_resize(CommandQueueHandle cmd_queue_handle, SwapChainHandle swap_chain_handle, uint32_t desired_framebuffer_count = 3) = 0;
        virtual void swap_chain_destroy(SwapChainHandle swap_chain_handle) = 0;

        virtual BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) = 0;
        virtual void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) = 0;
        virtual void buffer_destroy(BufferHandle buffer_handle) = 0;
    };

}