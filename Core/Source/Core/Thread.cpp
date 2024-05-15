#include "pch.h"
#include "Thread.h"
#ifdef PLATFORM_WINDOWS
#   include "Platform/Windows/WindowsThread.h"
#endif

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // THREAD //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<Thread> Thread::Create(std::string name)
    {
#ifdef PLATFORM_WINDOWS
        return Ref<WindowsThread>::Create(std::move(name));
#endif
        ASSERT(false, "Platform not supported!");
    }

}