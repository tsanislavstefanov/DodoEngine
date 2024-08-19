#include "pch.h"
#include "VulkanShaderCompiler.h"
#include "Core/FileSystem.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static const char* GetCacheDirectory()
        {
            return "Assets/Shaders/Cache/Vulkan";
        }

        static void CreateCacheDirectoryIfNeeded()
        {
            auto cacheDirectory = GetCacheDirectory();
             // Don't create when already exists!
            if (std::filesystem::exists(cacheDirectory))
            {
                return;
            }

            std::filesystem::create_directory(cacheDirectory);
        }



    }

    ////////////////////////////////////////////////////////////////
    // VULKAN SHADER COMPILER //////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    VulkanShaderCompiler::VulkanShaderCompiler(const std::string &shaderAssetPath)
        : m_ShaderAssetPath(shaderAssetPath)
    {
    }

    bool VulkanShaderCompiler::TryReload()
    {
        m_ShaderSources.clear();

        Utils::CreateCacheDirectoryIfNeeded();
        const std::string source = FileSystem::ReadFileAndSkipBom(m_ShaderAssetPath);
        DODO_ASSERT(source.size() != 0, "Failed to load shader: {}!", m_ShaderAssetPath);

        m_ShaderSources = PreProcess(source);

        return false;
    }

    std::map<VkShaderStageFlagBits, std::string> VulkanShaderCompiler::PreProcess(const std::string& source)
    {
    }

}