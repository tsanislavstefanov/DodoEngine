#include "pch.h"
#include "RenderContext.h"
#include "Core/Application.h"
#include "Drivers/Vulkan/VulkanRenderContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<RenderContext> RenderContext::Create()
    {
        const auto& app = Application::GetCurrent();
        switch (app.GetSpecs().RenderSettings.RenderApiType)
        {
            case RenderApiType::Vulkan : return Ref<VulkanRenderContext>::Create();
            default                    : DODO_ASSERT(false, "RenderApiType not supported!");
        }

        return nullptr;
    }

}