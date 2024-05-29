#include "pch.h"
#include "VulkanVertexBuffer.h"
#include "Vulkan.h"
#include "VulkanAllocator.h"
#include "VulkanContext.h"
#include "Renderer/Renderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN VERTEX BUFFER ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    VulkanVertexBuffer::VulkanVertexBuffer(size_t size)
        : m_Size(size)
    {
        m_LocalData.Allocate(m_Size);
        
        Ref<VulkanVertexBuffer> instance = this;
        Renderer::Schedule([instance]() mutable
        {
            VkBufferCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            createInfo.size  = instance->m_Size;
            createInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            VulkanAllocator allocator{};
            instance->m_MemoryAllocation = allocator.AllocateBuffer(createInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &instance->m_Buffer);
        });
    }

    VulkanVertexBuffer::VulkanVertexBuffer(void* data, size_t size)
        : m_Size(size)
    {
        m_LocalData = Buffer::Copy(data, m_Size);

        Ref<VulkanVertexBuffer> instance = this;
        Renderer::Schedule([instance]() mutable
        {
            VulkanAllocator allocator{};
            VmaAllocation stagingBufferAllocation = nullptr;
            VkBuffer stagingBuffer = VK_NULL_HANDLE;
            // Create staging buffer.
            {
                VkBufferCreateInfo createInfo{};
                createInfo.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                createInfo.size         = instance->m_Size;
                createInfo.usage        = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                createInfo.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;
                stagingBufferAllocation = allocator.AllocateBuffer(createInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &stagingBuffer);
            }

            // Copy local data to staging buffer.
            uint8_t* destinationData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
            std::memcpy(destinationData, instance->m_LocalData.Data, instance->m_LocalData.Size);
            allocator.UnmapMemory(stagingBufferAllocation);

            // Create GPU local vertex buffer.
            {
                VkBufferCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                createInfo.size  = instance->m_Size;
                createInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                instance->m_MemoryAllocation = allocator.AllocateBuffer(createInfo, VMA_MEMORY_USAGE_GPU_ONLY, &instance->m_Buffer);
            }

            // Upload staging buffer to GPU.
            VkCommandBuffer commandBuffer = VulkanContext::GetCurrent().AllocateThreadLocalCommandBuffer(true);
            VkBufferCopy copyRegion{};
            copyRegion.size = instance->m_LocalData.Size;
            vkCmdCopyBuffer(commandBuffer, stagingBuffer, instance->m_Buffer, 1, &copyRegion);
            VulkanContext::GetCurrent().FlushCommandBuffer(commandBuffer);
            allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
        });
    }

    void VulkanVertexBuffer::SetData(void* data, size_t size, uint32_t offset)
    {
        std::memcpy(m_LocalData.Data, (uint8_t*)data + offset, size);
        Ref<VulkanVertexBuffer> instance = this;
        Renderer::Schedule([instance, data, size, offset]() mutable
        {
            VulkanAllocator allocator{};
            uint8_t* destinationData = allocator.MapMemory<uint8_t>(instance->m_MemoryAllocation);
            std::memcpy(destinationData, (uint8_t*)data + offset, size);
            allocator.UnmapMemory(instance->m_MemoryAllocation);
        });
    }
    
}