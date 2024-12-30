#pragma once

#include "render_context.h"
#include "render_handle.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(CommandQueue);
    DODO_DEFINE_RENDER_HANDLE(Swapchain);
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

        virtual SwapchainHandle swapchain_create(SurfaceHandle surface_handle, uint32_t desired_framebuffer_count = 3) = 0;
        virtual void swapchain_begin_frame(SwapchainHandle swapchain_handle) = 0;
        virtual void swapchain_present(SwapchainHandle swapchain_handle) = 0;
        virtual void swapchain_on_resize(SwapchainHandle swapchain_handle, uint32_t width, uint32_t height) = 0;
        virtual void swapchain_destroy(SwapchainHandle swapchain_handle) = 0;

        virtual BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) = 0;
        virtual void buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset = 0) = 0;
        virtual void buffer_destroy(BufferHandle buffer_handle) = 0;
    };

}