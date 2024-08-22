#pragma once

#include "VulkanDevice.h"
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

        VulkanContext();

        VkInstance GetVulkanInstance() const
        {
            return m_Instance;
        }

        // Inherited via [RenderContext].
        void Destroy() override;

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL ReportDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                                 void* userData);

        static VulkanContext* s_Context;
        static constexpr uint32_t s_ApiVersion = VK_API_VERSION_1_0;
        std::set<std::string> m_SupportedExtensions{}, m_EnabledExtensions{};
        std::map<std::string, bool> m_RequestedExtensions{};
        const char* m_LayerName = "VK_LAYER_KHRONOS_validation";
        bool m_LayerFound = false;
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_Messenger = VK_NULL_HANDLE;
        Ref<VulkanDevice> m_Device = nullptr;
    };

}