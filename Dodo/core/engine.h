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
        void _prepare_for_drawing();
        void _begin_frame();
        void _end_frame();
        void _execute_frame();

        bool _is_running = true;
        Ref<Display> _display = nullptr;
        Display::WindowId _main_window_id = 0;
        Ref<RenderContext> _context = nullptr;
        Ref<Renderer> _renderer = nullptr;
        SurfaceHandle _main_surface = SurfaceHandle::null();
        CommandQueueFamilyHandle _main_queue_family = CommandQueueFamilyHandle::null();
        CommandQueueHandle _main_queue = CommandQueueHandle::null();
        SwapChainHandle _swap_chain = SwapChainHandle::null();

        struct Frame {
            CommandPoolHandle command_pool = CommandPoolHandle::null();
            bool wait_for_fence = false;
            FenceHandle fence = FenceHandle::null();
            SemaphoreHandle image_semaphore = {};
            CommandListHandle draw_command_list = {};
            SemaphoreHandle present_semaphore = {};
        };

        uint32_t _desired_framebuffer_count = 3;
        std::vector<Frame> _frames = {};
        uint32_t _frame_index = 0;
    };

}
