#include "pch.h"
#include "RenderContext.h"
#include "Core/Application.h"
#include "Drivers/Vulkan/VulkanContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<RenderContext> RenderContext::Create()
    {
        const ApplicationSpecs& appSpecs = Application::GetCurrent().GetSpecs();
        switch (appSpecs.RenderSettings.RendererApiType)
        {
            case RendererApiType::Vulkan: return Ref<VulkanContext>::Create();
            default: DODO_ASSERT(false, "Renderer API type not supported!");
        }

        return nullptr;
    }

}