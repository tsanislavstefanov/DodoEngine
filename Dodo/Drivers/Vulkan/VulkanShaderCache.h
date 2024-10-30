#pragma once

#include "VulkanShader.h"

namespace Dodo {

    class VulkanShaderCache
    {
    public:
        VulkanShaderCache() = default;

        void Serialize();
        void Deserialize();
        VkShaderStageFlagBits GetChangedStages(VulkanShader* shader);

    private:
        std::map<std::string, std::map<VkShaderStageFlagBits, ShaderStageMetadata>> m_ShaderCache{};
    };

}