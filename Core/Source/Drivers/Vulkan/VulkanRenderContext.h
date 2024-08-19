#pragma once

#include <vulkan/vulkan.h>

#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "Renderer/RenderContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN RENDER CONTEXT ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanRenderContext : public RenderContext
    {
    public:
        static VulkanRenderContext& GetCurrent()
        {
            return *s_Context;
        }

        static uint32_t GetVulkanApiVersion()
        {
            return s_Context->m_ApiVersion;
        }

        static VkInstance GetVulkanInstance()
        {
            return s_Context->m_Instance;
        }

        static Ref<VulkanDevice> GetCurrentDevice()
        {
            return s_Context->m_Device;
        }

        VulkanRenderContext();

        // Inherited via [RenderContext].
        void Destroy() override;

    private:
        static VulkanRenderContext* s_Context;
        uint32_t                    m_ApiVersion     = VK_API_VERSION_1_0;
        std::set<std::string>       m_SupportedExtensions{};
        std::set<std::string>       m_EnabledExtensions{};
        VkInstance                  m_Instance       = VK_NULL_HANDLE;
        const char*                 m_LayerName      = "VK_LAYER_KHRONOS_validation";
        bool                        m_LayerFound     = false;
        Ref<VulkanDebugMessenger>   m_DebugMessenger = nullptr;
        Ref<VulkanDevice>           m_Device         = nullptr;
    };

}