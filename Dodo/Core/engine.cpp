#include "pch.h"
#include "engine.h"
#include "Renderer/render_thread.h"

namespace Dodo {

    Engine::Engine(const CommandLineArgs& cmdLineArgs)
    {
        WindowSpecifications windowSpecs{};
        windowSpecs.Width = 1280;
        windowSpecs.Height = 720;
        windowSpecs.Title = "Dodo Engine";
        windowSpecs.Maximized = false;
        m_Window = RenderWindow::Create(std::move(windowSpecs));
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

        render_thread.start();
        m_Renderer = Renderer::create(*render_thread, RendererType::vulkan, *m_Window);
        // To compile all default shaders.
        render_thread.pump();
    }

    Engine::~Engine()
    {
        render_thread.stop();
        m_Renderer = nullptr;
    }

    void Engine::run()
    {
        while (m_IsRunning)
        {
            m_Window->ProcessEvents();

            render_thread.wait_on_render_complete();
            // Render thread is few frames behind.
            render_thread.render();
            m_Renderer->BeginFrame();
            // FRAME.
            m_Renderer->EndFrame();
        }
    }

    void Engine::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
        dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent&) { m_IsRunning = false; return false; });
    }

    bool Engine::OnWindowResize(WindowResizeEvent& e)
    {
        if ((e.Width == 0) || (e.Height == 0))
            return false;

        m_Renderer->OnResize(e.Width, e.Height);
        return false;
    }

}