#pragma once

#include "Renderer/Shaders/Shader.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN SHADER ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanShader : public Shader
    {
    public:
        VulkanShader(const std::string& assetPath);

        void Reload() override;
        void Reload_RenderThread() override;

    private:
        std::string m_AssetPath;
    };

}