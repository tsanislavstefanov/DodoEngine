#include "pch.h"
#include "renderer.h"
#include "swapchain.h"
#include "core/window.h"
#ifdef DODO_USE_VULKAN
#   include "drivers/vulkan/vulkan_renderer.h"
#endif

namespace Dodo {

    Ref<Renderer> Renderer::create(
            RenderThread& render_thread, RendererType type, const Window& target_window,
            VSyncMode vsync_mode /* = VSyncMode::enabled */)
    {
        switch (type)
        {
#ifdef DODO_USE_VULKAN
            case RendererType::vulkan:
                return Ref<VulkanRenderer>::create(render_thread, target_window, vsync_mode);
#endif
            default:
            {
                DODO_ASSERT(false, "Renderer type not supported!");
                return nullptr;
            }
        }
    }

    Renderer::Renderer(RenderThread& render_thread)
        : render_thread_(render_thread)
    {}

    void Renderer::submit(RenderCommand&& command, bool execute_immediately /* = false */)
    {
        render_thread_.submit(std::forward<RenderCommand>(command), execute_immediately);
    }

}