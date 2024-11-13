#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanShaderCompiler.h"

#include <yaml-cpp/yaml.h>

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
        
        static const char* GetCachedFileExtensionByShaderStage(VkShaderStageFlagBits shaderStage)
        {
            switch (shaderStage)
            {
                case VK_SHADER_STAGE_VERTEX_BIT:   return ".cached_vulkan_debug.vertex";
                case VK_SHADER_STAGE_FRAGMENT_BIT: return ".cached_vulkan_debug.fragment";
            }

            DODO_VERIFY(false);
            return nullptr;
        }

    }

    static const std::filesystem::path s_CacheDirectory = "Assets/Shaders/Cache/Vulkan";
    static const std::filesystem::path s_ShaderRegistryPath = s_CacheDirectory / "ShaderRegistry.cache";

    VulkanShaderCompiler::VulkanShaderCompiler()
    {
        std::filesystem::create_directories(s_CacheDirectory);
        DeserializeCache();
    }

    bool VulkanShaderCompiler::Recompile(VulkanShader* shader, bool force_compile)
    {
        DODO_LOG_INFO_TAG("Renderer", "Preprocessing shader: {0}.", shader->AssetPath.string());
        const std::map<VkShaderStageFlagBits, std::string> stages = Preprocess(shader);
        if (stages.empty())
            return false;

        std::map<VkShaderStageFlagBits, ShaderStageMetadata> metadata{};
        for (const auto& [stage, stageSource] : stages)
            metadata[stage].Hash = FNV1a{}(stageSource);

        const VkShaderStageFlagBits changed_stages = HasCacheChanged(shader, metadata);
        for (const auto& [stage, stageSource] : stages)
        {
            std::vector<uint32_t> output_binary{};
            if (!force_compile && (stage & ~changed_stages))
                output_binary = try_get_cached_binary(shader, stage);

            // No cached binary found!
            if (output_binary.empty())
            {

            }
        }

        return true;
    }

    void VulkanShaderCompiler::DeserializeCache()
    {
    }

    std::map<VkShaderStageFlagBits, std::string> VulkanShaderCompiler::Preprocess(VulkanShader* shader) const
    {
        const std::string shaderSource = File::ReadAndSkipBOM(shader->AssetPath);

        VulkanShaderPreprocessor preprocessor{};
        std::map<VkShaderStageFlagBits, std::string> shaderStages = preprocessor.SplitToStages(shaderSource);

        shaderc::Compiler compiler{};
        for (const auto& [stage, stageSource] : shaderStages)
        {
            shaderc::CompileOptions options{};
            options.SetWarningsAsErrors();

            const shaderc_shader_kind shaderKind = Utils::ConvertShaderStageToShaderCKind(stage);
            const auto result = compiler.PreprocessGlsl(stageSource, shaderKind, shader->AssetPath.string().c_str(), options);
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

    VkShaderStageFlagBits VulkanShaderCompiler::HasCacheChanged(VulkanShader* shader, const std::map<VkShaderStageFlagBits, ShaderStageMetadata>& metadata)
    {
        const bool shaderNotCached = m_ShaderCache.find(shader->AssetPath.string()) == m_ShaderCache.end();
        VkShaderStageFlagBits changedStages{};
        for (const auto& [stage, stageMetadata] : metadata)
            // [] operator used!
            // Which means that the element gets inserted if it's not already there.
            if (shaderNotCached || (stageMetadata != m_ShaderCache[shader->AssetPath.string()][stage]))
            {
                m_ShaderCache[shader->AssetPath.string()][stage] = stageMetadata;
                *(int*)&changedStages |= stage;
            }

        if (changedStages)
            SerializeCache();

        return changedStages;
    }

    void VulkanShaderCompiler::SerializeCache() const
    {
        YAML::Emitter yaml{};
        yaml << YAML::BeginMap;
        yaml << YAML::Key << "Shaders" << YAML::BeginSeq;
        for (const auto& [shaderAssetPath, shader] : m_ShaderCache)
        {
            yaml << YAML::BeginMap;
            yaml << YAML::Key << "ShaderAssetPath" << YAML::Value << shaderAssetPath;
            yaml << YAML::Key << "ShaderStages" << YAML::BeginSeq;

            for (const auto& [stage, stageMetadata] : shader)
            {
                yaml << YAML::BeginMap;
                yaml << YAML::Key << "StageName" << YAML::Value << Utils::ConvertShaderStageToString(stage);
                yaml << YAML::Key << "StageHash" << YAML::Value << stageMetadata.Hash;
                yaml << YAML::EndMap;
            }

            yaml << YAML::EndSeq;
            yaml << YAML::EndMap;
        }

        yaml << YAML::EndSeq << YAML::EndMap;

        std::ofstream stream(s_ShaderRegistryPath);
        stream << yaml.c_str();
    }

    std::vector<uint32_t> VulkanShaderCompiler::try_get_cached_binary(VulkanShader* shader, VkShaderStageFlagBits shaderStage) const
    {
        const std::string extension = Utils::GetCachedFileExtensionByShaderStage(shaderStage);
        std::ifstream is(s_CacheDirectory / (shader->AssetPath.filename().string() + extension), std::ios::binary);
        if (!is)
            return {};

        is.seekg(0, std::ios::end);
        const std::streampos size = is.tellg();
        is.seekg(0, std::ios::beg);
        std::vector<uint32_t> result{};
        result.resize(size / sizeof(uint32_t));
        is.read(reinterpret_cast<char*>(result.data()), size);
        is.close();
        return result;
    }

    void VulkanShaderCompiler::Reflect()
    {
    }



}