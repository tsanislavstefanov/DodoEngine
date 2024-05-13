#pragma once

#include "Core/Window.h"

////////////////////////////////////////////////////////////////
// WINDOWS WINDOW //////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class WindowsWindow : public Window
{
public:
    explicit WindowsWindow(const WindowSpecs& specs);

    [[nodiscard]] HINSTANCE GetModule() const
    {
        return m_Module;
    }

    // Inherited via [Window].
    [[nodiscard]] void* GetHandle() const override
    {
        return m_Handle;
    }

    void SetTitle(const std::string& title) override;

    void PollEvents() override;

    void Dispose() override;

private:
    static LRESULT CALLBACK Win32Proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);

    HINSTANCE m_Module = nullptr;
    HWND      m_Handle = nullptr;
};