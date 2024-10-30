#include "pch.h"
#include "window.h"
#ifdef DODO_WINDOWS
#   include "platforms/windows/windows_window.h"
#endif

namespace Dodo {

    Ref<Window> Window::Create(WindowSpecifications&& specs)
    {
#ifdef DODO_WINDOWS
        return Ref<WindowsWindow>::Create(std::forward<WindowSpecifications>(specs));
#endif
    }

}