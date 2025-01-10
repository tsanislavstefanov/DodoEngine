#pragma once

#include "display.h"
#include "renderer/renderer.h"
#include "renderer/render_context.h"

namespace Dodo {

    struct CommandLineArgs {
        const int count = 0;
        const char* const* values = nullptr;

        const char* operator[](size_t index) const {
            DODO_ASSERT((index >= 0) && (index < count));
            return values[index];
        }
    };

    class Engine {
    public:
        Engine(const CommandLineArgs& cmd_line_args);
        ~Engine();

        void iterate_main_loop();

    private:
        void _on_event(Display::Event& e);
        void _main_window_on_event(Display::Event& e);

        bool _is_running = true;
        Ref<Display> _display = nullptr;
        Display::WindowId _main_window_id = 0;
        Ref<RenderContext> _context = nullptr;
        Ref<Renderer> _renderer = nullptr;
        SurfaceHandle _main_surface_handle = nullptr;
        CommandQueueHandle _main_queue_handle = nullptr;
        SwapChainHandle _swap_chain_handle = nullptr;
    };

}
