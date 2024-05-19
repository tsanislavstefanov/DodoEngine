#include "pch.h"
#include "WindowsRenderThread.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOWS RENDER THREAD ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    WindowsRenderThread::WindowsRenderThread(RenderThreadPolicy policy)
        :
        RenderThread(policy)
    {
        if (policy == RenderThreadPolicy::MultiThreaded)
        {
            InitializeCriticalSection  (&m_Data.CriticalSection);
            InitializeConditionVariable(&m_Data.ConditionVar   );
        }
    }

    WindowsRenderThread::~WindowsRenderThread()
    {
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            DeleteCriticalSection(&m_Data.CriticalSection);
        }
    }

    void WindowsRenderThread::Wait(RenderThreadState state)
    {
        if (m_ThreadPolicy == RenderThreadPolicy::SingleThreaded)
        {
            return;
        }

        EnterCriticalSection(&m_Data.CriticalSection);
        while (m_State != state)
        {
            // Sleeps on the specified condition variable and releases the specified
            // critical section so that another thread can wake it.
            SleepConditionVariableCS(&m_Data.ConditionVar, &m_Data.CriticalSection, INFINITE);
        }

        LeaveCriticalSection(&m_Data.CriticalSection);
    }

    void WindowsRenderThread::Update(RenderThreadState state)
    {
        if (m_ThreadPolicy == RenderThreadPolicy::SingleThreaded)
        {
            return;
        }

        EnterCriticalSection(&m_Data.CriticalSection);
        m_State = state;
        WakeAllConditionVariable(&m_Data.ConditionVar);
        LeaveCriticalSection(&m_Data.CriticalSection);
    }

    void WindowsRenderThread::WaitAndUpdate(RenderThreadState waitForState, RenderThreadState newState)
    {
        if (m_ThreadPolicy == RenderThreadPolicy::SingleThreaded)
        {
            return;
        }

        EnterCriticalSection(&m_Data.CriticalSection);
        while (m_State != waitForState)
        {
            // Sleeps on the specified condition variable and releases the specified
            // critical section so that another thread can wake it.
            SleepConditionVariableCS(&m_Data.ConditionVar, &m_Data.CriticalSection, INFINITE);
        }

        m_State = newState;
        WakeAllConditionVariable(&m_Data.ConditionVar);
        LeaveCriticalSection(&m_Data.CriticalSection);
    }
}