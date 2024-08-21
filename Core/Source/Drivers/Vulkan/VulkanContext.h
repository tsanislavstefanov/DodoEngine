#pragma once

#include <vulkan/vulkan.h>

#include "Renderer/RenderContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanContext : public RenderContext
    {
    public:
        static VulkanContext& GetCurrent()
        {
            return *s_Context;
        }

        static uint32_t GetVulkanApiVersion()
        {
            return s_ApiVersion;
        }

        static VkInstance GetVulkanInstance()
        {
            return s_Context->m_Instance;
        }

        VulkanContext();

        // Inherited via [RenderContext].
        Ref<RenderDeviceDriver> CreateDeviceDriver() const override;

        void Destroy() override;

        Ref<Swapchain> GetSwapchain() const override;

    private:
        static VulkanContext* s_Context;
        static constexpr uint32_t s_ApiVersion = VK_API_VERSION_1_0;
        std::set<std::string> m_SupportedExtensions{}, m_EnabledExtensions{};
        std::map<std::string, bool> m_RequestedExtensions{};
        const char* m_LayerName = "VK_LAYER_KHRONOS_validation";
        bool m_LayerFound = false;
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_Messenger = VK_NULL_HANDLE;
    };

}