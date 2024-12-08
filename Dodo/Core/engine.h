#pragma once

#include "display.h"

namespace Dodo {

    struct CommandLineArgs {
        const int count = 0;
        const char* const* values = nullptr;

        const char* operator[](size_t index) const {
            DODO_ASSERT((index >= 0) && (index < count), "Index out of range!");
            return values[index];
        }
    };

    class Engine {
    public:
        Engine(const CommandLineArgs& cmd_line_args);
        ~Engine();

        void iterate_main_loop();

    private:
        void on_event(Display::WindowId window, Display::Event& e);
        void main_window_on_event(Display::Event& e);

        bool _is_running = true;
        Ref<Display> _display = nullptr;
        Display::WindowId _main_window = 0;
        bool _is_running = true;
        Ref<RenderContext> _render_context = nullptr;
    };

}