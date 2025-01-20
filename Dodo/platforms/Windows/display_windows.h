#pragma once

#ifdef DODO_WINDOWS

#include <Windows.h>

#include "core/display.h"

namespace Dodo {

    class DisplayWindows : public Display {
    public:
        struct PlatformData {
            HMODULE hinstance{NULL};
            HWND hwnd{NULL};
        };

        DisplayWindows();

        WindowId window_create(const WindowSpecifications& window_specs) override;
        const void* window_get_platform_data(WindowId window) const override;
        void window_set_event_callback(WindowId window, const std::function<void(Event&)>& callback) override;
        void window_process_events(WindowId window) override;
        void window_destroy(WindowId window) override;
        uint32_t render_backend_get_count() const override;
        Ref<RenderBackend> render_backend_get(size_t index) const override;

    private:
        struct WindowData {
            WindowId window = invalid_window;
            PlatformData platform_data = {};
            uint32_t width = 0;
            uint32_t height{0};
            std::string title{};
            std::function<void(Event&)> event_callback{};
        };

        static LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        std::vector<Ref<RenderContext>> _contexts = {};
        WindowId _window_id_counter = 0;
        std::map<WindowId, WindowData> _window_data = {};
    };

}

#endif
