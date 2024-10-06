#include "pch.h"
#include "windows_thread.h"

namespace Dodo {

    WindowsThread::WindowsThread(const std::string& name, ThreadAffinity affinity)
    {
        const auto handle = reinterpret_cast<HANDLE>(thread_.native_handle());
        const std::wstring description(name.begin(), name.end());
        SetThreadDescription(handle, description.c_str());

        if (affinity != ThreadAffinity::none)
        {
            const auto handle = reinterpret_cast<HANDLE>(thread_.native_handle());
            const DWORD_PTR mask = ((uint64_t)1 << (uint64_t)affinity);
            SetThreadAffinityMask(handle, mask);
        }
    }

}