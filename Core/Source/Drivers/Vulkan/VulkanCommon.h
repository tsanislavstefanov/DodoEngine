#pragma once

#include <vulkan/vulkan.h>

#include "Core/Core.h"

////////////////////////////////////////////////////////////////
// DEBUG UTILS /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#ifndef DODO_VK_RESULT
#   ifdef DODO_DEBUG
#       define DODO_VK_RESULT(VK_RESULT) DODO_VERIFY((VK_RESULT) == VK_SUCCESS)
#   else
#       define DODO_VK_RESULT(...)
#   endif
#endif

////////////////////////////////////////////////////////////////
// LOADER UTILS ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#define GET_VK_INSTANCE_PROC_ADDR(INSTANCE, NAME)                         \
    pfn##NAME = (PFN_vk##NAME)vkGetInstanceProcAddr(INSTANCE, "vk"#NAME); \
    DODO_VERIFY(pfn##NAME);

#define GET_VK_DEVICE_PROC_ADDR(DEVICE, NAME)                         \
    pfn##NAME = (PFN_vk##NAME)vkGetDeviceProcAddr(DEVICE, "vk"#NAME); \
    DODO_VERIFY(pfn##NAME);