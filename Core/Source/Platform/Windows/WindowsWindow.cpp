#include "pch.h"
#include "WindowsWindow.h"
#include "WindowsInput.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOWS WINDOW //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    WindowsWindow::WindowsWindow(const WindowSpecs& specs)
    {
        m_Data.Width = specs.Width;
        m_Data.Height = specs.Height;
        m_Module = GetModuleHandleW(nullptr);

        const WCHAR* className = L"WindowClass";
        WNDCLASSEXW wndClass{};
        ZeroMemory(&wndClass, sizeof(WNDCLASSEXW));
        wndClass.cbSize = sizeof(WNDCLASSEXW);
        wndClass.style = CS_OWNDC;
        wndClass.lpfnWndProc = Win32Proc;
        wndClass.hInstance = m_Module;
        wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wndClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wndClass.lpszClassName = className;
        RegisterClassExW(&wndClass);

        RECT rect{};
        ZeroMemory(&rect, sizeof(RECT));
        rect.right  = (LONG)m_Data.Width;
        rect.bottom = (LONG)m_Data.Height;
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        // Create window.
        const std::wstring title(specs.Title.begin(), specs.Title.end());
        const HWND handle = CreateWindowExW(0,                                              // Optional styles.
                                            className,                                      // Class name.
                                            title.c_str(),                                  // Title.
                                            WS_OVERLAPPEDWINDOW,                            // Style.
                                            CW_USEDEFAULT, CW_USEDEFAULT,                   // Position.
                                            rect.right - rect.left, rect.bottom - rect.top, // Size.
                                            nullptr,                                        // Parent.
                                            nullptr,                                        // Menu.
                                            m_Module,                                       // Module.
                                            nullptr);                                       // Additional application data.

        // Set window user data and show (focused).
        DODO_ASSERT(m_Handle = handle, "Failed to create [Windows] window!");
        SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LPARAM>(&m_Data));
        ShowWindow(handle, SW_SHOWDEFAULT);
        SetFocus(handle);
        
        // Create input devices.
        WindowsInput::Init();
    }

    void WindowsWindow::PollEvents()
    {
        MSG msg{};
        ZeroMemory(&msg, sizeof(MSG));
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // Translate virtual key message into character message.
            TranslateMessage(&msg);
            // Dispatch message to window procedure.
            DispatchMessageW(&msg);
        }
    }

    LRESULT WindowsWindow::Win32Proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Check for input events.
        if (WindowsInput::Win32Proc(handle, msg, wparam, lparam))
        {
            return 0;
        }

        // Handle all other events.
        const LONG_PTR userData = GetWindowLongPtrW(handle, GWLP_USERDATA);
        auto& windowData = *reinterpret_cast<WindowData*>(userData);
        switch (msg)
        {
            case WM_SIZE:
            {
                const auto width  = (uint32_t)LOWORD(lparam);
                const auto height = (uint32_t)HIWORD(lparam);
                if (width == windowData.Width && height == windowData.Height)
                {
                    return 0;
                }

                windowData.Width  = width;
                windowData.Height = height;
                WindowResizeEvent e(width, height);
                windowData.EventCallback(e);
                break;
            }

            case WM_CLOSE:
            {
                WindowCloseEvent e{};
                windowData.EventCallback(e);
                break;
            }

            case WM_QUIT:
            {
                PostQuitMessage(0);
                break;
            }

            default: { break; }
        }

        // Use default procedure.
        return DefWindowProcW(handle, msg, wparam, lparam);
    }

}