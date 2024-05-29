#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "Renderer/VertexBuffer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VERTEX BUFFER ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanVertexBuffer : public VertexBuffer
    {
    public:
        VulkanVertexBuffer(size_t size);
        VulkanVertexBuffer(void* data, size_t size);

        void SetData(void* data, size_t size, uint32_t offset);

    private:
        size_t m_Size;
        Buffer m_LocalData = nullptr;
        VmaAllocation m_MemoryAllocation = nullptr;
        VkBuffer m_Buffer = nullptr;
    };

}