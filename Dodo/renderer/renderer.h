#pragma once

#include "render_context.h"
#include "render_handle.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(CommandQueueFamily);
    DODO_DEFINE_RENDER_HANDLE(CommandQueue);
    DODO_DEFINE_RENDER_HANDLE(SwapChain);
    DODO_DEFINE_RENDER_HANDLE(Buffer);

    class Renderer : public RefCounted {
    public:
        enum CommandQueueFamilyBits {
            graphics = 0x1,
            compute = 0x2,
            copy = 0x4,
            count = 3
        };

        enum class BufferUsage {
            vertex_buffer,
            auto_count,
            none
        };

        virtual ~Renderer() = default;

        virtual void initialize(size_t device_index) = 0;

        virtual CommandQueueFamilyHandle command_queue_family_get(const std::bitset<CommandQueueFamilyBits::count>& cmd_queue_family_bits, SurfaceHandle surface_handle) = 0;
        virtual CommandQueueHandle command_queue_create(CommandQueueFamilyHandle cmd_queue_family_handle) = 0;
        // virtual void command_queue_execute(CommandQueueHandle cmd_queue_handle, ...) = 0;
        virtual void command_queue_destroy(CommandQueueHandle cmd_queue_handle) = 0;

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