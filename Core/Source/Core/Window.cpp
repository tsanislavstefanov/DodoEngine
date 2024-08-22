#include "pch.h"
#include "Window.h"
#include "Input/Input.h"
#ifdef DODO_WINDOWS
#   include "Platform/Windows/WindowsWindow.h"
#endif

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOW //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<Window> Window::Create(const WindowSpecs& specs)
    {
#ifdef DODO_WINDOWS
        return Ref<WindowsWindow>::Create(specs);
#else
        DODO_ASSERT(false, "Platform not supported!");
        return nullptr;
#endif
    }

    void Window::Init()
    {
        m_RenderContext = RenderContext::Create();
        m_Swapchain = Swapchain::Create(m_Handle, m_Data.Width, m_Data.Height);
    }

    void Window::ProcessEvents()
    {
        Input::Update();
        PollEvents();
    }

    void Window::Destroy()
    {
        m_Swapchain->Destroy();
        m_RenderContext->Destroy();
    }

}