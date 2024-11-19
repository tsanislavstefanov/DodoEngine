#include "pch.h"
#include "display.h"
#include "platforms/windows/display_windows.h"

namespace Dodo {

    Ref<Display> Display::create(RenderContext::Type context_type) {
#ifdef DODO_WINDOWS
        return Ref<DisplayWindows>::create(context_type);
#endif
    }

}