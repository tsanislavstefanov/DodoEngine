#include "pch.h"
#include "engine.h"

namespace Dodo {

    Engine::Engine(const CommandLineArgs& cmdLineArgs)
    {
        WindowSpecifications windowSpecs{};
        windowSpecs.Width = 1280;
        windowSpecs.Height = 720;
        windowSpecs.Title = "Dodo Engine";
        windowSpecs.Maximized = false;
        m_Window = Window::Create(std::move(windowSpecs));
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

        m_Renderer = Renderer::Create(m_RenderThread, RendererType::Vulkan, *m_Window);
        m_RenderThread.Start(RenderThreadPolicy::MultiThreaded);
    }

    Engine::~Engine()
    {
        m_RenderThread.Stop();
        m_Renderer = nullptr;
    }

    void Engine::run()
    {
        while (m_IsRunning)
        {
            Stopwatch waitStopwatch{};
            m_RenderThread.WaitUntilRenderComplete();
            main_thread_performance_stats_.wait_time = waitStopwatch.GetMilliseconds();

            // Process events even when the window is not focused.
            // This way, we can get notified when the window has regained focus.
            m_Window->ProcessEvents();

            m_RenderThread.Flush();

            m_Renderer->BeginFrame();
            // FRAME.
            m_Renderer->EndFrame();
        }
    }

    void Engine::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { OnWindowResize(e); });
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent&) { m_IsRunning = false; });
    }

    void Engine::OnWindowResize(WindowResizeEvent& e)
    {
        if ((e.Width == 0) || (e.Height == 0))
            return;

        m_Renderer->OnResize(e.Width, e.Height);
    }

}