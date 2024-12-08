#pragma once

#include "core/core.h"

#ifdef DODO_WINDOWS
#   define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#ifndef DODO_VERIFY_VK_RESULT
#   ifdef DODO_DEBUG
#       define DODO_VERIFY_VK_RESULT(VK_RESULT) DODO_VERIFY((VK_RESULT) == VK_SUCCESS)
#   else
#       define DODO_VERIFY_VK_RESULT(...)
#   endif
#endif

#ifndef DODO_GET_VK_INSTANCE_PROC_ADDR
#   define DODO_GET_VK_INSTANCE_PROC_ADDR(INSTANCE, NAME) \
        pfn##NAME = (PFN_vk##NAME)vkGetInstanceProcAddr(INSTANCE, "vk"#NAME); \
        DODO_VERIFY(pfn##NAME);
#endif

#ifndef DODO_GET_VK_DEVICE_PROC_ADDR
#   define DODO_GET_VK_DEVICE_PROC_ADDR(DEVICE, NAME) \
        pfn##NAME = (PFN_vk##NAME)vkGetDeviceProcAddr(DEVICE, "vk"#NAME); \
        DODO_VERIFY(pfn##NAME);
#endif

namespace Dodo::Utils {

    static std::string to_string_vulkan_version(uint32_t version) {
        return std::format(
            "{0}.{1}.{2}",
            VK_API_VERSION_MAJOR(version),
            VK_API_VERSION_MINOR(version),
            VK_API_VERSION_PATCH(version));
    }

}