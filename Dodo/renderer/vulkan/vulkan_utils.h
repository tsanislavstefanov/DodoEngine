#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#ifndef DODO_VERIFY_VK_RESULT
#   ifdef DODO_DEBUG
#       define DODO_ASSERT_VK_RESULT(VK_RESULT) DODO_ASSERT((VK_RESULT) == VK_SUCCESS)
#   else
#       define DODO_ASSERT_VK_RESULT(...)
#   endif
#endif

namespace Dodo::Utils {

    inline std::string to_string_vulkan_version(uint32_t version) {
        return std::format(
            "{0}.{1}.{2}",
            VK_API_VERSION_MAJOR(version),
            VK_API_VERSION_MINOR(version),
            VK_API_VERSION_PATCH(version));
    }

}