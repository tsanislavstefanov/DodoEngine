#include "pch.h"
#include "Thread.h"
#ifdef DODO_WINDOWS
#   include "Platforms/Windows/WindowsThread.h"
#endif

namespace Dodo {

    Ref<Thread> Thread::Create(const std::string& name, ThreadAffinity affinity)
    {
#ifdef DODO_WINDOWS
        return Ref<WindowsThread>::Create(name, affinity);
#endif
    }

    void Thread::Join()
    {
        if (m_Thread.joinable())
            m_Thread.join();
    }

}