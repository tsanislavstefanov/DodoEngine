#pragma once

#include "Buffer.h"
#include "Shader.h"
#include "vsync_mode.h"

namespace Dodo {

    class RenderThread;
    class RenderWindow;

    class Renderer : public RefCounted
    {
    public:
        enum class Type
        {
            vulkan,
            auto_count,
            none
        };

        struct Specification
        {
            RenderThread& render_thread;
            Type type = Type::none;
            Ref<RenderWindow> target_window = nullptr;
            VSyncMode vsync_mode = VSyncMode::none;
        };

        static Ref<Renderer> create(const Specification& specification);

        virtual ~Renderer() = default;

        virtual BufferHandle BufferCreate(BufferUsage bufferUsage, size_t size, void* data = nullptr) = 0;
        virtual void BufferUploadData(BufferHandle bufferHandle, void* data, size_t size, size_t offset = 0) = 0;
        virtual void BufferDestroy(BufferHandle bufferHandle) = 0;

        virtual ShaderHandle ShaderCreate(const std::filesystem::path& assetPath) = 0;
        virtual void ShaderReload(ShaderHandle shaderHandle, bool forceCompile = false) = 0;
        virtual void ShaderDestroy(ShaderHandle shaderHandle) = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
        virtual Type GetType() const = 0;
    };

}