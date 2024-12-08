#pragma once

#include "renderer/vulkan/render_context_vulkan.h"

namespace Dodo {

    class RenderContextVulkanWindows : public RenderContextVulkan {
    public:
        RenderContextVulkanWindows() = default;

        SurfaceHandle surface_create(Display::WindowId window_id, const SurfaceSpecifications& surface_specs, const void* platform_data) override;

    protected:
        const char* _get_platform_surface_extension() const override;
    };

}