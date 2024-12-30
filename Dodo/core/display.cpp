#include "pch.h"
#include "display.h"
#ifdef DODO_WINDOWS
#   include "platforms/windows/display_windows.h"
#endif
#include "renderer/render_context.h"

namespace Dodo {

    Ref<Display> Display::create() {
#ifdef DODO_WINDOWS
        return Ref<DisplayWindows>::create();
#else
        return nullptr;
#endif
    }

}