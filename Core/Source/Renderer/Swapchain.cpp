#include "pch.h"
#include "Swapchain.h"
#include "Core/Application.h"
#include "Drivers/Vulkan/VulkanSwapchain.h"

namespace Dodo {

    Ref<Swapchain> Swapchain::Create(void* windowHandle, uint32_t windowWidth, uint32_t windowHeight)
    {
        switch (Application::GetCurrent().GetSpecs().RenderDeviceType)
        {
            case RenderDeviceType::Vulkan : return Ref<VulkanSwapchain>::Create(windowHandle, windowWidth, windowHeight);
            default                       : DODO_ASSERT(false, "RenderDeviceType not supported!");
        }

        return nullptr;
    }

}