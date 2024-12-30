#pragma once

#ifdef _WIN32
#   ifndef DODO_WINDOWS
#      define DODO_WINDOWS
#   endif
#else
#   error "Platform not supported!"
#endif

#ifdef DODO_WINDOWS
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
#   endif
#endif

////////////////////////////////////////////////////////////////
// ASSERTION ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#ifdef DODO_ENABLE_ASSERT
#   ifndef DODO_ASSERT
#       define DODO_ASSERT(CONDITION) if (!(CONDITION)) { DODO_DEBUG_BREAK(); }
#   endif
#else
#   ifndef DODO_ASSERT
#       define DODO_ASSERT(...)
#   endif
#endif

constexpr size_t operator"" _mb(size_t mb) {
    // 1 MB = 1024 * 1024 bytes.
    return mb * 1024 * 1024;
}

constexpr size_t operator"" _kb(size_t kb) {
    // 1 KB = 1024 bytes.
    return kb * 1024;
}