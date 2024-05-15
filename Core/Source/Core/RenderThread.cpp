#include "pch.h"
#include "RenderThread.h"
#include "Renderer/Renderer.h"

namespace Dodo {

    RenderThread::RenderThread(ThreadPolicy policy)
        : m_ThreadPolicy(policy)
        , m_Thread(Thread::Create("RenderThread"))
    {}

    void RenderThread::Run()
    {
        m_IsRunning = true;
        if (m_ThreadPolicy == ThreadPolicy::MultiThreaded)
        {
            m_Thread->Dispatch(Renderer::RenderThreadProc, this);
        }
    }

}