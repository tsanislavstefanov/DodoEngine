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
#endif
        return nullptr;
    }

    void Window::Init()
    {
        // Create render context & swapchain.
        m_RenderContext = RenderContext::Create();
        m_Swapchain = Swapchain::Create();
    }

    void Window::ProcessEvents()
    {
        Input::Update();
        PollEvents();
    }

    void Window::Destroy()
    {
        // Swapchain has to be destroyed before the context
        // because the Swapchain needs the context.
        m_Swapchain->Destroy();
        m_RenderContext->Destroy();
    }

}