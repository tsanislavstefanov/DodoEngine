#include "pch.h"
#include "Application.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"

////////////////////////////////////////////////////////////////
// APPLICATION /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

Application* Application::s_App = nullptr;

Application::Application(ApplicationSpecs specs)
    : m_Specs(std::move(specs))
{
    ASSERT(!s_App, "Application: Instance already exists!");
    s_App = this;
}

void Application::Run()
{
    Init();

    while (m_IsRunning)
    {
        m_Window->PollEvents();

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
            }

            m_FrameRateWatch.Reset();
            m_FrameRate = 0;
        }
    }

    Dispose();
}

void Application::Init()
{
    WindowSpecs windowSpecs{};
    windowSpecs.Width  = m_Specs.Width;
    windowSpecs.Height = m_Specs.Height;
    windowSpecs.Title  = m_Specs.Title;
    m_Window = Window::Create(windowSpecs);
    m_Window->Resize.Connect([](const WindowResizeEvent& e) { Renderer::Resize(e.Width, e.Height); });
    m_Window->Close .Connect([this]() { m_IsRunning = false; });

    Renderer::SetSettings(m_Specs.RenderSettings);
    Renderer::Init();

    OnInit();

    m_FrameRateWatch.Start();
}

void Application::Dispose()
{
    Renderer::Dispose();
    m_Window->Dispose();

    // Dispose client.
    OnDispose();
}