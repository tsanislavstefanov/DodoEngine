#pragma once

#include "render_handle.h"
#include "core/display.h"

namespace Dodo {

    class RenderDevice;

    DODO_DEFINE_RENDER_HANDLE(Surface);

    class RenderBackend : public RefCounted {
    public:
        enum class Type {
#ifdef DODO_VULKAN
            vulkan,
#endif
        };

        enum class VSyncMode {
            enabled,
            mailbox,
            disabled
        };

        struct SurfaceSpecifications {
            uint32_t width = 0;
            uint32_t height = 0;
            VSyncMode vsync_mode = VSyncMode::disabled;
        };

        struct Adapter {
            enum class Type {
                integrated,
                performance,
                unknown
            };

            enum class Vendor {
                nvidia,
                amd,
                intel,
                unknown
            };

            std::string name = {};
            Type type = Type::unknown;
            Vendor vendor = Vendor::unknown;
        };

        virtual ~RenderBackend() = default;

        virtual void initialize() = 0;
        virtual void on_event(Display::Event& e) = 0;
        virtual Type get_type() const = 0;
        virtual Ref<RenderDevice> render_device_create() = 0;
        virtual SurfaceHandle surface_create(Display::WindowId window, const SurfaceSpecifications& surface_specs, const void* platform_data) = 0;
        virtual uint32_t surface_get_width(SurfaceHandle surface) = 0;
        virtual uint32_t surface_get_height(SurfaceHandle surface) = 0;
        virtual void surface_set_size(SurfaceHandle surface, uint32_t width, uint32_t height) = 0;
        virtual bool surface_get_needs_resize(SurfaceHandle surface) = 0;
        virtual void surface_set_needs_resize(SurfaceHandle surface, bool needs_resize) = 0;
        virtual void surface_destroy(SurfaceHandle surface) = 0;
        virtual uint32_t adapter_get_count() const = 0;
        virtual const Adapter& adapter_get(size_t index) const = 0;
    };

}
