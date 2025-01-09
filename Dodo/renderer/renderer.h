#pragma once

#include "render_context.h"
#include "render_handle.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(CommandQueueFamily);
    DODO_DEFINE_RENDER_HANDLE(CommandQueue);
    DODO_DEFINE_RENDER_HANDLE(CommandListAllocator);
    DODO_DEFINE_RENDER_HANDLE(CommandList);
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

        enum CommandQueueFamilyBits {
            COMMAND_QUEUE_FAMILY_DRAW_BIT = 0x1,
            COMMAND_QUEUE_FAMILY_COMPUTE_BIT = 0x2,
            COMMAND_QUEUE_FAMILY_COPY_BIT = 0x4,
            COMMAND_QUEUE_FAMILY_MAX_COUNT = 3
        };

        virtual CommandQueueFamilyHandle command_queue_family_get(const std::bitset<COMMAND_QUEUE_FAMILY_MAX_COUNT>& cmd_queue_family_bits, SurfaceHandle surface_handle) = 0;
        virtual CommandQueueHandle command_queue_create(CommandQueueFamilyHandle cmd_queue_family_handle) = 0;
        virtual void command_queue_destroy(CommandQueueHandle cmd_queue_handle) = 0;

        enum class CommandListType {
            primary,
            secondary,
            none
        };

        virtual CommandListAllocatorHandle command_list_allocator_create(CommandQueueFamilyHandle cmd_queue_family_handle, CommandListType cmd_list_type) = 0;
        virtual void command_list_allocator_destroy(CommandListAllocatorHandle cmd_list_allocator_handle) = 0;

        virtual CommandListHandle command_list_create(CommandListAllocatorHandle cmd_list_allocator_handle) = 0;
        virtual void command_list_begin(CommandListHandle cmd_list_handle) = 0;
        virtual void command_list_end(CommandListHandle cmd_list_handle) = 0;

        virtual SwapChainHandle swap_chain_create(SurfaceHandle surface_handle) = 0;
        virtual void swap_chain_resize(CommandQueueHandle cmd_queue_handle, SwapChainHandle swap_chain_handle, uint32_t desired_framebuffer_count = 3) = 0;
        virtual void swapchain_begin_frame(SwapChainHandle swap_chain_handle) = 0;
        virtual void swapchain_present(SwapChainHandle swap_chain_handle) = 0;
        virtual void swapchain_on_resize(SwapChainHandle swap_chain_handle, uint32_t width, uint32_t height) = 0;
        virtual void swap_chain_destroy(SwapChainHandle swap_chain_handle) = 0;

        virtual BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) = 0;
        virtual void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) = 0;
        virtual void buffer_destroy(BufferHandle buffer_handle) = 0;
    };

}
