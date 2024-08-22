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
        switch (Application::GetCurrent().GetSpecs().RenderDeviceType)
        {
            case RenderDeviceType::Vulkan : return Ref<VulkanContext>::Create();
            default                       : DODO_ASSERT(false, "RenderDeviceType not supported!");
        }

        return nullptr;
    }

}