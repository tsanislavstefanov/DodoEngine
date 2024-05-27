#pragma once

#include "ThreadingPolicy.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD STATE /////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    enum class RenderThreadState
    {
        Idle     ,
        Kick     ,
        Busy     ,
        AutoCount,
        None
    };

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderThread
    {
    public:
        RenderThread(ThreadingPolicy threadingPolicy)
            : m_ThreadingPolicy(threadingPolicy)
        {}

        bool IsRunning() const
        {
            return m_IsRunning;
        }

        void Run ();
        void Pump();
        void NextFrame();
        void Kick();
        void BlockUntilRenderComplete();
        void Wait(RenderThreadState state);
        void Update(RenderThreadState state);
        void WaitAndUpdate(RenderThreadState waitForState, RenderThreadState newState);
        void Stop();

    private:
        ThreadingPolicy m_ThreadingPolicy = ThreadingPolicy::None;
        RenderThreadState m_State = RenderThreadState::None;
        bool m_IsRunning = false;
        std::thread m_Thread{};
        std::mutex  m_Mutex {};
        std::condition_variable m_ConditionVar{};
    };

}