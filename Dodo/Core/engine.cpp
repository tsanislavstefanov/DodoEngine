#include "pch.h"
#include "engine.h"

namespace Dodo {

    Engine::Engine(const CommandLineArgs& cmd_line_args) {
        _display = Display::create();

        Display::WindowSpecifications window_specs{};
        window_specs.width = 1280;
        window_specs.height = 720;
        window_specs.title = "Dodo Engine";
        _main_window = _display->window_create(window_specs);
        _display->window_set_event_callback(_main_window, [this](Display::WindowId window, Display::Event& e) { on_event(window, e); });
        _render_context = _display->render_context_create(Renderer::Type::vulkan);
    }

    Engine::~Engine() {
    }

    void Engine::iterate_main_loop() {
        while (_display->window_should_close(_main_window)) {
            _display->window_process_events(_main_window);
        }
    }

    void Engine::on_event(Display::WindowId window, Display::Event& e) {
        switch (e.type) {

        }
    }

}