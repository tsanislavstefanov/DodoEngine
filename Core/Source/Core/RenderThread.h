#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // THREAD POLICY ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    enum class ThreadPolicy
    {
        SingleThreaded,
        MultiThreaded ,
        AutoCount     ,
        None
    };

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderThread
    {
    public:
        RenderThread(ThreadPolicy policy);

        void Run();

    private:
        void Dispatch();

        ThreadPolicy m_ThreadPolicy = ThreadPolicy::None;
        bool         m_IsRunning    = false;
        std::thread  m_Thread{};
        std::string  m_ThreadName   = "RenderThread";
    };
}