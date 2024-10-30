#pragma once

#include <cstdint>

#include "VulkanUtils.h"

namespace Dodo {

    struct VulkanBuffer
    {
        size_t Size = 0;
        VkBuffer Buffer = VK_NULL_HANDLE;
        VmaAllocation MemoryAllocation = nullptr;
    };

}