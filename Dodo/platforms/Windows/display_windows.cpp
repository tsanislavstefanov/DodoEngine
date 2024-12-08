#include "pch.h"
#include "display_windows.h"
#include "render_context_vulkan_windows.h"

namespace Dodo {

    DisplayWindows::DisplayWindows(RenderContext::Type context_type) {
        switch (context_type) {
            case RenderContext::Type::vulkan:
                _render_context = Ref<RenderContextVulkanWindows>::create();
                break;

            default: DODO_VERIFY(false); break;
        }
    }

    Display::WindowId DisplayWindows::window_create(const WindowSpecifications& window_specs) {
        const HMODULE hinstance = GetModuleHandleW(NULL);

        WNDCLASSEXW wndclass{};
        ZeroMemory(&wndclass, sizeof(WNDCLASSEXW));
        wndclass.cbSize = sizeof(WNDCLASSEXW);
        wndclass.style = CS_OWNDC;
        wndclass.lpfnWndProc = DisplayWindows::_wnd_proc;
        wndclass.hInstance = hinstance;
        wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wndclass.lpszClassName = L"Dodo";
        RegisterClassExW(&wndclass);

        RECT rect{};
        ZeroMemory(&rect, sizeof(RECT));
        rect.right  = static_cast<LONG>(window_specs.width);
        rect.bottom = static_cast<LONG>(window_specs.height);
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        const std::wstring title(window_specs.title.begin(), window_specs.title.end());
        HWND hwnd = NULL;
        DODO_ASSERT(
            hwnd = CreateWindowExW(
                0,
                L"Dodo",
                title.c_str(),
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                rect.right - rect.left, rect.bottom - rect.top,
                NULL,
                NULL,
                hinstance,
                NULL),
            "DisplayWindows failed to create HWND!"
        );

        if (!hwnd) {
            return invalid_window_id;
        }

        const WindowId window_id = _window_id_counter++;
        WindowData& data = _window_ids[window_id];
        data.window_id = window_id;
        data.platform_data = PlatformData{ hinstance, hwnd };
        data.width = window_specs.width;
        data.height = window_specs.height;
        data.title = window_specs.title;

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&data));
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        SetFocus(hwnd);

        return window_id;
    }

    void DisplayWindows::window_set_event_callback(WindowId window_id, EventCallback&& callback) {
        if (_window_ids.contains(window_id)) {
            _window_ids.at(window_id).event_callback = std::move(callback);
        }
    }

    void DisplayWindows::window_process_events(WindowId window_id) {
        if (_window_ids.contains(window_id)) {
            MSG msg{};
            ZeroMemory(&msg, sizeof(MSG));
            while (PeekMessageW(&msg, _window_ids.at(window_id).platform_data.hwnd, 0, 0, PM_REMOVE)) {
                // Translate virtual key message into character message.
                TranslateMessage(&msg);
                // Dispatch message to window procedure.
                DispatchMessageW(&msg);
            }
        }
    }

    const void* DisplayWindows::window_get_platform_data(WindowId window_id) const {
        if (_window_ids.contains(window_id)) {
            return &_window_ids.at(window_id).platform_data;
        }

        return nullptr;
    }

    LRESULT DisplayWindows::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        const LONG_PTR user_data = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (!user_data) {
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        }

        auto& window_data = *reinterpret_cast<WindowData*>(user_data);
        switch (msg) {
            case WM_SIZE:
                const auto width  = static_cast<uint32_t>(LOWORD(lparam));
                const auto height = static_cast<uint32_t>(HIWORD(lparam));
                if (window_data.width == width && window_data.height == height) {
                    break;
                }

                window_data.width  = width;
                window_data.height = height;

                Event e{};
                e.type = Event::Type::window_resize;
                e.resized.width  = width;
                e.resized.height = height;
                window_data.event_callback(window_data.window_id, e);
                break;

            case WM_CLOSE:
                Event e{};
                e.type = Event::Type::window_close;
                window_data.event_callback(window_data.window_id, e);
                PostQuitMessage(0);
                break;

            default: break;
        }

        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

}