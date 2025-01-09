#pragma once

#include "render_handle.h"
#include "core/display.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(Surface);

    class Renderer;

    class RenderContext : public RefCounted {
    public:
        enum class Type {
#ifdef DODO_VULKAN
            vulkan,
#endif
            auto_count,
            none
        };

        virtual ~RenderContext() = default;

        virtual void initialize() = 0;
        virtual void on_event(Display::Event& e) = 0;
        virtual Type get_type() const = 0;

        enum class VSyncMode {
            disabled,
            enabled,
            mailbox,
            auto_count,
            none
        };

        struct SurfaceSpecifications {
            uint32_t width  = 0;
            uint32_t height = 0;
            VSyncMode vsync_mode = VSyncMode::none;
        };

        virtual SurfaceHandle surface_create(Display::WindowId window_id, const SurfaceSpecifications& surface_specs, const void* platform_data) = 0;
        virtual void surface_on_resize(SurfaceHandle surface_handle, uint32_t width, uint32_t height) = 0;
        virtual void surface_destroy(SurfaceHandle surface_handle) = 0;

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