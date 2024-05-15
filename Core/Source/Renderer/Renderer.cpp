#include "pch.h"
#include "Renderer.h"
#include "RenderDevice.h"
#include "Drivers/Vulkan/VulkanDevice.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static std::unique_ptr<RenderDevice> CreateRenderDevice(RenderDeviceType deviceType)
        {
            switch (deviceType)
            {
                case RenderDeviceType::Vulkan: return std::make_unique<VulkanDevice>();
            }

            ASSERT(false, "RenderDevice type not supported!");
        }

    }

    ////////////////////////////////////////////////////////////////
    // RENDERER DATA ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct RendererData
    {
        RenderSettings                Settings{};
        std::unique_ptr<RenderDevice> Device = nullptr;
    };

    static RendererData s_Data{};

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    const RenderSettings& Renderer::GetSettings()
    {
        return s_Data.Settings;
    }

    void Renderer::SetSettings(const RenderSettings& settings)
    {
        s_Data.Settings = settings;
    }

    void Renderer::Init()
    {
        s_Data.Device = Utils::CreateRenderDevice(s_Data.Settings.RenderDeviceType);
    }

    void Renderer::BeginFrame()
    {
        s_Data.Device->BeginFrame();
    }

    void Renderer::Resize(uint32_t width, uint32_t height)
    {
        s_Data.Device->Resize(width, height);
    }

    void Renderer::EndFrame()
    {
        s_Data.Device->EndFrame();
    }

    void Renderer::Dispose()
    {
        s_Data.Device->Dispose();
    }

}