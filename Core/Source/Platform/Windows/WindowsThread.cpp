#include "pch.h"
#include "WindowsThread.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOWS THREAD //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void WindowsThread::Join()
    {
        m_Thread.join();
    }

    void WindowsThread::SetName()
    {
        const auto handle = reinterpret_cast<HANDLE>(m_Thread.native_handle());
        std::wstring name(m_Name.begin(), m_Name.end());
        SetThreadDescription (handle, name.c_str());
        SetThreadAffinityMask(handle, 8);
    }

}