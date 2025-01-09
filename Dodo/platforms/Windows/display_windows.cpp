#include "pch.h"

#ifdef DODO_WINDOWS

#include "display_windows.h"
#ifdef DODO_VULKAN
#   include "render_context_vulkan_windows.h"
#endif

namespace Dodo {

    DisplayWindows::DisplayWindows() {
#ifdef DODO_VULKAN
        auto context_vk = Ref<RenderContextVulkanWindows>::create();
        _contexts.push_back(context_vk);
#endif
    }

    Display::WindowId DisplayWindows::window_create(const WindowSpecifications& window_specs) {
        const HMODULE hinstance = GetModuleHandleW(NULL);

        WNDCLASSEXW wndclass = {};
        ZeroMemory(&wndclass, sizeof(WNDCLASSEXW));
        wndclass.cbSize = sizeof(WNDCLASSEXW);
        wndclass.style = CS_OWNDC;
        wndclass.lpfnWndProc = DisplayWindows::_wnd_proc;
        wndclass.hInstance = hinstance;
        wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndclass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wndclass.lpszClassName = L"Dodo";
        RegisterClassExW(&wndclass);

        RECT rect = {};
        ZeroMemory(&rect, sizeof(RECT));
        rect.right = static_cast<LONG>(window_specs.width );
        rect.bottom = static_cast<LONG>(window_specs.height);
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        const std::wstring title(window_specs.title.begin(), window_specs.title.end());
        HWND hwnd = NULL;
        DODO_ASSERT(hwnd = CreateWindowExW(
            0,
            L"Dodo",
            title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            NULL,
            NULL,
            hinstance,
            NULL
        ));

        if (!hwnd) {
            return invalid_window_id;
        }

        const WindowId window_id = _window_id_counter++;
        WindowData& data = _window_data_by_id[window_id];
        data.window_id = window_id;
        data.platform_data.hinstance = hinstance;
        data.platform_data.hwnd = hwnd;
        data.width = window_specs.width;
        data.height = window_specs.height;
        data.title = window_specs.title;

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&data));
        window_show_and_focus(window_id);

        return window_id;
    }

    void DisplayWindows::window_show_and_focus(WindowId window_id) {
        if (_window_data_by_id.contains(window_id)) {
            const WindowData& data = _window_data_by_id.at(window_id);
            ShowWindow(_window_data_by_id.at(window_id).platform_data.hwnd, SW_SHOWDEFAULT);
            window_focus(window_id);
        }
    }

    void DisplayWindows::window_focus(WindowId window_id) {
        if (_window_data_by_id.contains(window_id)) {
            SetFocus(_window_data_by_id.at(window_id).platform_data.hwnd);
        }
    }

    void DisplayWindows::window_set_event_callback(WindowId window_id, Func<void(Event&)>&& callback) {
        if (_window_data_by_id.contains(window_id)) {
            _window_data_by_id.at(window_id).event_callback = std::move(callback);
        }
    }

    void DisplayWindows::window_process_events(WindowId window_id) {
        if (_window_data_by_id.contains(window_id)) {
            MSG msg = {};
            ZeroMemory(&msg, sizeof(MSG));
            while (PeekMessageW(&msg, _window_data_by_id.at(window_id).platform_data.hwnd, 0, 0, PM_REMOVE)) {
                // Translate virtual key message into character message.
                TranslateMessage(&msg);
                // Dispatch message to window procedure.
                DispatchMessageW(&msg);
            }
        }
    }

    const void* DisplayWindows::window_get_platform_data(WindowId window_id) const {
        if (!_window_data_by_id.contains(window_id)) {
            return nullptr;
        }

        return &_window_data_by_id.at(window_id).platform_data;
    }

    uint32_t DisplayWindows::context_get_count() const {
        return static_cast<uint32_t>(_contexts.size());
    }

    Dodo::Ref<Dodo::RenderContext> DisplayWindows::context_get(size_t index) const {
        return _contexts.at(index);
    }

    LRESULT DisplayWindows::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        const LONG_PTR user_data = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (!user_data) {
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        }

        auto& window_data = *reinterpret_cast<WindowData*>(user_data);
        switch (msg) {
            case WM_SIZE: {
                const auto width = static_cast<uint32_t>(LOWORD(lparam));
                const auto height = static_cast<uint32_t>(HIWORD(lparam));
                if ((window_data.width == width) && (window_data.height == height)) {
                    break;
                }

                window_data.width = width;
                window_data.height = height;

                Event e = {};
                e.window_id = window_data.window_id;
                e.type = Event::Type::window_resize;
                e.resized.width = width;
                e.resized.height = height;
                window_data.event_callback(e);
                break;
            }

            case WM_CLOSE: {
                Event e = {};
                e.window_id = window_data.window_id;
                e.type = Event::Type::window_close;
                window_data.event_callback(e);
                PostQuitMessage(0);
                break;
            }

            default:
                break;
        }

        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

}

#endif