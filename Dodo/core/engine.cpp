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
        _main_surface = _context->surface_create(_main_window_id, main_surface_specs, _display->window_get_platform_data(_main_window_id));
    }

    Engine::~Engine() {
    }

    void Engine::iterate_main_loop() {
        _prepare_for_drawing();
        while (_is_running) {
            _display->window_process_events(_main_window_id);

            _begin_frame();
            // RENDER
            _end_frame();
            _execute_frame();
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

    void Engine::_prepare_for_drawing() {
        _main_queue_family = _renderer->command_queue_family_get(Renderer::CommandQueueFamilyType::draw, _main_surface);
        _main_queue = _renderer->command_queue_create(_main_queue_family);
        _swap_chain = _renderer->swap_chain_create(_main_surface);

        _frames.clear();
        _frames.resize(_desired_framebuffer_count);
        for (size_t i = 0; i < _desired_framebuffer_count; i++) {
            Frame& frame = _frames.at(i);
            frame.command_pool = _renderer->command_pool_create(_main_queue_family, Renderer::CommandListType::primary);
            frame.fence = _renderer->fence_create();
            frame.image_semaphore = _renderer->semaphore_create();
            frame.draw_command_list = _renderer->command_list_create(frame.command_pool);
            frame.present_semaphore = _renderer->semaphore_create();
        }
    }

    void Engine::_begin_frame() {
        Frame& frame = _frames.at(_frame_index);
        if (frame.wait_for_fence) {
            _renderer->fence_wait(frame.fence);
            frame.wait_for_fence = false;
        }

        auto swap_chain_status = Renderer::SwapChainStatus::ok;
        FramebufferHandle framebuffer = _renderer->swap_chain_acquire_next_framebuffer(frame.image_semaphore, _swap_chain, frame.fence, swap_chain_status);
        if (swap_chain_status == Renderer::SwapChainStatus::out_of_date) {
            _renderer->swap_chain_resize(_main_queue, _swap_chain, _desired_framebuffer_count);
            framebuffer = _renderer->swap_chain_acquire_next_framebuffer(frame.image_semaphore, _swap_chain, frame.fence, swap_chain_status);
            DODO_ASSERT(swap_chain_status == Renderer::SwapChainStatus::ok);
        }

        _renderer->command_list_begin(frame.draw_command_list);
    }

    void Engine::_end_frame() {
        Frame& frame = _frames.at(_frame_index);
        _renderer->command_list_end(frame.draw_command_list);
    }

    void Engine::_execute_frame() {
        Frame& frame = _frames.at(_frame_index);

        _renderer->command_queue_execute_and_present();
        frame.wait_for_fence = true;
    }

}
