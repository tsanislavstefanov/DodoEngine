#include "pch.h"
#include "WindowsThread.h"

namespace Dodo {

    WindowsThread::WindowsThread(const std::string& name, ThreadAffinity affinity)
    {
        const HANDLE handle = static_cast<HANDLE>(m_Thread.native_handle());
        const std::wstring description(name.begin(), name.end());
        SetThreadDescription(handle, description.c_str());
        if (affinity != ThreadAffinity::None)
        {
            const DWORD_PTR mask = static_cast<DWORD_PTR>(1) << static_cast<uint64_t>(affinity);
            SetThreadAffinityMask(handle, mask);
        }
    }

}