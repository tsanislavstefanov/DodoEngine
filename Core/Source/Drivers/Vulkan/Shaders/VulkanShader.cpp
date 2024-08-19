#include "pch.h"
#include "VulkanShader.h"
#include "VulkanShaderCompiler.h"
#include "Renderer/RenderThread.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN SHADER ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    VulkanShader::VulkanShader(const std::string& assetPath)
        : m_AssetPath(assetPath)
    {
        Reload();
    }

    void VulkanShader::Reload()
    {
        RenderThread::Submit([instance = Ref<VulkanShader>(this)]() mutable {
            instance->Reload_RenderThread();
        });
    }

    void VulkanShader::Reload_RenderThread()
    {
        VulkanShaderCompiler compiler(m_AssetPath);
        if (!compiler.TryReload())
        {
            // TODO: log or assert error.
        }
    }

}