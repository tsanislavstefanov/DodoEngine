#include "pch.h"
#include "RenderDevice.h"
#include "Drivers/Vulkan/VulkanDevice.h"

namespace Dodo {

    Ref<RenderDevice> RenderDevice::Create(RenderDeviceType renderDeviceType)
    {
        switch (renderDeviceType)
        {
            case RenderDeviceType::Vulkan : return Ref<VulkanDevice>::Create();
            default                       : DODO_ASSERT(false, "RenderDeviceType not supported!");
        }

        return nullptr;
    }

}