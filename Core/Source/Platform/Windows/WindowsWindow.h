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

    protected:
        // Inherited via [Window].
        void PollEvents() override;

    private:
        static LRESULT CALLBACK WndProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);

        HINSTANCE m_Module = nullptr;
    };

}