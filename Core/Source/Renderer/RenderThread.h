#pragma once

#include "Core/ThreadPolicy.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderThread
    {
    public:
        RenderThread(ThreadPolicy threadPolicy)
            : m_ThreadPolicy(threadPolicy)
        {}

        void Dispatch();
        void Pump();
        void BlockUntilRenderComplete();
        void NextFrame();
        void Stop();
        void Kick();

        ////////////////////////////////////////////////////////////
        // STATE ///////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        enum class State
        {
            Idle,
            Kick,
            Busy,
            AutoCount,
            None
        };

        void Wait(State state);
        void Update(State state);
        void WaitAndUpdate(State waitForState, State newState);

        bool IsRunning() const
        {
            return m_IsRunning;
        }

    private:
        ThreadPolicy m_ThreadPolicy = ThreadPolicy::None;
        bool m_IsRunning = false;
        std::thread m_Thread{};
        State m_State = State::None;
        std::mutex m_Mutex{};
        std::condition_variable m_ConditionVar{};
    };

}