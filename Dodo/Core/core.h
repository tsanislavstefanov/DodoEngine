#pragma once

#ifdef _WIN32
#   ifndef DODO_WINDOWS
#      define DODO_WINDOWS
#   endif
#else
#   error "Platform not supported!"
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
#   ifdef DODO_WINDOWS
#       define DODO_DEBUG_BREAK() __debugbreak()
#   endif
#endif

#if defined(DODO_WINDOWS) || defined(DODO_LINUX) || defined(DODO_MAC_OS)
#   ifndef DODO_USE_VULKAN
#       define DODO_USE_VULKAN
#   endif
#endif

#ifdef DODO_WINDOWS
#   ifndef DODO_USE_DIRECT_X
#        define DODO_USE_DIRECT_X
#   endif
#endif

#ifdef DODO_WINDOWS
#   ifndef DODO_USE_METAL
#        define DODO_USE_METAL
#   endif
#endif

////////////////////////////////////////////////////////////////
// ASSERTION ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#ifdef DODO_DEBUG
#   include "Diagnostics/Log.h"
#   define DODO_ASSERT(CONDITION, ...) if (!(CONDITION)) { DODO_LOG_ERROR(__VA_ARGS__); DODO_DEBUG_BREAK(); }
#   define DODO_VERIFY(CONDITION) if (!(CONDITION)) { DODO_DEBUG_BREAK(); }
#else
#   define DODO_ASSERT(...)
#   define DODO_VERIFY(...)
#endif