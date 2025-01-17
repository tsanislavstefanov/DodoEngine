#pragma once

#include "render_handle.h"
#include "core/display.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(Surface);

    class Renderer;

    class RenderContext : public RefCounted {
    public:
        enum Type {
#ifdef DODO_VULKAN
            RENDER_CONTEXT_TYPE_VULKAN,
#endif
        };

        virtual ~RenderContext() = default;

        virtual void initialize() = 0;
        virtual void on_event(Display::Event& e) = 0;
        virtual Type get_type() const = 0;

        enum VSyncMode {
            VSYNC_MODE_ENABLED,
            VSYNC_MODE_MAILBOX,
            VSYNC_MODE_DISABLED
        };

        struct SurfaceSpecifications {
            uint32_t width = 0;
            uint32_t height = 0;
            VSyncMode vsync_mode = VSYNC_MODE_DISABLED;
        };

        virtual SurfaceHandle surface_create(Display::WindowId window, const SurfaceSpecifications& surface_specs, const void* platform_data) = 0;
        virtual void surface_set_size(SurfaceHandle surface, uint32_t width, uint32_t height) = 0;
        virtual bool surface_get_needs_resize(SurfaceHandle surface) = 0;
        virtual void surface_set_needs_resize(SurfaceHandle surface, bool needs_resize) = 0;
        virtual void surface_destroy(SurfaceHandle surface) = 0;

        virtual Ref<Renderer> renderer_create() = 0;

        struct Adapter {
            enum class Type {
                discrete,
                integrated,
                unknown,
                auto_count,
                none
            };

            enum class Vendor {
                amd,
                img_tec,
                nvidia,
                arm,
                qualcomm,
                intel,
                unknown,
                auto_count,
                none
            };

            std::string name = {};
            Type type = Type::none;
            Vendor vendor = Vendor::none;
        };

        virtual uint32_t adapter_get_count() const = 0;
        virtual const Adapter& adapter_get(size_t index) const = 0;
    };

}