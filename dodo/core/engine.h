#pragma once

#include "window.h"
#include "graphics/renderer.h"

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
        void on_window_resized(WindowResizeEvent& e);
        void on_window_close();

        Ref<Window> window_ = nullptr;
        bool is_running_ = true;
        RenderThread render_thread_{};
        Ref<Renderer> renderer_ = nullptr;
        MainThreadPerformanceStats main_thread_performance_stats_{};
    };

}