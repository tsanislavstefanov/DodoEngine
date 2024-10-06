#include "pch.h"
#include "engine.h"
#include "graphics/swapchain.h"
#include "graphics/drivers/vulkan/vulkan_buffer.h"

namespace Dodo {

    Engine::Engine(const CommandLineArgs& cmd_line_args)
    {
        WindowSpecifications window_specs{};
        window_specs.width = 1280;
        window_specs.height = 720;
        window_specs.title = "Dodo Engine";
        window_specs.maximized = false;
        window_ = Window::create(std::move(window_specs));
        window_->on_resized.connect([this](WindowResizeEvent& e) { on_window_resized(e); });
        window_->on_close.connect([this]() { on_window_close(); });

        renderer_ = Renderer::create(render_thread_, RendererType::vulkan, *window_);
        render_thread_.start(renderer_->get_thread_policy());
    }

    Engine::~Engine()
    {
        render_thread_.stop();
        renderer_ = nullptr;
    }

    void Engine::run()
    {
        while (is_running_)
        {
            Stopwatch wait_stopwatch{};
            render_thread_.wait_on_render_complete();
            main_thread_performance_stats_.wait_time = wait_stopwatch.milliseconds();

            // Process events even when the window is not
            // focused. This way, we can get notified when
            // the window has regained focus.
            window_->process_events();
            if (!window_->has_focus())
            {
                continue;
            }

            render_thread_.flush_all();

            renderer_->begin_frame();
            // FRAME.
            renderer_->end_frame();
        }
    }

    void Engine::on_window_resized(WindowResizeEvent& e)
    {
        if ((e.width == 0) || (e.height == 0))
            return;

        renderer_->resize_swapchain(e.width, e.height);
    }

    void Engine::on_window_close()
    {
        is_running_ = false;

        BufferSpecifications buffer_specs{};
        buffer_specs.size = sizeof(float);
        buffer_specs.usage = BufferUsage::vertex_buffer;
        BufferID id = renderer_->create_buffer(std::move(buffer_specs));

        renderer_->submit([id]() {
            auto info = reinterpret_cast<VulkanBufferInfo*>(id.id);
            auto alloc = info->allocation;
            auto buff = info->buffer;
        });

        renderer_->free_buffer(id);
    }

}