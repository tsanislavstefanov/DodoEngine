#include "pch.h"
#include "windows_window.h"
#ifdef DODO_USE_VULKAN
#   include <vulkan/vulkan_win32.h>
#endif

namespace Dodo {

    WindowsWindow::WindowsWindow(WindowSpecifications&& specs)
        : Window(std::forward<WindowSpecifications>(specs))
    {
        module_ = GetModuleHandleW(NULL);
        const auto class_name = L"WindowClass";

        WNDCLASSEXW wnd_class{};
        ZeroMemory(&wnd_class, sizeof(WNDCLASSEXW));
        wnd_class.cbSize = sizeof(WNDCLASSEXW);
        wnd_class.style = CS_OWNDC;
        wnd_class.lpfnWndProc = WindowsWindow::wnd_proc;
        wnd_class.hInstance = module_;
        wnd_class.hCursor = LoadCursor(NULL, IDC_ARROW);
        wnd_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wnd_class.lpszClassName = class_name;
        RegisterClassExW(&wnd_class);

        RECT rect{};
        ZeroMemory(&rect, sizeof(RECT));
        rect.right  = (LONG)specifications_.width ;
        rect.bottom = (LONG)specifications_.height;
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        const std::wstring title(specs.title.begin(), specs.title.end());
        handle_ = CreateWindowExW(
            0,
            class_name,
            title.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            NULL,
            NULL,
            module_,
            NULL);

        SetWindowLongPtrW(handle_, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
        ShowWindow(handle_, SW_SHOWDEFAULT);
        SetFocus(handle_);
    }

    void WindowsWindow::process_events()
    {
        MSG msg{};
        ZeroMemory(&msg, sizeof(MSG));
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // Translate virtual key message into character message.
            TranslateMessage(&msg);
            // Dispatch message to window procedure.
            DispatchMessageW(&msg);
        }
    }

    LRESULT WindowsWindow::wnd_proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        auto user_data = GetWindowLongPtrW(handle, GWLP_USERDATA);
        if (!user_data)
        {
            return DefWindowProcW(handle, msg, wparam, lparam); 
        }

        auto& window = *reinterpret_cast<WindowsWindow*>(user_data);
        switch (msg)
        {
            case WM_SIZE:
            {
                auto& specs = window.specifications_;
                const auto width  = (uint32_t)LOWORD(lparam);
                const auto height = (uint32_t)HIWORD(lparam);
                if (specs.width == width && specs.height == height)
                    break;

                specs.width  = width;
                specs.height = height;
                WindowResizeEvent e{ specs.width, specs.height };
                window.on_resized(e);
                break;
            }

            case WM_SETFOCUS:
            {
                window.has_focus_ = true;
                break;
            }

            case WM_KILLFOCUS:
            {
                window.has_focus_ = false;
                break;
            }

            case WM_CLOSE:
            {
                window.on_close();
                break;
            }

            case WM_QUIT:
            {
                PostQuitMessage(0);
                break;
            }

            default:
                break;
        }

        return DefWindowProcW(handle, msg, wparam, lparam);
    }

#ifdef DODO_USE_VULKAN
    std::vector<std::string> WindowsWindow::get_required_vulkan_instance_extensions() const
    {
        return { VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    }

    VkSurfaceKHR WindowsWindow::create_vulkan_surface(VkInstance instance) const
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkWin32SurfaceCreateInfoKHR surface_create_info{};
        surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surface_create_info.hinstance = module_;
        surface_create_info.hwnd = handle_;
        DODO_VERIFY_VK_RESULT(vkCreateWin32SurfaceKHR(instance, &surface_create_info, VK_NULL_HANDLE, &surface));
        return surface;
    }
#endif

}