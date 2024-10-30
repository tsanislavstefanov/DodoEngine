#pragma once

#include "VulkanAdapter.h"
#include "VulkanInstance.h"

namespace Dodo {

    class VulkanDevice : public RefCounted
    {
    public:
        VulkanDevice(Ref<VulkanInstance> instance);
        ~VulkanDevice();

        VkCommandBuffer AllocateThreadLocalCommandBuffer(bool begin = true);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);
        inline Ref<VulkanAdapter> GetAdapter() const { return m_Adapter; }
        inline VkDevice GetVulkanDevice() const { return m_Device; }
        inline VkQueue GetVulkanGraphicsQueue() const { return m_GraphicsQueue; }

    private:
        void RequestExtension(const std::string& name, bool isRequired);
        void ValidateRequestedExtesnions();
        VkCommandPool GetOrCreateThreadLocalCommandPool();

        Ref<VulkanAdapter> m_Adapter = nullptr;
        std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos{};
        std::map<std::string, bool> m_RequestedExtensions{};
        std::vector<const char*> m_EnabledExtensions{};
        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        std::map<std::thread::id, VkCommandPool> m_CommandPoolByThreadID{};
    };

}