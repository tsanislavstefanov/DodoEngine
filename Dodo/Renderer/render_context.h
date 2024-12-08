#pragma once

#include "render_handle.h"
#include "core/display.h"

namespace Dodo {

    DODO_DEFINE_RENDER_HANDLE(Surface);

    class Renderer;

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

        struct SurfaceSpecifications {
            uint32_t width  = 0;
            uint32_t height = 0;
            VSyncMode vsync_mode = VSyncMode::none;
        };

        struct Adapter {
            enum class Type {
                discrete,
                integrated,
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

            std::string name{};
            Type type = Type::none;
            Vendor vendor = Vendor::none;
        };

        RenderContext() = default;
        virtual ~RenderContext() = default;

        virtual SurfaceHandle surface_create(Display::WindowId window, const SurfaceSpecifications& surface_specs, const void* platform_data) = 0;
        virtual void surface_set_size(SurfaceHandle surface, uint32_t width, uint32_t height) = 0;
        virtual void surface_destroy(SurfaceHandle surface) = 0;
        virtual Ref<Renderer> renderer_create(size_t device_type) const = 0;

        uint32_t get_adapter_count() const;
        const Adapter& get_adapter(size_t index) const;

    protected:
        std::vector<Adapter> _adapters{};
    };

}