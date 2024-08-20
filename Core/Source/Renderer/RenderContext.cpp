#include "pch.h"
#include "RenderContext.h"
#include "Drivers/Vulkan/VulkanRenderContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<RenderContext> RenderContext::Create(RenderDeviceType renderDeviceType)
    {
        switch (renderDeviceType)
        {
            case RenderDeviceType::Vulkan : return Ref<VulkanRenderContext>::Create();
            default                       : DODO_ASSERT(false, "RenderDeviceType not supported!");
        }

        return nullptr;
    }

}