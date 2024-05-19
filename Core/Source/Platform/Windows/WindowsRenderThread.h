#pragma once

#include "Core/RenderThread.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOWS RENDER THREAD ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class WindowsRenderThread : public RenderThread
    {
    public:
        WindowsRenderThread(RenderThreadPolicy policy);

        ~WindowsRenderThread() override;

        // Inherited via [RenderThread].
        void Wait(RenderThreadState state) override;

        void Update(RenderThreadState state) override;

        void WaitAndUpdate(RenderThreadState waitForState, RenderThreadState newState) override;

    private:
        ////////////////////////////////////////////////////////////
        // RENDER THREAD DATA //////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct RenderThreadData
        {
            // Prefer critical section because it is faster than mutex.
            // Link: https://learn.microsoft.com/en-us/windows/win32/sync/critical-section-objects
            CRITICAL_SECTION   CriticalSection{};
            CONDITION_VARIABLE ConditionVar   {};
        };

        RenderThreadData m_Data{};
    };

}