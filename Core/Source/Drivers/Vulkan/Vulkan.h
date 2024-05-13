#pragma once

#include "Core/Core.h"

#ifdef ENABLE_ASSERTION
#   ifndef ASSERT_VK_RESULT
#      define ASSERT_VK_RESULT(VK_RESULT, ...) ASSERT(VK_RESULT == VK_SUCCESS, __VA_ARGS__)
#   endif
#   ifndef VERIFY_VK_RESULT
#      define VERIFY_VK_RESULT(VK_RESULT) VERIFY(VK_RESULT == VK_SUCCESS)
#   endif
#endif