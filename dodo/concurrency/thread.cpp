#include "pch.h"
#include "thread.h"
#ifdef DODO_WINDOWS
#   include "platforms/windows/windows_thread.h"
#endif

namespace Dodo {

    Ref<Thread> Thread::create(
            const std::string& name,
            ThreadAffinity affinity /* = ThreadAffinity::none */)
    {
#ifdef DODO_WINDOWS
        return Ref<WindowsThread>::create(name, affinity);
#endif
    }

    void Thread::join()
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
    }

}