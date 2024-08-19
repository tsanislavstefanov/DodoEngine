#include "pch.h"
#define VMA_IMPLEMENTATION
#include "VulkanAllocator.h"
#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanRenderContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN ALLOCATOR ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    
    VulkanAllocatorData* VulkanAllocator::s_Data = nullptr;

    void VulkanAllocator::Init()
    {
        s_Data = new VulkanAllocatorData();
        const Ref<VulkanDevice>& device = VulkanRenderContext::GetCurrentDevice();

        // Create (V)ulkan (M)emory (A)llocator.
        VmaAllocatorCreateInfo createInfo{};
        createInfo.physicalDevice   = device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
        createInfo.device           = device->GetVulkanDevice();
        createInfo.instance         = VulkanRenderContext::GetVulkanInstance();
        createInfo.vulkanApiVersion = VulkanRenderContext::GetVulkanApiVersion();
        DODO_VK_RESULT(vmaCreateAllocator(&createInfo, &s_Data->Allocator));
    }

    void VulkanAllocator::Destroy()
    {
        vmaDestroyAllocator(s_Data->Allocator);
        delete s_Data;
        s_Data = nullptr;
    }

    VmaAllocation VulkanAllocator::AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer *buffer)
    {
        VmaAllocationCreateInfo createInfo{};
        createInfo.usage         = usage;
        VmaAllocation allocation = nullptr;
        DODO_VK_RESULT(vmaCreateBuffer(s_Data->Allocator, &bufferCreateInfo, &createInfo, buffer, &allocation, nullptr));
        return allocation;
    }

    void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
    {
        vmaDestroyBuffer(s_Data->Allocator, buffer, allocation);
    }

    void VulkanAllocator::UnmapMemory(VmaAllocation allocation)
    {
        vmaUnmapMemory(s_Data->Allocator, allocation);
    }

    void VulkanAllocator::Free(VmaAllocation allocation)
    {
        vmaFreeMemory(s_Data->Allocator, allocation);
    }

}