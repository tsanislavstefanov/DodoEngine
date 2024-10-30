#include "pch.h"
#include "Renderer.h"
#include "Drivers/Vulkan/VulkanRenderer.h"

namespace Dodo {

    Ref<Renderer> Renderer::Create(RenderThread& renderThread, RendererType type, const Window& targetWindow, VSyncMode vsyncMode)
    {
        switch (type)
        {
            case RendererType::Vulkan: return Ref<VulkanRenderer>::Create(renderThread, targetWindow, vsyncMode);
            default: break;
        }

        DODO_ASSERT(false, "Renderer type not supported!");
        return nullptr;
    }

    Renderer::Renderer(RenderThread& renderThread)
        : m_RenderThread{renderThread}
    {}

    void Renderer::Submit(RenderCommand&& command)
    {
        m_RenderThread.Submit(std::forward<RenderCommand>(command));
    }

}