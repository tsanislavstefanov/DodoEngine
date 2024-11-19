#include "pch.h"
#include "render_context.h"

namespace Dodo {

    void RenderContext::window_set_vsync_mode(Display::WindowId window, VSyncMode vsync_mode) {
        if (_surfaces.contains(window)) {
            surface_set_vsync_mode(_surfaces.at(window);
        }
    }

    void RenderContext::window_resize(Display::WindowId window, uint32_t width, uint32_t height) {
        if (_surfaces.contains(window)) {
            surface_resize(_surfaces.at(window), width, height);
        }
    }

}