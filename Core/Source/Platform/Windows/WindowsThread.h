#pragma once

#include "Core/Thread.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOWS THREAD //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class WindowsThread : public Thread
    {
    public:
        WindowsThread(std::string name)
             : Thread(std::move(name))
        {}

        void Join() override;

    protected:
        void SetName() override;
    };

}