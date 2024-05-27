#pragma once

#include "Core/Window.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOWS WINDOW //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(const WindowSpecs& specs);

        // Inherited via [Window].
        void* GetHandle() const override { return m_Handle; }
        void  SetTitle(const std::string& title) override;
        void  PollEvents() override;
        void  Destroy() override;

    private:
        static LRESULT CALLBACK Win32Proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);

        HINSTANCE m_Module = nullptr;
        HWND m_Handle = nullptr;
    };

}