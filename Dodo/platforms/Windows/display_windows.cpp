#include "pch.h"
#include "display_windows.h"
#include "windows_vulkan_context.h"

namespace Dodo {

    Display::WindowId DisplayWindows::window_create(const WindowSpecifications& window_specs) {
        const HMODULE hinstance  = GetModuleHandleW(NULL);

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
            "WindowsDisplay failed to create HWND!"
        );

        const WindowId window = _current_window_id++;
        WindowData& data = _windows[window];
        data.hinstance = hinstance;
        data.hwnd = hwnd;
        data.window = window;
        data.width = window_specs.width;
        data.height = window_specs.height;
        data.title = window_specs.title;

        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&data));
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        SetFocus(hwnd);

        return window;
    }

    void DisplayWindows::window_set_event_callback(WindowId window, EventCallback&& callback) {
        if (_windows.contains(window)) {
            _windows.at(window).event_callback = std::move(callback);
        }
    }

    bool DisplayWindows::window_should_close(WindowId window) const {
        if (_windows.contains(window)) {
            return _windows.at(window).should_close;
        }

        return false;
    }

    void DisplayWindows::window_process_events(WindowId window) {
        if (_windows.contains(window)) {
            MSG msg{};
            ZeroMemory(&msg, sizeof(MSG));
            while (PeekMessageW(&msg, _windows.at(window).hwnd, 0, 0, PM_REMOVE)) {
                // Translate virtual key message into character message.
                TranslateMessage(&msg);
                // Dispatch message to window procedure.
                DispatchMessageW(&msg);
            }
        }
    }

    Ref<RenderContext> DisplayWindows::render_context_create(Renderer::Type type) {
        switch (type) {
            case Renderer::Type::vulkan: return Ref<RenderContextVulkanWindows>::create();
            default: break;
        }

        DODO_VERIFY(false);
        return nullptr;
    }

    LRESULT DisplayWindows::_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        const LONG_PTR user_data = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (!user_data) {
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        }

        auto& window_data = *reinterpret_cast<WindowData*>(user_data);
        switch (msg) {
            case WM_SIZE: {
                const auto width  = static_cast<uint32_t>(LOWORD(lparam));
                const auto height = static_cast<uint32_t>(HIWORD(lparam));
                if (window_data.width == width && window_data.height == height) {
                    break;
                }

                window_data.width  = width;
                window_data.height = height;

                Event e{};
                e.type = Event::Type::window_resize;
                e.resize = { width, height };
                window_data.event_callback(window_data.window, e);
                break;
            }

            case WM_CLOSE: {
                window_data.should_close = true;

                Event e{};
                e.type = Event::Type::window_close;
                window_data.event_callback(window_data.window, e);
                break;
            }

            case WM_QUIT: {
                PostQuitMessage(0);
                break;
            }

            default: {
                break;
            }
        }

        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

}