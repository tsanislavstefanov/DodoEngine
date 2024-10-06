#pragma once

#include "core/window.h"

namespace Dodo {

    class WindowsWindow : public Window
    {
    public:
        WindowsWindow(WindowSpecifications&& specs);

        void process_events() override;

        inline void* native_handle() const override
        {
            return handle_;
        }

        inline HINSTANCE module() const
        {
            return module_;
        }

#ifdef DODO_USE_VULKAN
        std::vector<std::string> get_required_vulkan_instance_extensions() const override;
        VkSurfaceKHR create_vulkan_surface(VkInstance instance) const override;
#endif

    private:
        static LRESULT CALLBACK wnd_proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);

        HINSTANCE module_ = nullptr;
        HWND handle_ = nullptr;
    };

}