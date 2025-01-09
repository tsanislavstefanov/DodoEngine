#include "pch.h"

#if defined(DODO_VULKAN) && defined(DODO_WINDOWS)

#include "render_context_vulkan_windows.h"
#include "display_windows.h"

#include <vulkan/vulkan_win32.h>

namespace Dodo {

    SurfaceHandle RenderContextVulkanWindows::surface_create(Display::WindowId window_id, const SurfaceSpecifications& surface_specs, const void* platform_data) {
        const auto& data = *static_cast<const DisplayWindows::PlatformData*>(platform_data);

        VkWin32SurfaceCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        create_info.hinstance = data.hinstance;
        create_info.hwnd = data.hwnd;
        VkSurfaceKHR surface_vk = nullptr;
        DODO_ASSERT_VK_RESULT(vkCreateWin32SurfaceKHR(instance_get(), &create_info, NULL, &surface_vk));

        auto surface = new Surface();
        surface->surface_vk = surface_vk;
        surface->width = surface_specs.width;
        surface->height = surface_specs.height;
        surface->vsync_mode = surface_specs.vsync_mode;

        _surfaces[window_id] = SurfaceHandle(surface);
        return _surfaces.at(window_id);
    }

    const char* RenderContextVulkanWindows::_get_platform_surface_extension() const {
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    }

}

#endif