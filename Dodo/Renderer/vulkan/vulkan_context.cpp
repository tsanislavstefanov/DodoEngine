#include "pch.h"
#include "vulkan_context.h"
#include "vulkan_renderer.h"

namespace Dodo {
    SurfaceHandle RenderContextVulkan::surface_create(Display::WindowId window) const {
        DODO_ASSERT(false, "Surface can only be created by platform agnostic context!");
        return SurfaceHandle{};
    }

    void RenderContextVulkan::surface_set_vsync_mode(SurfaceHandle surface, VSyncMode vsync_mode) {
        SurfaceVulkan* surface_vk = reinterpret_cast<SurfaceVulkan*>(surface.handle);
        surface_vk->vsync_mode = vsync_mode;
        surface_vk->needs_resize = true;
    }

    void RenderContextVulkan::surface_resize(SurfaceHandle surface, uint32_t width, uint32_t height) {
        SurfaceVulkan* surface_vk = reinterpret_cast<SurfaceVulkan*>(surface.handle);
        surface_vk->width  = width;
        surface_vk->height = height;
        surface_vk->needs_resize = true;
    }

    Ref<Renderer> RenderContextVulkan::renderer_create() const {
        return Ref<RendererVulkan>::create();
    }

}