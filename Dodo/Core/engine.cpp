#include "pch.h"
#include "engine.h"

namespace Dodo {

    Engine::Engine(const CommandLineArgs& cmd_line_args) {
        _display = Display::create(RenderContext::Type::vulkan);
        _render_context = _display->get_render_context();

        // Create (main) window.
        Display::WindowSpecifications window_specs{};
        window_specs.width = 1280;
        window_specs.height = 720;
        window_specs.title = "Dodo Engine";
        _main_window = _display->window_create(window_specs);
        _display->window_set_event_callback(_main_window, [this](Display::WindowId window, Display::Event& e) { on_event(window, e); });



    }

    Engine::~Engine() {
    }

    void Engine::iterate_main_loop() {
        while (_is_running) {
            _display->window_process_events(_main_window);
        }
    }

    void Engine::on_event(Display::WindowId window, Display::Event& e) {
        if (window == _main_window) {
            main_window_on_event(e);
        }
    }

    void Engine::main_window_on_event(Display::Event& e) {
        switch (e.type) {
            case Display::Event::Type::window_close: {
                _is_running = false;
                break;
            }

            default: break;
        }
    }

}