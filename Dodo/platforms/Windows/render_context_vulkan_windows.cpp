#include "pch.h"

#if defined(DODO_VULKAN) && defined(DODO_WINDOWS)

#include "render_context_vulkan_windows.h"
#include "display_windows.h"

#include <vulkan/vulkan_win32.h>

namespace Dodo {

    SurfaceHandle RenderContextVulkanWindows::surface_create(Display::WindowId window, const SurfaceSpecifications& surface_specs, const void* platform_data) {
        const auto& data = *static_cast<const DisplayWindows::PlatformData*>(platform_data);

        VkWin32SurfaceCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        create_info.hinstance = data.hinstance;
        create_info.hwnd = data.hwnd;
        VkSurfaceKHR vk_surface = nullptr;
        DODO_ASSERT_VK_RESULT(vkCreateWin32SurfaceKHR(instance_get(), &create_info, NULL, &vk_surface));

        SurfaceInfo surface_info = {};
        surface_info.vk_surface = vk_surface;
        surface_info.width = surface_specs.width;
        surface_info.height = surface_specs.height;
        surface_info.vsync_mode = surface_specs.vsync_mode;

        SurfaceHandle surface = _surface_owner.create(surface_info);
        _surfaces[window] = surface;
        return surface;
    }

    const char* RenderContextVulkanWindows::_get_platform_surface_extension() const {
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    }

}

#endif