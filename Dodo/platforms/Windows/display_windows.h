#pragma once

#include "core/display.h"

namespace Dodo {

    class DisplayWindows : public Display {
    public:
        DisplayWindows() = default;

        WindowId window_create(const WindowSpecifications& window_specs) override;
        void window_set_event_callback(WindowId window, EventCallback&& callback) override;
        bool window_should_close(WindowId window) const override;
        void window_process_events(WindowId window) override;
        Ref<RenderContext> render_context_create(Renderer::Type type) override;

    private:
        static LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        struct WindowData {
            WindowId window = 0;
            HMODULE hinstance = NULL;
            HWND hwnd = NULL;
            uint32_t width  = 0;
            uint32_t height = 0;
            std::string title{};
            Display::EventCallback event_callback{};
            bool should_close = false;
        };

        std::map<WindowId, WindowData> _windows{};
    };

}