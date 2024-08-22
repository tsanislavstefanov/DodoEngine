#include "pch.h"
#include "Platform.h"

#ifdef DODO_WINDOWS
#   include <processthreadsapi.h>
#endif

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // PLATFORM ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void Platform::SetThreadName(std::thread& thread, const std::string& name)
    {
#ifdef DODO_WINDOWS
        const auto handle = reinterpret_cast<HANDLE>(thread.native_handle());
        const std::wstring description(name.begin(), name.end());
        SetThreadDescription(handle, description.c_str());
#endif
    }

    void Platform::SetThreadAffinity(std::thread& thread, uint64_t affinity)
    {
#ifdef DODO_WINDOWS
        const auto handle = reinterpret_cast<HANDLE>(thread.native_handle());
        const DWORD_PTR mask = 1 << affinity;
        SetThreadAffinityMask(handle, mask);
#endif
    }

}