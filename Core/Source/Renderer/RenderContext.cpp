#include "pch.h"
#include "RenderContext.h"
#include "Drivers/Vulkan/VulkanContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<RenderContext> RenderContext::Create(RenderDeviceDriverType deviceDriverType)
    {
        switch (deviceDriverType)
        {
            case RenderDeviceDriverType::Vulkan : return Ref<VulkanContext>::Create();
            default : DODO_ASSERT(false, "RenderDeviceDriverType not supported!");
        }

        return nullptr;
    }

}