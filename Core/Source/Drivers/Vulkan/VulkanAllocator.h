#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN ALLOCATOR DATA ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct VulkanAllocatorData
    {
        VmaAllocator Allocator = nullptr;
    };

    ////////////////////////////////////////////////////////////////
    // VULKAN ALLOCATOR ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanAllocator
    {
    public:
        static void Init();
        static void Destroy();

        VmaAllocation AllocateBuffer(VkBufferCreateInfo bufferCreateInfo, VmaMemoryUsage usage, VkBuffer* buffer);
        void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

        // VmaAllocation allocate_image(VkImageCreateInfo p_image_info, VmaMemoryUsage p_usage, VkImage* r_image);

        template<typename T>
        T* MapMemory(VmaAllocation allocation)
        {
            T* mappedMemory = nullptr;
            vmaMapMemory(s_Data->Allocator, allocation, reinterpret_cast<void**>(&mappedMemory));
            return mappedMemory;
        }

        void UnmapMemory(VmaAllocation allocation);
        void Free(VmaAllocation allocation);

        // void free(VmaAllocation p_allocation);
        // void destroy_image(VkImage p_image, VmaAllocation p_allocation);
    private:
        static VulkanAllocatorData* s_Data;
    };

}