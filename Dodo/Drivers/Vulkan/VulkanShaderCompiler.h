#pragma once

#include "VulkanShader.h"
#include "VulkanShaderCache.h"

namespace Dodo {

    class VulkanShaderCompiler
    {
    public:
        VulkanShaderCompiler();

        bool Recompile(VulkanShader* shader, bool forceCompile = false);

    private:
        std::map<VkShaderStageFlagBits, std::string> Preprocess(VulkanShader* shader) const;
        void Reflect();

        VulkanShaderCache m_ShaderCache{};
        ShaderStageMetadata m_Metadata{};
        std::map<VkShaderStageFlagBits, std::vector<uint32_t>> m_SPIRVDebugData{}, m_SPIRVData{};
    };

}