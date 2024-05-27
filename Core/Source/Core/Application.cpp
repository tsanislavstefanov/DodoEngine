#include "pch.h"
#include "Application.h"
#include "Renderer/Renderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // APPLICATION /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Application* Application::s_App = nullptr;

    Application::Application(ApplicationSpecs specs)
        : m_Specs(std::move(specs))
        , m_RenderThread(specs.ThreadingPolicy)
    {
        DODO_ASSERT(!s_App, "Application instance already exists!");
        s_App = this;
    }

    void Application::Run()
    {
        Init();
        while (m_IsRunning)
        {
            // Wait for RenderThread to finish frame.
            {
                Stopwatch waitStopwatch{};
                m_RenderThread.BlockUntilRenderComplete();
                m_PerformanceStats.MainThreadWaitTime = waitStopwatch.GetAsMilliseconds();
            }

            Stopwatch stopwatch{};
            stopwatch.Start();

            m_Window->ProcessEvents();

            // Render previous frame and prepare new.
            m_RenderThread.NextFrame();

            // Don't update the application when not focused.
            if (!m_Window->HasFocus())
            {
                continue;
            }

            // Render frame.
            Renderer::Schedule([this]()
            {
                m_Window->GetSwapchain()->BeginFrame();
            });

            Renderer::Schedule([this]()
            {
                m_Window->GetSwapchain()->Present();
            });

            // Accumulate frame rate.
            m_PerformanceStats.FrameRate++;
            if (stopwatch.GetAsSeconds() >= 1.0)
            {
                if (m_Specs.ShowFrameRate)
                {
                    m_Window->SetTitle(std::format("{0} [Frame Rate: {1}]", m_Specs.Title, m_PerformanceStats.FrameRate));
                }

                m_PerformanceStats.FrameRate = 0;
            }
        }

        Close();
    }

    void Application::Init()
    {
        Renderer::SetSettings(m_Specs.RenderSettings);

        // Create window & set event callback.
        WindowSpecs windowSpecs{};
        windowSpecs.Width  = m_Specs.Width;
        windowSpecs.Height = m_Specs.Height;
        windowSpecs.Title  = m_Specs.Title;
        m_Window = Window::Create(windowSpecs);
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

        Renderer::Init();

        m_RenderThread.Run();

        // Render one frame.
        m_RenderThread.Pump();
    }

    void Application::Close()
    {
        // Wait for the RenderThread, then destroy the Window.
        m_RenderThread.Stop();

        m_Window->SetEventCallback([](Event&) {});
        m_Window->Destroy();

        Renderer::Shutdown();
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResized(e); });
        dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return OnWindowMinimized(e); });
        dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClosed(e); });
    }

    bool Application::OnWindowResized(WindowResizeEvent& e)
    {
        Renderer::Schedule([this, width = e.Width, height = e.Height]()
        {
            m_Window->GetSwapchain()->OnResize(width, height);
        });
        return false;
    }

    bool Application::OnWindowMinimized(WindowMinimizeEvent &e)
    {
        m_IsMinimized = e.Iconify;
        return false;
    }

    bool Application::OnWindowClosed(WindowCloseEvent &e)
    {
        m_IsRunning = false;
        return false;
    }

}