#pragma once

#include "render_context.h"
#include "render_handle.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(Buffer);
    DODO_DEFINE_RENDER_HANDLE(Swapchain);

    class Renderer : public RefCounted {
    public:
        virtual ~Renderer() = default;

        virtual SwapchainHandle swapchain_create(SurfaceHandle surface) = 0;
        virtual void swapchain_begin_frame(SwapchainHandle swapchain) = 0;
        virtual void swapchain_present(SwapchainHandle swapchain) = 0;
        virtual void swapchain_on_resize(SwapchainHandle swapchain, uint32_t width, uint32_t height) = 0;
        virtual void swapchain_destroy(SwapchainHandle swapchain) = 0;

        enum class BufferUsage {
            vertex_buffer,
            auto_count,
            none
        };

        virtual BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) = 0;
        virtual void buffer_upload_data(BufferHandle buffer, void* data, size_t size, size_t offset = 0) = 0;
        virtual void buffer_destroy(BufferHandle buffer) = 0;
    };

}