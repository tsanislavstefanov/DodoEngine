#include "pch.h"
#include "display.h"
#ifdef DODO_WINDOWS
#   include "platforms/windows/display_windows.h"
#endif

namespace Dodo {

    Display& Display::singleton_get() {
        #if defined(DODO_WINDOWS)
            static DisplayWindows display{};
            return display;
        #endif
    }

}