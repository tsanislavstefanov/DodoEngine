#pragma once

#include <vulkan/vulkan.h>
#ifndef DODO_VK_RESULT
#   include "core/core.h"
#   ifdef DODO_DEBUG
#       define DODO_VERIFY_VK_RESULT(VK_RESULT) \
            DODO_VERIFY((VK_RESULT) == VK_SUCCESS)
#   else
#       define DODO_VK_RESULT(...)
#   endif
#endif

namespace Dodo { namespace Utils {

    static std::string stringify_vulkan_version(uint32_t version)
    {
        return std::format("{0}.{1}.{2}",
                           VK_API_VERSION_MAJOR(version),
                           VK_API_VERSION_MINOR(version),
                           VK_API_VERSION_PATCH(version));
    }

}}