#pragma once

#include "renderer/vulkan/vulkan_context.h"

namespace Dodo {

    class RenderContextVulkanWindows : public RenderContextVulkan {
    public:
        RenderContextVulkanWindows() = default;

        SurfaceHandle surface_create(Display::WindowId window) const override;

    protected:
        const char* _get_platform_surface_extension() const override;
    };

}