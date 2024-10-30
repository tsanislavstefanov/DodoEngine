#pragma once

#include "VulkanAdapter.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanShader.h"
#include "VulkanShaderCompiler.h"
#include "VulkanSwapChain.h"
#include "Renderer/Renderer.h"

namespace Dodo {

    class VulkanRenderer : public Renderer
    {
    public:
        VulkanRenderer(RenderThread& renderThread, const Window& targetWindow, VSyncMode vsyncMode);
        ~VulkanRenderer();

        BufferHandle BufferCreate(BufferUsage bufferUsage, size_t size, void* data = nullptr) override;
        void BufferUploadData(BufferHandle bufferHandle, void* data, size_t size, size_t offset = 0) override;
        void BufferDestroy(BufferHandle bufferHandle) override;

        ShaderHandle ShaderCreate(const std::filesystem::path& assetPath) override;
        void ShaderReload(ShaderHandle shaderHandle, bool forceCompile = false) override;
        void ShaderDestroy(ShaderHandle shaderHandle) override;

        void BeginFrame() override;
        void EndFrame() override;
        void OnResize(uint32_t width, uint32_t height) override;
        RendererType GetType() const { return RendererType::Vulkan; }

    private:
        Ref<VulkanInstance> m_Instance = nullptr;
        Ref<VulkanDevice> m_Device = nullptr;
        Ref<VulkanSwapChain> m_SwapChain = nullptr;
        VulkanShaderCompiler m_ShaderCompiler{};
    };

}