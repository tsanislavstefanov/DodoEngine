#include "pch.h"
#include "VulkanAllocator.h"
#include "Vulkan.h"
#include "VulkanContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN ALLOCATOR ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    
    VulkanAllocatorData* VulkanAllocator::s_Data = nullptr;

    void VulkanAllocator::Init()
    {
        s_Data = new VulkanAllocatorData();

        const auto& device = VulkanContext::GetCurrentDevice();
        VmaAllocatorCreateInfo createInfo{};
        createInfo.vulkanApiVersion = VulkanContext::GetCurrent().GetVulkanApiVersion();
        createInfo.instance         = VulkanContext::GetCurrentInstance();
        createInfo.physicalDevice   = device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
        createInfo.device           = device->GetVulkanDevice();
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
        createInfo.usage = usage;
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

}