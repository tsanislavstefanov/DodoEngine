#pragma once

#include <vulkan/vulkan.h>

#include "renderer/render_context.h"

namespace Dodo {

    class RenderContextVulkan : public RenderContext {
    public:
        RenderContextVulkan() = default;

        SurfaceHandle surface_create(Display::WindowId window) const override;
        void surface_set_vsync_mode(SurfaceHandle surface, VSyncMode vsync_mode) override;
        void surface_resize(SurfaceHandle surface, uint32_t width, uint32_t height) override;
        Ref<Renderer> renderer_create() const override;

    protected:
        struct SurfaceVulkan {
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            uint32_t width  = 0;
            uint32_t height = 0;
            VSyncMode vsync_mode = VSyncMode::none;
            bool needs_resize = false;
        };

        virtual const char* _get_platform_surface_extension() const = 0;
    };

}