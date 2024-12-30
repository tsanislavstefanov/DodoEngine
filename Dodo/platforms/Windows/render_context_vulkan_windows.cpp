#include "pch.h"

#if defined(DODO_VULKAN) && defined(DODO_WINDOWS)

#include "render_context_vulkan_windows.h"
#include "display_windows.h"

#include <vulkan/vulkan_win32.h>

namespace Dodo {

    SurfaceHandle RenderContextVulkanWindows::surface_create(Display::WindowId window_id, const SurfaceSpecifications& surface_specs, const void* platform_data) {
        const auto& data = *static_cast<const DisplayWindows::PlatformData*>(platform_data);

        VkWin32SurfaceCreateInfoKHR surface_create_info = {};
        surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surface_create_info.hinstance = data.hinstance;
        surface_create_info.hwnd = data.hwnd;
        VkSurfaceKHR surface = nullptr;
        DODO_ASSERT_VK_RESULT(vkCreateWin32SurfaceKHR(instance_get(), &surface_create_info, NULL, &surface));

        auto surface_vk = new SurfaceVulkan();
        surface_vk->surface = surface;
        surface_vk->width = surface_specs.width;
        surface_vk->height = surface_specs.height;
        surface_vk->vsync_mode = surface_specs.vsync_mode;

        _surfaces[window_id] = SurfaceHandle(surface_vk);
        return _surfaces.at(window_id);
    }

    const char* RenderContextVulkanWindows::_get_platform_surface_extension() const {
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    }

}

#endif
