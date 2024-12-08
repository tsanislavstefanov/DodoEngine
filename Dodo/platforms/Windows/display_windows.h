#pragma once

#include <Windows.h>

#include "core/display.h"

namespace Dodo {

    class DisplayWindows : public Display {
    public:
        struct PlatformData {
            HMODULE hinstance = NULL;
            HWND hwnd = NULL;
        };

        DisplayWindows(RenderContext::Type context_type);

        WindowId window_create(const WindowSpecifications& window_specs) override;
        void window_set_event_callback(WindowId window_id, EventCallback&& callback) override;
        void window_process_events(WindowId window_id) override;
        const void* window_get_platform_data(WindowId window_id) const override;

    private:
        static LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        struct WindowData {
            WindowId window_id = 0;
            PlatformData platform_data{};
            uint32_t width  = 0;
            uint32_t height = 0;
            std::string title{};
            EventCallback event_callback{};
        };

        std::map<WindowId, WindowData> _window_ids{};
    };

}