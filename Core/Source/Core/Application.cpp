#include "pch.h"
#include "Application.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // APPLICATION /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationSpecs& specs)
        : m_Specs(specs)
        , m_RenderThread(specs.ThreadPolicy)
    {
        DODO_ASSERT(!s_Instance, "Application instance already exists!");
        s_Instance = this;
    }

    void Application::Run()
    {
        Init();
        while (m_IsRunning)
        {
            Stopwatch waitStopwatch{};
            m_RenderThread.BlockUntilRenderComplete();
            m_PerformanceStats.MainThreadWaitTime = waitStopwatch.GetAsMilliseconds();

            Stopwatch workStopwatch{};
            m_Window->ProcessEvents();
            m_RenderThread.NextFrame();

            Renderer::Submit([this]() {
                Ref<Swapchain> swapchain = m_Window->GetSwapchain();
                swapchain->BeginFrame();
            });

            Renderer::Submit([this]() {
                Ref<Swapchain> swapchain = m_Window->GetSwapchain();
                swapchain->EndFrame();
            });

            m_PerformanceStats.MainThreadWorkTime = workStopwatch.GetAsMilliseconds();
        }

        Close();
    }

    void Application::Init()
    {
        m_Window = Window::Create(m_Specs.WindowSpecs);
        m_Window->Init();
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

        m_RenderThread.Dispatch();
        Renderer::Init(m_Specs.RendererSpecs);
        m_RenderThread.Pump();
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
        dispatcher.Dispatch<WindowCloseEvent >([this](WindowCloseEvent & e) { return OnWindowClose (e); });
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        Renderer::Submit([this, width = e.Width, height = e.Height]() {
            Ref<Swapchain> swapchain = m_Window->GetSwapchain();
            swapchain->ResizeViewport(width, height);
        });

        return false;
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_IsRunning = false;
        return false;
    }

    void Application::Close()
    {
        m_RenderThread.Stop();

        m_Window->SetEventCallback([](Event&) {});
        m_Window->Destroy();

        Renderer::Destroy();
    }

}