#include "pch.h"
#include "Application.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // APPLICATION /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Application* Application::s_App = nullptr;

    Application::Application(ApplicationSpecs specs)
        :
        m_Specs(std::move(specs)),
        m_RenderThread(RenderThread::Create(m_Specs.RenderThreadPolicy))
    {
        ASSERT(!s_App, "Application: Instance already exists!");
        s_App = this;
    }

    void Application::Run()
    {
        Init();

        while (m_IsRunning)
        {
            // Wait render thread to render frame.
            {
                Stopwatch stopwatch{};
                m_RenderThread->BlockUntilRenderComplete();
                m_PerformanceStats.MainThreadWaitTime = stopwatch.GetAsMilliseconds();
            }

            m_Window->PollEvents();

            m_RenderThread->NextFrame();
            // Render previous frame.
            m_RenderThread->Kick();

            // Don't update the application when not focused.
            if (!m_Window->HasFocus())
            {
                continue;
            }

            // Update client.
            OnUpdate();

            // Render frame.
            Renderer::BeginFrame();
            OnBeginRender();
            OnRender();
            OnEndRender();
            Renderer::EndFrame();

            // Prepare input devices for next frame.
            Input::Update();

            // Accumulate frame rate.
            m_FrameRate++;
            if (m_FrameRateWatch.GetAsSeconds() >= 1.0)
            {
                if (m_Specs.ShowFrameRate)
                {
                    m_Window->SetTitle(std::format("{0} [Frame Rate: {1}]", m_Specs.Title, m_FrameRate));
                    LOG_CORE_WARNING("MainThread wait time: {}.", m_PerformanceStats.MainThreadWaitTime);
                    LOG_CORE_WARNING("RenderThread wait time: {}.", m_PerformanceStats.RenderThreadWaitTime);
                    LOG_CORE_WARNING("RenderThread work time: {}.", m_PerformanceStats.RenderThreadWorkTime);
                }

                m_FrameRateWatch.Reset();
                m_FrameRate = 0;
            }
        }

        Dispose();
    }

    void Application::Init()
    {
        m_RenderThread->Run();

        WindowSpecs windowSpecs{};
        windowSpecs.Width  = m_Specs.Width;
        windowSpecs.Height = m_Specs.Height;
        windowSpecs.Title  = m_Specs.Title;
        m_Window = Window::Create(windowSpecs);
        m_Window->Resize.Connect([](const WindowResizeEvent& e) { Renderer::Resize(e.Width, e.Height); });
        m_Window->Close .Connect([this]() { m_IsRunning = false; });

        Renderer::SetSettings(m_Specs.RenderSettings);
        Renderer::Init();

        // Render one frame.
        m_RenderThread->Pump();

        OnInit();

        m_FrameRateWatch.Start();
    }

    void Application::Dispose()
    {
        m_RenderThread->Kill();

        Renderer::Dispose();
        m_Window->Dispose();

        // Dispose client.
        OnDispose();
    }

}