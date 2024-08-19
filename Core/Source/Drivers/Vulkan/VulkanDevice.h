#pragma once

#include <vulkan/vulkan.h>

#include "VulkanPhysicalDevice.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN DEVICE ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanDevice : public RefCounted
    {
    public:
        VulkanDevice(VkInstance instance);

        VkCommandBuffer AllocateThreadLocalCommandBuffer(bool begin);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer);
        void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);
        void Destroy();

        Ref<VulkanPhysicalDevice> GetPhysicalDevice() const
        {
            return m_PhysicalDevice;
        }

        VkDevice GetVulkanDevice () const
        {
            return m_Device;
        }
        
        uint32_t GetGraphicsQueueIndex() const
        {
            return m_GraphicsQueueIndex;
        }

        VkQueue GetGraphicsVulkanQueue() const
        {
            return m_GraphicsQueue;
        }

    private:
        VkCommandPool GetOrCreateThreadLocalCommandPool();

        Ref<VulkanPhysicalDevice>                m_PhysicalDevice     = nullptr;
        std::set<std::string>                    m_EnabledExtensions{};
        VkDevice                                 m_Device             = VK_NULL_HANDLE;
        uint32_t                                 m_GraphicsQueueIndex = UINT32_MAX;
        VkQueue                                  m_GraphicsQueue      = VK_NULL_HANDLE;
        std::map<std::thread::id, VkCommandPool> m_ThreadLocalCommandPools{};
    };

}