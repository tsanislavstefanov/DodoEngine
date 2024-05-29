#include "pch.h"
#include "RenderContext.h"
#include "Renderer.h"
#include "Drivers/Vulkan/VulkanContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<RenderContext> RenderContext::Create()
    {
        switch (Renderer::GetSettings().RendererApiType)
        {
            case RendererApiType::Vulkan: return Ref<VulkanContext>::Create();
            default: DODO_ASSERT(false, "Renderer API type not supported!");
        }

        return nullptr;
    }

}