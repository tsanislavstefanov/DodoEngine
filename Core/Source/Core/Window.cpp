#include "pch.h"
#include "Window.h"
#ifdef PLATFORM_WINDOWS
#   include "Platform/Windows/WindowsWindow.h"
#endif

////////////////////////////////////////////////////////////////
// WINDOW //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

std::unique_ptr<Window> Window::Create(const WindowSpecs& specs)
{
#ifdef PLATFORM_WINDOWS
    return std::make_unique<WindowsWindow>(specs);
#endif

    ASSERT(false, "Platform not supported!");
}