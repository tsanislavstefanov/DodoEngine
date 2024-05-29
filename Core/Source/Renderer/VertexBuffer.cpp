#include "pch.h"
#include "VertexBuffer.h"
#include "Renderer.h"
#include "Drivers/Vulkan/VulkanVertexBuffer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VERTEX BUFFER ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<VertexBuffer> VertexBuffer::Create(size_t size)
    {
        switch (Renderer::GetSettings().RendererApiType)
        {
            case RendererApiType::Vulkan: return Ref<VulkanVertexBuffer>::Create(size);
            default: DODO_ASSERT(false, "Renderer API type not supported!");
        }

        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(void* data, size_t size)
    {
        switch (Renderer::GetSettings().RendererApiType)
        {
            case RendererApiType::Vulkan: return Ref<VulkanVertexBuffer>::Create(data, size);
            default: DODO_ASSERT(false, "Renderer API type not supported!");
        }

        return nullptr;
    }

}