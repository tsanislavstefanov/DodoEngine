#include "pch.h"
#include "WindowsWindow.h"
#include "WindowsInput.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOWS WINDOW //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    WindowsWindow::WindowsWindow(const WindowSpecs& specs)
    {
        m_Data.Width  = specs.Width;
        m_Data.Height = specs.Height;
        m_Data.Title  = specs.Title;
        m_Module      = GetModuleHandleW(nullptr);

        const WCHAR* className = L"WindowClass";
        WNDCLASSEXW wndclass{};
        ZeroMemory(&wndclass, sizeof(WNDCLASSEXW));
        wndclass.cbSize        = sizeof(WNDCLASSEXW);
        wndclass.style         = CS_OWNDC;
        wndclass.lpfnWndProc   = Win32Proc;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = 0;
        wndclass.hInstance     = m_Module;
        wndclass.hIcon         = nullptr;
        wndclass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wndclass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wndclass.lpszMenuName  = L"";
        wndclass.lpszClassName = className;
        wndclass.hIconSm       = nullptr;
        RegisterClassExW(&wndclass);

        RECT rect{};
        ZeroMemory(&rect, sizeof(RECT));
        rect.left   = 0;
        rect.top    = 0;
        rect.right  = static_cast<LONG>(m_Data.Width );
        rect.bottom = static_cast<LONG>(m_Data.Height);
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        const std::wstring title(m_Data.Title.begin(), m_Data.Title.end());
        m_Handle = CreateWindowExW(0,                                              // Optional styles.
                                   className,                                      // Class name.
                                   title.c_str(),                                  // Title.
                                   WS_OVERLAPPEDWINDOW,                            // Style.
                                   CW_USEDEFAULT, CW_USEDEFAULT,                   // Position.
                                   rect.right - rect.left, rect.bottom - rect.top, // Size.
                                   nullptr,                                        // Parent.
                                   nullptr,                                        // Menu.
                                   m_Module,                                       // Module.
                                   nullptr);                                       // Additional application data.

        SetWindowLongPtrW(m_Handle, GWLP_USERDATA, reinterpret_cast<LPARAM>(&m_Data));
        ShowWindow(m_Handle, SW_SHOWDEFAULT);
        SetFocus(m_Handle);

        // Create [Windows] input devices.
        WindowsInput::Init();

        // Create render context & swapchain.
        m_RenderContext = RenderContext::Create();
        m_Swapchain = Swapchain::Create(m_Handle, m_Data.Width, m_Data.Height);
    }

    void WindowsWindow::SetTitle(const std::string& title)
    {
        const std::wstring text(title.begin(), title.end());
        SetWindowTextW(m_Handle, text.c_str());
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

    void WindowsWindow::Destroy()
    {
        // Swapchain has to be destroyed before the context,
        // because the Swapchain needs the context.
        m_Swapchain->Destroy();
        m_RenderContext->Destroy();
    }

    LRESULT WindowsWindow::Win32Proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        const LONG_PTR userData = GetWindowLongPtrW(handle, GWLP_USERDATA);
        auto& windowData = *reinterpret_cast<WindowData*>(userData);
        switch (msg)
        {
            case WM_SETFOCUS:
            {
                windowData.HasFocus = true;
                break;
            }

            case WM_KILLFOCUS:
            {
                windowData.HasFocus = false;
                break;
            }

            case WM_SIZE:
            {
                const auto width  = static_cast<uint32_t>(LOWORD(lparam));
                const auto height = static_cast<uint32_t>(HIWORD(lparam));
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

            case WM_DESTROY:
            {
                PostQuitMessage(0);
                break;
            }

            default: { break; }
        }

        if (WindowsInput::Win32Proc(handle, msg, wparam, lparam))
        {
            return 0;
        }

        // Use default procedure.
        return DefWindowProcW(handle, msg, wparam, lparam);
    }

}