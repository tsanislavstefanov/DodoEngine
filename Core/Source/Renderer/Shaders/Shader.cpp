#include "pch.h"
#include "Shader.h"
#include "Core/Application.h"
#include "Drivers/Vulkan/Shaders/VulkanShader.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SHADER //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<Shader> Shader::Create(const std::string& assetPath)
    {
        const auto& app = Application::GetCurrent();
        switch (app.GetSpecs().RenderSettings.RenderApiType)
        {
            case RenderApiType::Vulkan : return Ref<VulkanShader>::Create(assetPath);
            default                    : DODO_ASSERT(false, "RenderApiType not supported!");
        }

        return nullptr;
    }

}