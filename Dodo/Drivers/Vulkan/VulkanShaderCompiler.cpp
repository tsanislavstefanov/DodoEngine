#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanShaderCompiler.h"
#include "VulkanShaderCache.h"
#include "VulkanShaderPreprocessor.h"
#include "VulkanShaderUtils.h"

#include "Core/File.h"
#include "Core/Hash.h"

namespace Dodo {

    namespace Utils {

        static shaderc_shader_kind ConvertShaderStageToShaderCKind(VkShaderStageFlagBits shaderStage)
        {
            switch (shaderStage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT:   return shaderc_glsl_vertex_shader;
                case VK_SHADER_STAGE_FRAGMENT_BIT: return shaderc_glsl_fragment_shader;
                default: break;
            }

            DODO_ASSERT(false, "ShaderStage not supported!");
            return {};
        }

    }

    static constexpr const char* s_CacheDirectory = "Assets/Cache/Shader/Vulkan";

    VulkanShaderCompiler::VulkanShaderCompiler()
    {
        std::filesystem::create_directories(s_CacheDirectory);
    }

    bool VulkanShaderCompiler::Recompile(VulkanShader* shader, bool forceCompile)
    {
        DODO_LOG_INFO_TAG("Renderer", "Preprocessing shader: {0}.", shader->AssetPath.string());
        const std::map<VkShaderStageFlagBits, std::string> shaderStages = Preprocess(shader);
        if (shaderStages.empty())
            return false;

        shader->Metadata.Hash = FNV1a{}(shader->Source);
        const VkShaderStageFlagBits changedStages = m_ShaderCache.GetChangedStages(shader);

        return true;
    }

    std::map<VkShaderStageFlagBits, std::string> VulkanShaderCompiler::Preprocess(VulkanShader* shader) const
    {
        VulkanShaderPreprocessor preprocessor{};
        std::map<VkShaderStageFlagBits, std::string> shaderStages = preprocessor.SplitToStages(shader->Source);

        shaderc::Compiler compiler{};
        for (const auto& [stage, source] : shaderStages)
        {
            shaderc::CompileOptions options{};
            options.SetWarningsAsErrors();

            const shaderc_shader_kind shaderKind = Utils::ConvertShaderStageToShaderCKind(stage);
            const auto result = compiler.PreprocessGlsl(source, shaderKind, shader->AssetPath.string().c_str(), options);
            if (result.GetCompilationStatus() != shaderc_compilation_status_success)
            {
                DODO_LOG_ERROR_TAG("Renderer", "Compiler failed to preprocess stage: {0}!", Utils::ConvertShaderStageToString(stage));
                DODO_LOG_ERROR_TAG("Renderer", "    Message: {0}", result.GetErrorMessage());
                return {};
            }

            shaderStages.at(stage) = std::string(result.begin(), result.end());
        }

        return shaderStages;
    }

    void VulkanShaderCompiler::Reflect()
    {
    }



}