#pragma once

#include "Window.h"
#include "Bindings/Event.h"
#include "Renderer/RenderSettings.h"
#include "Renderer/RenderThread.h"

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
            DODO_ASSERT(index < Count && index >= 0, "Index out of range!");
            return Args[index];
        }
    };

    ////////////////////////////////////////////////////////////////
    // APPLICATION SPECS ///////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct ApplicationSpecs
    {
        CommandLineArgs CmdLineArgs{};
        uint32_t        Width        = 0;
        uint32_t        Height       = 0;
        std::string     Title        = "Unnamed";
        bool            EnableImGui  = false;
        ThreadPolicy    ThreadPolicy = ThreadPolicy::None;
        RenderSettings  RenderSettings{};
    };

    ////////////////////////////////////////////////////////////////
    // PERFORMANCE STATS ///////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct PerformanceStats
    {
        double MainThreadWaitTime = 0.0;
        double MainThreadWorkTime = 0.0;
    };

    ////////////////////////////////////////////////////////////////
    // APPLICATION /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Application
    {
    public:
        static inline Application& GetCurrent()
        {
            return *s_Instance;
        }

        Application(const ApplicationSpecs& specs);
        virtual ~Application() = default;

        void Run();

        const ApplicationSpecs& GetSpecs() const
        {
            return m_Specs;
        }

        Ref<Window> GetWindow() const
        {
            return m_Window;
        }

        const PerformanceStats& GetStats() const
        {
            return m_PerformanceStats;
        }

    private:
        void Init();
        void OnEvent(Event& e);
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnWindowClose(WindowCloseEvent& e);
        void Close();

        static Application* s_Instance;
        ApplicationSpecs    m_Specs;
        RenderThread        m_RenderThread;
        Ref<Window>         m_Window      = nullptr;
        bool                m_IsRunning   = true;
        bool                m_ShouldPause = false;
        PerformanceStats    m_PerformanceStats{};
    };

}