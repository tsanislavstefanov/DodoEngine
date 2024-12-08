#include "pch.h"
#include "display.h"
#ifdef DODO_WINDOWS
#   include "platforms/windows/display_windows.h"
#endif

namespace Dodo {

    Ref<Display> Display::create(RenderContext::Type context_type) {
#ifdef DODO_WINDOWS
        return Ref<DisplayWindows>::create(context_type);
#endif
    }

    Ref<RenderContext> Display::get_render_context() const {
        DODO_ASSERT(_render_context, "RenderContext is not initialized!");
        return _render_context;
    }

}