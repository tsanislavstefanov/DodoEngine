#pragma once

#include "RenderThread.h"
#include "Window.h"
#include "Renderer/RenderSettings.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // COMMAND LINE ARGS ///////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct CommandLineArgs
    {
        int    Count = 0;
        char** Args  = nullptr;

        const char* operator[](size_t index) const
        {
            DODO_ASSERT(index < Count && index >= 0, "CommandLineArgs index out of range!");
            return Args[index];
        }
    };

    ////////////////////////////////////////////////////////////////
    // APPLICATION SPECS ///////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct ApplicationSpecs
    {
        CommandLineArgs CmdLineArgs{};
        uint32_t Width  = 0;
        uint32_t Height = 0;
        std::string Title{};
        bool ShowFrameRate = false;
        bool EnableImGui = false;
        RenderSettings  RenderSettings{};
        ThreadingPolicy ThreadingPolicy = ThreadingPolicy::None;
    };

    ////////////////////////////////////////////////////////////////
    // PERFORMANCE STATS ///////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct PerformanceStats
    {
        double   MainThreadWaitTime   = 0.0;
        double   MainThreadWorkTime   = 0.0;
        double   RenderThreadWaitTime = 0.0;
        double   RenderThreadWorkTime = 0.0;
        uint64_t FrameRate            =   0;
    };

    ////////////////////////////////////////////////////////////////
    // APPLICATION /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Application
    {
    public:
        static Application& GetCurrent()
        {
            return *s_App;
        }

        Application(ApplicationSpecs specs);

        virtual ~Application() = default;

        const ApplicationSpecs& GetSpecs() const
        {
            return m_Specs;
        }

        Ref<Window> GetWindow()
        {
            return m_Window;
        }

        Ref<Window> GetWindow() const
        {
            return m_Window;
        }

        PerformanceStats& GetStats()
        {
            return m_PerformanceStats;
        }

        const PerformanceStats& GetStats() const
        {
            return m_PerformanceStats;
        }

        void Run();

    private:
        static Application* s_App;

        void Init ();
        void Close();

        void OnEvent(Event& e);
        bool OnWindowResized(WindowResizeEvent& e);
        bool OnWindowMinimized(WindowMinimizeEvent& e);
        bool OnWindowClosed(WindowCloseEvent& e);

        ApplicationSpecs m_Specs;
        RenderThread m_RenderThread;
        Ref<Window> m_Window = nullptr;
        bool m_IsRunning = true, m_IsMinimized = false;
        PerformanceStats m_PerformanceStats{};
    };

}