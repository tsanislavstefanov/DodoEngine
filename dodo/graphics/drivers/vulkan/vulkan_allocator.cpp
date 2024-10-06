#include "pch.h"
#include "vulkan.h"
#include "vulkan_allocator.h"

namespace Dodo {

    VulkanAllocator::VulkanAllocator(VulkanAllocatorSpecifications&& specs)
    {
        VmaAllocatorCreateInfo allocatorCreateInfo{};
        allocatorCreateInfo.vulkanApiVersion = specs.api_version;
        allocatorCreateInfo.instance = specs.instance;
        allocatorCreateInfo.physicalDevice = specs.physical_device;
        allocatorCreateInfo.device = specs.device;
        DODO_VERIFY_VK_RESULT(vmaCreateAllocator(&allocatorCreateInfo, &m_Allocator));
    }

    VulkanAllocator::~VulkanAllocator()
    {
        vmaDestroyAllocator(m_Allocator);
    }

    VmaAllocation VulkanAllocator::allocate(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage, VkBuffer* buffer)
    {
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = usage;
        VmaAllocation result = VK_NULL_HANDLE;
        DODO_VERIFY_VK_RESULT(vmaCreateBuffer(m_Allocator, &bufferCreateInfo, &allocationCreateInfo, buffer, &result, VK_NULL_HANDLE));
        return result;
    }

    void VulkanAllocator::free(VkBuffer buffer, VmaAllocation allocation)
    {
        vmaDestroyBuffer(m_Allocator, buffer, allocation);
    }

    void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
    {
        vmaUnmapMemory(m_Allocator, allocation);
    }

    void VulkanAllocator::free(VmaAllocation allocation)
    {
        vmaFreeMemory(m_Allocator, allocation);
    }

}