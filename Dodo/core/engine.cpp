#include "pch.h"
#include "engine.h"

namespace Dodo {

    Engine::Engine(const CommandLineArgs& cmd_line_args) {
        _display = Display::create ();

        const RenderContext::Type desired_context_type = RenderContext::Type::vulkan;
        for (size_t i = 0; i < _display->context_get_count(); i++) {
            if (_display->context_get(i)->get_type() == desired_context_type) {
                _context = _display->context_get(i);
                break;
            }
        }

        _context->initialize();
        _renderer = _context->renderer_create();
        size_t device_index = 0;
        for (size_t i = 0; i < _context->adapter_get_count(); i++) {
            if (_context->adapter_get(i).type == RenderContext::Adapter::Type::discrete) {
                device_index = i;
                break;
            }
        }

        _renderer->initialize(device_index);

        Display::WindowSpecifications main_window_specs = {};
        main_window_specs.width = 1280;
        main_window_specs.height = 720;
        main_window_specs.title = "Dodo Engine";
        _main_window_id = _display->window_create(main_window_specs);
        _display->window_set_event_callback(_main_window_id, [this](Display::Event& e) { _on_event(e); });

        RenderContext::SurfaceSpecifications main_surface_specs = {};
        main_surface_specs.width = main_window_specs.width;
        main_surface_specs.height = main_window_specs.height;
        main_surface_specs.vsync_mode = RenderContext::VSyncMode::enabled;
        _main_surface_handle = _context->surface_create(_main_window_id, main_surface_specs, _display->window_get_platform_data(_main_window_id));

        _renderer->swapchain_create(_main_surface_handle);
    }

    Engine::~Engine() {
    }

    void Engine::iterate_main_loop() {
        while (_is_running) {
            _display->window_process_events(_main_window_id);
        }
    }

    void Engine::_on_event(Display::Event& e) {
        if (e.window_id == _main_window_id) {
            _main_window_on_event(e);
        }

        _context->on_event(e);
    }

    void Engine::_main_window_on_event(Display::Event& e) {
        switch (e.type) {
            case Display::Event::Type::window_close: {
                _is_running = false;
                break;
            }

            default:
                break;
        }
    }

}
