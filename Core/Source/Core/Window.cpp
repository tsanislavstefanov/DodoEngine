#include "pch.h"
#include "Window.h"
#ifdef PLATFORM_WINDOWS
#   include "Platform/Windows/WindowsWindow.h"
#endif

////////////////////////////////////////////////////////////////
// WINDOW //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

Ref<Window> Window::Create(const WindowSpecs& specs)
{
#ifdef PLATFORM_WINDOWS
    return Ref<WindowsWindow>::Create(specs);
#endif

    ASSERT(false, "Platform not supported!");
}