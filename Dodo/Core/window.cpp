#include "pch.h"
#include "window.h"
#ifdef DODO_WINDOWS
#   include "platforms/windows/windows_window.h"
#endif

namespace Dodo {

    Ref<RenderWindow> RenderWindow::Create(WindowSpecifications&& specs)
    {
#ifdef DODO_WINDOWS
        return Ref<WindowsWindow>::create(std::forward<WindowSpecifications>(specs));
#endif
    }

}