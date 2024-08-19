#include "pch.h"
#include "Application.h"
#include "Renderer/Renderer.h"
#include "Renderer/Swapchain.h"

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
            // Wait for RenderThread to finish frame [N - 2].
            {
                Stopwatch stopwatch{};
                m_RenderThread.BlockUntilRenderComplete();
                m_PerformanceStats.MainThreadWaitTime = stopwatch.GetAsMilliseconds();
            }

            m_Window->ProcessEvents();
            
            // Kick RenderThread to render previous frame [N - 1]
            // while preparing new frame [N].
            m_RenderThread.NextFrame();

            RenderThread::Submit([this]() {
                Ref<Swapchain> swapchain = m_Window->GetSwapchain();
                swapchain->BeginFrame_RenderThread();
            });

            RenderThread::Submit([this]() {
                Ref<Swapchain> swapchain = m_Window->GetSwapchain();
                swapchain->Present_RenderThread();
            });
        }

        Close();
    }

    void Application::Init()
    {
        m_RenderThread.Dispatch();

        // Create window & set event callback.
        WindowSpecs windowSpecs{};
        windowSpecs.Width  = m_Specs.Width;
        windowSpecs.Height = m_Specs.Height;
        windowSpecs.Title  = m_Specs.Title;
        m_Window = Window::Create(windowSpecs);
        m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });
        m_Window->Init();

        Renderer::Init();

        // Render one frame.
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
        RenderThread::Submit([this, width = e.Width, height = e.Height]() {
            Ref<Swapchain> swapchain = m_Window->GetSwapchain();
            swapchain->OnResize_RenderThread(width, height);
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

        // Reset & destroy window.
        m_Window->SetEventCallback([](Event&) {});
        m_Window->Destroy();

        // Renderer needs to be destroyed after the window!
        Renderer::Destroy();
    }

}