#pragma once

#include "window.h"
#include "renderer/Renderer.h"

namespace Dodo {

    struct CommandLineArgs
    {
        const int count = 0;
        const char* const* values = nullptr;

        const char* operator[](size_t index) const
        {
            DODO_ASSERT((index >= 0) && (index < count), "Index out of range!");
            return values[index];
        }
    };

    struct MainThreadPerformanceStats
    {
        double wait_time = 0.0;
        double work_time = 0.0;
    };

    class Engine
    {
    public:
        Engine(const CommandLineArgs& cmd_line_args);
        ~Engine();

        void run();

    private:
        void OnEvent(Event& e);
        void OnWindowResize(WindowResizeEvent& e);

        Ref<Window> m_Window = nullptr;
        bool m_IsRunning = true;
        RenderThread m_RenderThread{};
        Ref<Renderer> m_Renderer = nullptr;
        MainThreadPerformanceStats main_thread_performance_stats_{};
    };

}