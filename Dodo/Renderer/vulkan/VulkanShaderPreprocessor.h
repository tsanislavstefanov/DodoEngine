#pragma once

namespace Dodo {

    class VulkanShaderPreprocessor
    {
    public:
        VulkanShaderPreprocessor() = default;

        std::map<VkShaderStageFlagBits, std::string> SplitToStages(const std::string& shaderSource);
    };

}