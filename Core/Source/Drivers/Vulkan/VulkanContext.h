#pragma once

#include <vulkan/vulkan.h>

#include "VulkanDevice.h"
#include "Renderer/RenderContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN DEBUG MESSENGER //////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanDebugMessenger : public RefCounted
    {
    public:
        VulkanDebugMessenger(VkInstance instance);

        void Destroy();

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_Messenger = VK_NULL_HANDLE;
    };

    ////////////////////////////////////////////////////////////////
    // VULKAN CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanContext : public RenderContext
    {
    public:
        static VulkanContext& GetCurrent() { return *s_Context; }
        static VkInstance GetCurrentInstance() { return s_Context->m_Instance; }
        static Ref<VulkanDevice> GetCurrentDevice() { return s_Context->m_Device; }

        VulkanContext();

        uint32_t GetVulkanApiVersion() const { return m_ApiVersion; }

        VkCommandBuffer AllocateThreadLocalCommandBuffer(bool begin);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);

        // Inherited via [RenderContext].
        void Destroy() override;

    private:
        static VulkanContext* s_Context;

        VkCommandPool GetOrCreateThreadLocalCommandPool();

        uint32_t m_ApiVersion = VK_API_VERSION_1_0;
        std::set<std::string> m_SupportedExtensions{};
        std::set<std::string> m_EnabledExtensions{};
        VkInstance  m_Instance  = VK_NULL_HANDLE;
        const char* m_LayerName = "VK_LAYER_KHRONOS_validation";
        bool m_LayerFound = false;
        Ref<VulkanDebugMessenger> m_DebugMessenger = nullptr;
        Ref<VulkanDevice> m_Device = nullptr;
        std::map<std::thread::id, VkCommandPool> m_CommandPools{};
    };

}