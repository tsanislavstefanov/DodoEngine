#pragma once

#if defined(_WIN32)
#   ifndef DODO_WINDOWS
#      define DODO_WINDOWS
#   endif
#elif defined(__linux__)
#   ifndef DODO_LINUX
#       define DODO_LINUX
#   endif
#else
#   error "Platform not supported!"
#endif

#if defined(DODO_WINDOWS) || defined(DODO_LINUX)
#   ifndef DODO_VULKAN
#       define DODO_VULKAN
#   endif
#endif

#ifdef _DEBUG
#   ifndef DODO_DEBUG
#       define DODO_DEBUG
#   endif
#else
#   ifndef DODO_RELEASE
#      define DODO_RELEASE
#   endif
#endif

#ifdef DODO_DEBUG
#   ifndef DODO_ENABLE_ASSERT
#       define DODO_ENABLE_ASSERT
#   endif
#   ifdef DODO_WINDOWS
#       ifndef DODO_DEBUG_BREAK
#           define DODO_DEBUG_BREAK() __debugbreak()
#       endif
#   else
#       ifndef DODO_DEBUG_BREAK
#           define DODO_DEBUG_BREAK()
#       endif
#   endif
#endif

#ifdef DODO_ENABLE_ASSERT
#   ifndef DODO_ASSERT
#       define DODO_ASSERT(CONDITION) if (!(CONDITION)) { DODO_DEBUG_BREAK(); }
#   endif
#else
#   ifndef DODO_ASSERT
#       define DODO_ASSERT(...)
#   endif
#endif

#include <cstddef>

constexpr size_t operator"" _kb(size_t p_bytes) {
    return p_bytes * 1024;
}

constexpr size_t operator"" _mb(size_t p_bytes) {
    return p_bytes * 1024 * 1024;
}
