#include "pch.h"
#include "Renderer.h"
#include "RenderApi.h"
#include "Core/Application.h"
#include "Drivers/Vulkan/VulkanRenderApi.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static RenderApi* CreateRenderApi(RenderApiType apiType)
        {
            switch (apiType)
            {
                case RenderApiType::Vulkan : return new VulkanRenderApi();
                default                    : return nullptr;
            }
        }

    }

    ////////////////////////////////////////////////////////////////
    // DATA ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static RenderSettings s_RenderSettings{};
    static RenderApi* s_RenderApi = nullptr;

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void Renderer::Init()
    {
        const auto& app = Application::GetCurrent();
        s_RenderSettings = app.GetSpecs().RenderSettings;
        s_RenderApi = Utils::CreateRenderApi(s_RenderSettings.RenderApiType);
    }

    void Renderer::Destroy()
    {
        delete s_RenderApi;
        s_RenderApi = nullptr;
    }

}