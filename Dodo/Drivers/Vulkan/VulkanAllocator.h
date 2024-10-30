#pragma once

#include "VulkanInstance.h"
#include "VulkanDevice.h"

namespace Dodo {

    class VulkanAllocator : public RefCounted
    {
    public:
        VulkanAllocator(Ref<VulkanInstance> instance, Ref<VulkanDevice> device);
        ~VulkanAllocator();

        VmaAllocation Allocate(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage, VkBuffer* buffer) const;

        template<typename T>
        T* MapMemory(VmaAllocation allocation);
        void UnmapMemory(VmaAllocation allocation) const;
        void Destroy(VkBuffer buffer, VmaAllocation allocation) const;
        void Destroy(VmaAllocation allocation) const;
    private:
        VmaAllocator m_Allocator = nullptr;
    };

    template <typename T>
    inline T* VulkanAllocator::MapMemory(VmaAllocation allocation)
    {
        T* mappedMemory = nullptr;
        vmaMapMemory(m_Allocator, allocation, reinterpret_cast<void**>(&mappedMemory));
        return mappedMemory;
    }

}