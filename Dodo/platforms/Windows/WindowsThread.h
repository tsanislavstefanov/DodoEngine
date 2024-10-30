#pragma once

#include "concurrency/Thread.h"

namespace Dodo {

    class WindowsThread : public Thread
    {
    public:
        WindowsThread(const std::string& name, ThreadAffinity affinity);
    };

}