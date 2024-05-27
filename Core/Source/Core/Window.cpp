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
#endif
        DODO_ASSERT(false, "Platform not supported!");
    }

    void Window::ProcessEvents()
    {
        Input::Update();
        PollEvents();
    }

}