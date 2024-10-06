#include "pch.h"
#include "WindowsInput.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static KeyCode ConvertVirtualKeyToKeyCode(SHORT virtualKey)
        {
            switch (virtualKey)
            {
                case VK_LEFT  : return KeyCode::LeftArrow;
                case VK_UP    : return KeyCode::UpArrow;
                case VK_RIGHT : return KeyCode::RightArrow;
                case VK_DOWN  : return KeyCode::DownArrow;
                case '0'      : return KeyCode::Alpha0;
                case '1'      : return KeyCode::Alpha1;
                case '2'      : return KeyCode::Alpha2;
                case '3'      : return KeyCode::Alpha3;
                case '4'      : return KeyCode::Alpha4;
                case '5'      : return KeyCode::Alpha5;
                case '6'      : return KeyCode::Alpha6;
                case '7'      : return KeyCode::Alpha7;
                case '8'      : return KeyCode::Alpha8;
                case '9'      : return KeyCode::Alpha9;
                case 'A'      : return KeyCode::A;
                default       : DODO_ASSERT(false, "KeyCode not supported!");
            }
            
            return KeyCode::None;
        }
        
        static MouseCode ConvertToMouseCode(SHORT vbutton)
        {
            return MouseCode::None;
        }

    }

    ////////////////////////////////////////////////////////////////
    // WINDOWS KEYBOARD ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    WindowsKeyboard::WindowsKeyboard()
    {
        Reset();
    }

    bool WindowsKeyboard::WndProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                const auto virtualKey = static_cast<SHORT>(wparam);
                const KeyCode keyCode = Utils::ConvertVirtualKeyToKeyCode(virtualKey);
                if (HIWORD(lparam) & KF_UP)
                {
                    OnKeyUp(keyCode);
                }
                else
                {
                    OnKeyDown(keyCode);
                }

                return true;
            }

            default: { break; }
        }

        return false;
    }

    ////////////////////////////////////////////////////////////////
    // WINDOWS MOUSE ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    WindowsMouse::WindowsMouse()
    {
        Reset();
    }

    bool WindowsMouse::WndProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
            case WM_MOUSEWHEEL:
            {
                const auto zDelta = static_cast<int>(GET_WHEEL_DELTA_WPARAM(wparam));
                // Normalize between -1.0 and 1.0.
                const auto wheelDelta = static_cast<float>(zDelta) / WHEEL_DELTA;
                OnWheelScroll(wheelDelta);
            }

            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
            {
                if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN ||
                    msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN)
                {
                }

                return true;
            }

            default: { break; }
        }

        return false;
    }

    ////////////////////////////////////////////////////////////////
    // WINDOWS INPUT DATA //////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct WindowsInputData
    {
        WindowsKeyboard Keyboard{};
        WindowsMouse Mouse{};
    };

    static WindowsInputData s_Data{};

    ////////////////////////////////////////////////////////////////
    // WINDOWS INPUT ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void WindowsInput::Init()
    {
        Keyboard = &s_Data.Keyboard;
        Mouse = &s_Data.Mouse;
    }

    bool WindowsInput::WndProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (s_Data.Keyboard.WndProc(handle, msg, wparam, lparam))
        {
            return true;
        }
        
        if (s_Data.Mouse.WndProc(handle, msg, wparam, lparam))
        {
            return true;
        }
        
        return false;
    }

}