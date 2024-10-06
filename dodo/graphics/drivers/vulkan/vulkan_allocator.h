#pragma once

#include <vma/vk_mem_alloc.h>

namespace Dodo {

    struct VulkanAllocatorSpecifications
    {
        uint32_t api_version = 0;
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
    };

    class VulkanAllocator : public RefCounted
    {
    public:
        VulkanAllocator(VulkanAllocatorSpecifications&& specs);
        ~VulkanAllocator();

        VmaAllocation allocate(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage, VkBuffer* buffer);
        void free(VkBuffer buffer, VmaAllocation allocation);

        template<typename T>
        T* MapMemory(VmaAllocation allocation);
        void UnmapMemory(VmaAllocation allocation);
        void free(VmaAllocation allocation);
    private:
        VmaAllocator m_Allocator = nullptr;
    };

    template <typename T>
    inline T* VulkanAllocator::MapMemory(VmaAllocation allocation)
    {
        T* mappedMemory = nullptr;
        vmaMapMemory(m_Allocator, allocation, static_cast<void**>(&mappedMemory));
        return mappedMemory;
    }

}