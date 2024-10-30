#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanAllocator.h"

namespace Dodo {

    VulkanAllocator::VulkanAllocator(Ref<VulkanInstance> instance, Ref<VulkanDevice> device)
    {
        VmaAllocatorCreateInfo allocatorCreateInfo{};
        allocatorCreateInfo.vulkanApiVersion = instance->GetAPIVersion();
        allocatorCreateInfo.instance = instance->GetVulkanInstance();
        allocatorCreateInfo.physicalDevice = device->GetAdapter()->GetVulkanPhysicalDevice();
        allocatorCreateInfo.device = device->GetVulkanDevice();
        DODO_VERIFY_VK_RESULT(vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator));
    }

    VulkanAllocator::~VulkanAllocator()
    {
        vmaDestroyAllocator(m_Allocator);
    }

    VmaAllocation VulkanAllocator::Allocate(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage, VkBuffer* buffer) const
    {
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = usage;
        VmaAllocation result = VK_NULL_HANDLE;
        DODO_VERIFY_VK_RESULT(vmaCreateBuffer(m_Allocator, &bufferCreateInfo, &allocationCreateInfo, buffer, &result, VK_NULL_HANDLE));
        return result;
    }

    void VulkanAllocator::UnmapMemory(VmaAllocation allocation) const
    {
        vmaUnmapMemory(m_Allocator, allocation);
    }

    void VulkanAllocator::Destroy(VkBuffer buffer, VmaAllocation allocation) const
    {
        vmaDestroyBuffer(m_Allocator, buffer, allocation);
    }

    void VulkanAllocator::Destroy(VmaAllocation allocation) const
    {
        vmaFreeMemory(m_Allocator, allocation);
    }

}