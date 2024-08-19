#pragma once

#include <vulkan/vulkan.h>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN SHADER COMPILER //////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanShaderCompiler
    {
    public:
        VulkanShaderCompiler(const std::string& shaderAssetPath);

        bool TryReload();

    private:
        std::map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);

        std::string m_ShaderAssetPath;
        std::map<VkShaderStageFlagBits, std::string> m_ShaderSources{};
    };

}