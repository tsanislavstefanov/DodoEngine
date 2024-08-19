#include "pch.h"
#include "RenderThread.h"
#include "Core/Platform.h"
#include "Diagnostics/Stopwatch.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    RenderThread* RenderThread::s_Instance = nullptr;

    RenderThread::RenderThread(ThreadPolicy threadPolicy)
        : m_ThreadPolicy(threadPolicy)
    {
        DODO_ASSERT(!s_Instance, "RenderThread instance already exists!");
        s_Instance = this;
        
        // Create command queues.
        for (size_t i = 0; i < MaxCommandQueueCount; i++)
        {
            m_CommandQueues.emplace_back();
        }
    }

    void RenderThread::Dispatch()
    {
        m_IsRunning = true;
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            m_Thread = std::thread(RenderThread::RenderProc, this);
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
        SwapQueues();
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

    void RenderThread::RenderProc(RenderThread* renderThread)
    {
        while (renderThread->IsRunning())
        {
            WaitAndRender(renderThread);
        }
    }

    void RenderThread::WaitAndRender(RenderThread* renderThread)
    {
        auto& performanceStats = renderThread->m_PerformanceStats;
        {
            Stopwatch stopwatch{};
            renderThread->WaitAndUpdate(State::Kick, State::Busy);
            performanceStats.WaitTime = stopwatch.GetAsMilliseconds();
        }

        Stopwatch stopwatch{};
        auto& renderQueue = renderThread->GetRenderQueue();
        renderQueue.Execute();
        // Render done!
        renderThread->Update(State::Idle);
        performanceStats.WorkTime = stopwatch.GetAsMilliseconds();
    }

    void RenderThread::Kick()
    {
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            Update(State::Kick);
        }
        else
        {
            WaitAndRender(this);
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

    void RenderThread::SwapQueues()
    {
        m_SubmissionQueueIndex = (m_SubmissionQueueIndex + 1) % MaxCommandQueueCount;
    }

}