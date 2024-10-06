#include "pch.h"
#include "window.h"
#ifdef DODO_WINDOWS
#   include "platforms/windows/windows_window.h"
#endif

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOW //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Ref<Window> Window::create(WindowSpecifications&& specs)
    {
#ifdef DODO_WINDOWS
        return Ref<WindowsWindow>::create(std::forward<WindowSpecifications>(specs));
#endif
    }

}