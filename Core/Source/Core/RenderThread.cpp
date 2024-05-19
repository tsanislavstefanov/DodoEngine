#include "pch.h"
#include "RenderThread.h"
#ifdef PLATFORM_WINDOWS
#   include "Platform/Windows/WindowsRenderThread.h"
#endif
#include "Renderer/Renderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<RenderThread> RenderThread::Create(RenderThreadPolicy policy)
    {
#ifdef PLATFORM_WINDOWS
        return Ref<WindowsRenderThread>::Create(policy);
#endif
        ASSERT(false, "Platform not supported!");
    }

    RenderThread::RenderThread(RenderThreadPolicy policy)
        :
        m_ThreadPolicy(policy)
    {
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            m_Thread = Thread::Create("Render Thread");
        }
    }

    void RenderThread::Run()
    {
        m_IsRunning = true;
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            m_Thread->Dispatch(Renderer::RenderThreadProc, this);
        }
    }

    void RenderThread::Pump()
    {
        NextFrame();
        Kick();
        BlockUntilRenderComplete();
    }

    void RenderThread::BlockUntilRenderComplete()
    {
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            Wait(RenderThreadState::Idle);
        }
    }

    void RenderThread::Kick()
    {
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            Update(RenderThreadState::Kick);
            return;
        }

        Renderer::WaitAndRender(this);
    }

    void RenderThread::Kill()
    {
        m_IsRunning = false;
        if (m_ThreadPolicy == RenderThreadPolicy::MultiThreaded)
        {
            m_Thread->Join();
        }
    }

    void RenderThread::NextFrame()
    {
        Renderer::SwapBuffers();
    }

}