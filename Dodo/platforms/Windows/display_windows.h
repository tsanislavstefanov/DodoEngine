#pragma once

#ifdef DODO_WINDOWS

#include <Windows.h>

#include "core/display.h"

namespace Dodo {

    class DisplayWindows : public Display {
    public:
        struct PlatformData {
            HMODULE hinstance = NULL;
            HWND hwnd = NULL;
        };

        DisplayWindows();

        WindowId window_create(const WindowSpecifications& window_specs) override;
        void window_show_and_focus(WindowId window_id) override;
        void window_focus(WindowId window_id) override;
        void window_set_event_callback(WindowId window_id, Func<void(Event&)>&& callback) override;
        void window_process_events(WindowId window_id) override;
        const void* window_get_platform_data(WindowId window_id) const override;

        uint32_t context_get_count() const override;
        Ref<RenderContext> context_get(size_t index) const override;

    private:
        static LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        struct WindowData {
            WindowId window_id = invalid_window_id;
            PlatformData platform_data{};
            uint32_t width = 0;
            uint32_t height = 0;
            std::string title{};
            Func<void(Event&)> event_callback{};
        };

        std::vector<Ref<RenderContext>> _contexts{};
        WindowId _window_id_counter = 0;
        std::map<WindowId, WindowData> _window_data_by_id{};
    };

}

#endif