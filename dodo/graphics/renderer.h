#pragma once

#include "buffer.h"
#include "render_thread.h"
#include "vsync_mode.h"
#include "core/window.h"

namespace Dodo {

    enum class RendererType
    {
        vulkan,
        auto_count,
        none
    };

    class Renderer : public RefCounted
    {
    public:
        static Ref<Renderer> create(
            RenderThread& render_thread, RendererType type, const Window& target_window,
            VSyncMode vsync_mode = VSyncMode::enabled);

        Renderer(RenderThread& render_thread);
        virtual ~Renderer() = default;

        void submit(RenderCommand&& command, bool execute_immediately = false);

        virtual BufferID create_buffer(BufferSpecifications&& buffer_specs) = 0;
        virtual void free_buffer(BufferID buffer_id) = 0;

        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;
        virtual void resize_swapchain(uint32_t width, uint32_t height) = 0;
        virtual RenderThreadPolicy get_thread_policy() const = 0;
        virtual RendererType get_type() const = 0;

    private:
        RenderThread& render_thread_;
    };

}