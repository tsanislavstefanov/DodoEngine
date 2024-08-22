#pragma once

#include "Window.h"
#include "Bindings/Event.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderThread.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // COMMAND LINE ARGS ///////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct CommandLineArgs
    {
        int Count = 0;
        char** Args = nullptr;

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
        WindowSpecs WindowSpecs{};
        bool EnableImGui = false;
        ThreadPolicy ThreadPolicy = ThreadPolicy::None;
        RenderDeviceType RenderDeviceType = RenderDeviceType::None;
        RendererSpecs RendererSpecs{};
    };

    ////////////////////////////////////////////////////////////////
    // APPLICATION /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Application
    {
    public:
        static Application& GetCurrent()
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

        const Ref<Window>& GetWindow() const
        {
            return m_Window;
        }

        ////////////////////////////////////////////////////////////
        // PERFORMANCE STATS ///////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct PerformanceStats
        {
            double MainThreadWaitTime = 0.0;
            double MainThreadWorkTime = 0.0;
        };

        const PerformanceStats& GetPerformanceStats() const
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
        ApplicationSpecs m_Specs;
        RenderThread m_RenderThread;
        Ref<Window> m_Window = nullptr;
        bool m_IsRunning = true;
        PerformanceStats m_PerformanceStats{};
    };

}