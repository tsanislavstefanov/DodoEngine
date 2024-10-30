#pragma once

#include "Buffer.h"
#include "RenderThread.h"
#include "Shader.h"
#include "VSyncMode.h"
#include "Core/window.h"

namespace Dodo {

    enum class RendererType
    {
        Vulkan,
        AutoCount,
        None
    };

    class Renderer : public RefCounted
    {
    public:
        static Ref<Renderer> Create(RenderThread& renderThread, RendererType type, const Window& targetWindow, VSyncMode vsyncMode = VSyncMode::Enabled);

        Renderer(RenderThread& renderThread);
        virtual ~Renderer() = default;

        void Submit(RenderCommand&& command);

        virtual BufferHandle BufferCreate(BufferUsage bufferUsage, size_t size, void* data = nullptr) = 0;
        virtual void BufferUploadData(BufferHandle bufferHandle, void* data, size_t size, size_t offset = 0) = 0;
        virtual void BufferDestroy(BufferHandle bufferHandle) = 0;

        virtual ShaderHandle ShaderCreate(const std::filesystem::path& assetPath) = 0;
        virtual void ShaderReload(ShaderHandle shaderHandle, bool forceCompile = false) = 0;
        virtual void ShaderDestroy(ShaderHandle shaderHandle) = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
        virtual RendererType GetType() const = 0;

    private:
        RenderThread& m_RenderThread;
    };

}