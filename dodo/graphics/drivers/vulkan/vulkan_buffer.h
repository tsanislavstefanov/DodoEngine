#pragma once

#include <vma/vk_mem_alloc.h>

#include "graphics/buffer_usage.h"

namespace Dodo {

    struct VulkanBufferInfo
    {
        size_t size = 0;
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = nullptr;
    };

    namespace Utils {

        static VkBufferUsageFlagBits convert_to_vulkan_buffer_usage(BufferUsage usage)
        {
            switch (usage)
            {
                case BufferUsage::vertex_buffer: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                default: return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
            }
        }

    }

}