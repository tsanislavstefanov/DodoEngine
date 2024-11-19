#pragma once

#include "renderer.h"
#include "core/display.h"

namespace Dodo {

    class RenderContext : public RefCounted {
    public:
        enum class Type {
            vulkan,
            auto_count,
            none
        };

        enum class VSyncMode {
            disabled,
            enabled,
            mailbox,
            auto_count,
            none
        };

        RenderContext() = default;
        virtual ~RenderContext() = default;

        virtual SurfaceHandle surface_create(Display::WindowId window) const = 0;
        virtual void surface_set_vsync_mode(SurfaceHandle surface, VSyncMode vsync_mode) = 0;
        virtual void surface_resize(SurfaceHandle surface, uint32_t width, uint32_t height) = 0;
        virtual Ref<Renderer> renderer_create() const = 0;
    };

}