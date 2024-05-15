#include "pch.h"
#include "WindowManager.h"
#include "Window.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOW MANAGER //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    std::vector<Window*> WindowManager::s_Windows{};

    Window* WindowManager::GetWindowByHandle(void* handle)
    {
        auto found = std::ranges::find_if(s_Windows, [handle](Window* window) {
            return handle == window->GetHandle();
        });

        if (found == s_Windows.end())
        {
            return nullptr;
        }

        return *found;
    }

    void WindowManager::AddWindow(Window* window)
    {
        s_Windows.push_back(window);
    }

}