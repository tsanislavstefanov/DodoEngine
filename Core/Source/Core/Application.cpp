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

        Stopwatch frameStopwatch{};
        frameStopwatch.Start();

        std::vector<Func<void()>> funcs{};

        while (m_IsRunning)
        {
            // Wait for RenderThread to finish [N - 2].
            {
                Stopwatch waitStopwatch{};
                m_RenderThread.BlockUntilRenderComplete();
                m_PerformanceStats.MainThreadWaitTime = waitStopwatch.GetAsMilliseconds();
            }

            funcs.push_back([this]() { LOG_CORE_WARNING("Wait time: {}", m_PerformanceStats.MainThreadWaitTime); });

            m_Window->ProcessEvents();

            // Render previous frame [N - 1] and prepare new [N].
            m_RenderThread.NextFrame();

            // Don't update the application when not focused.
            if (!m_Window->HasFocus())
            {
                continue;
            }

            Renderer::Schedule([this]()
            {
                m_Window->GetSwapchain()->BeginFrame();
            });

            Renderer::Schedule([this]()
            {
                int a  = 5 + 5;
            });

            Renderer::Schedule([this]()
            {
                m_Window->GetSwapchain()->Present();
            });

            m_PerformanceStats.FrameRate++;
            if (frameStopwatch.GetAsSeconds() >= 1.0)
            {
                if (m_Specs.ShowFrameRate)
                {
                    m_Window->SetTitle(std::format("{0} [Frame Rate: {1}]", m_Specs.Title, m_PerformanceStats.FrameRate));
                }

                m_PerformanceStats.FrameRate = 0;
                frameStopwatch.Reset();
            }

            for (auto& f : funcs)
                f.Invoke();
        }

        Close();
    }

    void Application::Init()
    {
        Renderer::SetSettings(m_Specs.RenderSettings);

        m_RenderThread.Run();

        // Create window & set event callback.
        WindowSpecs windowSpecs{};
        windowSpecs.Width  = m_Specs.Width;
        windowSpecs.Height = m_Specs.Height;
        windowSpecs.Title  = m_Specs.Title;
        m_Window = Window::Create(windowSpecs);
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

        Renderer::Init();

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

    bool Application::OnWindowMinimized(WindowMinimizeEvent& e)
    {
        m_IsMinimized = e.Iconify;
        return false;
    }

    bool Application::OnWindowClosed(WindowCloseEvent& e)
    {
        m_IsRunning = false;
        return false;
    }

}