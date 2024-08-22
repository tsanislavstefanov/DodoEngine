#include "pch.h"
#include "RenderThread.h"
#include "Renderer.h"
#include "Core/Platform.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void RenderThread::Dispatch()
    {
        m_IsRunning = true;
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            m_Thread = std::thread(Run, this);
            Platform::SetThreadName(m_Thread, "Render Thread");
            Platform::SetThreadAffinity(m_Thread, 0);
        }
    }

    void RenderThread::Pump()
    {
        NextFrame();
        BlockUntilRenderComplete();
    }

    void RenderThread::BlockUntilRenderComplete()
    {
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            Wait(State::Idle);
        }
    }

    void RenderThread::NextFrame()
    {
        Renderer::SwapQueues();
        Kick();
    }

    void RenderThread::Stop()
    {
        m_IsRunning = false;
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            m_Thread.join();
        }
    }

    void RenderThread::Kick()
    {
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            Update(State::Kick);
        }
        else
        {
            Renderer::WaitAndRender(this);
        }
    }

    void RenderThread::Wait(State state)
    {
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_ConditionVar.wait(lock, [this, state]() { return m_State == state; });
        }
    }

    void RenderThread::Update(State state)
    {
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_State = state;
            m_ConditionVar.notify_all();
        }
    }

    void RenderThread::WaitAndUpdate(State waitForState, State newState)
    {
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_ConditionVar.wait(lock, [this, waitForState]() { return m_State == waitForState; });
            m_State = newState;
            m_ConditionVar.notify_all();
        }
    }

    void Run(RenderThread* renderThread)
    {
        if (renderThread->IsRunning())
        {
            Renderer::WaitAndRender(renderThread);
        }
    }

}