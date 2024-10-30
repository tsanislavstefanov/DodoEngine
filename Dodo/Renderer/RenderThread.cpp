#include "pch.h"
#include "RenderThread.h"

namespace Dodo {

    void RenderThread::Start(RenderThreadPolicy threadPolicy)
    {
        m_ThreadPolicy = threadPolicy;
        m_IsRunning = true;
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            m_Thread = Thread::Create("Render Thread", ThreadAffinity::RenderThread);
            m_Thread->Dispatch(&RenderThread::Main, this);
        }
    }

    void RenderThread::WaitUntilRenderComplete()
    {
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
            WaitForState(State::Idle);
    }

    void RenderThread::Submit(RenderCommand&& command)
    {
        RenderCommandQueue& submissionQueue = m_CommandQueues.at(m_SubmissionQueueIndex);
        submissionQueue.Enqueue(std::forward<RenderCommand>(command));
    }

    void RenderThread::Flush()
    {
        m_SubmissionQueueIndex = (m_SubmissionQueueIndex + 1) % m_CommandQueues.size();
        Kick();
    }

    void RenderThread::Stop()
    {
        m_IsRunning = false;
        Pump();
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
            m_Thread->Join();
    }

    void RenderThread::Main(RenderThread* renderThread)
    {
        while (renderThread->IsRunning())
            renderThread->Render();
    }

    void RenderThread::WaitForState(State state)
    {
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_ConditionVar.wait(lock, [this, state]() -> bool { return m_State == state; });
        }
    }

    void RenderThread::TransitionToState(State state)
    {
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_State = state;
            m_ConditionVar.notify_all();
        }
    }

    void RenderThread::Kick()
    {
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
            TransitionToState(State::Kick);
        else
            Render();
    }

    void RenderThread::Pump()
    {
        Flush();
        WaitUntilRenderComplete();
    }

    void RenderThread::Render()
    {
        Stopwatch waitStopwatch{};
        WaitForState(State::Kick);
        m_PerformanceStats.WaitTime = waitStopwatch.GetMilliseconds();
        TransitionToState(State::Busy);

        RenderCommandQueue& renderQueue = m_CommandQueues.at(GetRenderQueueIndex());
        Stopwatch workStopwatch{};
        renderQueue.Execute();
        // Render done!
        m_PerformanceStats.WorkTime = workStopwatch.GetMilliseconds();
        TransitionToState(State::Idle);

        m_PerformanceStats.ExecutedCommandCount = renderQueue.GetCommandCount();
        renderQueue.Clear();
    }

}