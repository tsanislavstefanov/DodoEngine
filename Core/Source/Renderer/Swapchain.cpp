#include "pch.h"
#include "Swapchain.h"
#include "Core/Application.h"
#include "Drivers/Vulkan/VulkanSwapchain.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SWAPCHAIN ///////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<Swapchain> Swapchain::Create(void* windowHandle, uint32_t width, uint32_t height)
    {
        const ApplicationSpecs& appSpecs = Application::GetCurrent().GetSpecs();
        switch (appSpecs.RenderSettings.RendererApiType)
        {
            case RendererApiType::Vulkan: return Ref<VulkanSwapchain>::Create(windowHandle, width, height);
            default: DODO_ASSERT(false, "Renderer API type not supported!");
        }

        return nullptr;
    }

};