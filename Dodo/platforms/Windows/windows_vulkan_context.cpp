#include "pch.h"
#include "windows_vulkan_context.h"

#include <vulkan/vulkan_win32.h>

namespace Dodo {

    SurfaceHandle RenderContextVulkanWindows::surface_create(Display::WindowId window) const {
        return SurfaceHandle();
    }

    const char* RenderContextVulkanWindows::_get_platform_surface_extension() const {
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    }

}