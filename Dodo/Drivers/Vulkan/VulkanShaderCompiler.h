#pragma once

#include "VulkanShader.h"

namespace Dodo {

    class VulkanShaderCompiler
    {
    public:
        VulkanShaderCompiler();

        bool Recompile(VulkanShader* shader, bool forceCompile = false);

    private:
        void DeserializeCache();
        std::map<VkShaderStageFlagBits, std::string> Preprocess(VulkanShader* shader) const;
        VkShaderStageFlagBits HasCacheChanged(VulkanShader* shader, const std::map<VkShaderStageFlagBits, ShaderStageMetadata>& shaderMetadata);
        void SerializeCache() const;
        std::vector<uint32_t> try_get_cached_binary(VulkanShader* shader, VkShaderStageFlagBits shaderStage) const;
        void Reflect();

        std::map<std::string, std::map<VkShaderStageFlagBits, ShaderStageMetadata>> m_ShaderCache{};
        std::map<std::string, std::map<VkShaderStageFlagBits, std::vector<uint32_t>>> m_SPIRVData{};
    };

}