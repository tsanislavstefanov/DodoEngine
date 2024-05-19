#pragma once

#include "Thread.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD POLICY ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    enum class RenderThreadPolicy
    {
        SingleThreaded,
        MultiThreaded ,
        AutoCount     ,
        None
    };

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD STATE /////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    enum class RenderThreadState
    {
        Idle     ,
        Busy     ,
        Kick     ,
        AutoCount,
        None
    };

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderThread : public RefCounted
    {
    public:
        static Ref<RenderThread> Create(RenderThreadPolicy policy);

        RenderThread(RenderThreadPolicy policy);

        bool IsRunning() const
        {
            return m_IsRunning;
        }

        void Run();

        void Pump();

        void BlockUntilRenderComplete();

        void NextFrame();

        void Kick();

        void Kill();

        virtual void Wait(RenderThreadState state) = 0;

        virtual void Update(RenderThreadState state) = 0;

        virtual void WaitAndUpdate(RenderThreadState waitForState, RenderThreadState newState) = 0;

    protected:
        RenderThreadPolicy m_ThreadPolicy = RenderThreadPolicy::None;
        RenderThreadState  m_State  = RenderThreadState::None;

    private:
        bool         m_IsRunning    = false;
        Ref<Thread>  m_Thread       = nullptr;
    };
}