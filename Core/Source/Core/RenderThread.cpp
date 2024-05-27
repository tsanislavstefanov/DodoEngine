#include "pch.h"
#include "RenderThread.h"
#include "Platform.h"
#include "Renderer/Renderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void RenderThread::Run()
    {
        m_IsRunning = true;
        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
        {
            m_Thread = std::thread(Renderer::RenderThreadProc, this);
            Platform::SetThreadName(m_Thread, "Render Thread");
        }
    }

    void RenderThread::Pump()
    {
        NextFrame();
        BlockUntilRenderComplete();
    }

    void RenderThread::NextFrame()
    {
        Renderer::SwapQueues();
        Kick();
    }

    void RenderThread::Kick()
    {
        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
        {
            Update(RenderThreadState::Kick);
            return;
        }

        Renderer::WaitAndRender(this);
    }

    void RenderThread::BlockUntilRenderComplete()
    {
        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
        {
            Wait(RenderThreadState::Idle);
        }
    }

    void RenderThread::Wait(RenderThreadState state)
    {
        if (m_ThreadingPolicy == ThreadingPolicy::SingleThreaded)
        {
            return;
        }

        std::unique_lock<std::mutex> lock(m_Mutex);
        m_ConditionVar.wait(lock, [this, state]() { return m_State == state; });
    }

    void RenderThread::Update(RenderThreadState state)
    {
        if (m_ThreadingPolicy == ThreadingPolicy::SingleThreaded)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);
        m_State = state;
        m_ConditionVar.notify_all();
    }

    void RenderThread::WaitAndUpdate(RenderThreadState waitForState, RenderThreadState newState)
    {
        if (m_ThreadingPolicy == ThreadingPolicy::SingleThreaded)
        {
            return;
        }

        std::unique_lock<std::mutex> lock(m_Mutex);
        m_ConditionVar.wait(lock, [this, waitForState]() { return m_State == waitForState; });
        m_State = newState;
        m_ConditionVar.notify_all();
    }

    void RenderThread::Stop()
    {
        m_IsRunning = false;
        if (m_ThreadingPolicy == ThreadingPolicy::MultiThreaded)
        {
            m_Thread.join();
        }
    }

}