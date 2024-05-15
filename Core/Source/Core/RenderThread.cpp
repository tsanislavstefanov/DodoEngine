#include "pch.h"
#include "RenderThread.h"

namespace Dodo {

    RenderThread::RenderThread(ThreadPolicy policy)
        : m_ThreadPolicy(policy)
    {}

    void RenderThread::Run()
    {
        m_IsRunning = true;
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {

        }
    }

    void RenderThread::Dispatch()
    {

    }

}