#pragma once

#include "Renderer/RenderDevice.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN DEVICE ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanDevice : public RenderDevice
    {
    public:
        VulkanDevice(VkInstance instance);

        VkCommandBuffer AllocateThreadLocalCommandBuffer(bool begin);
        void FlushCommandBuffer(VkCommandBuffer cmdBuffer);
        void FlushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue);

        // Inherited via [RenderDevice].
        void Destroy() override;

    private:
        ////////////////////////////////////////////////////////////
        // QUEUE FAMILY INDICES ////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct QueueFamilyIndices
        {
            std::optional<uint32_t> Graphics{};
        };

        VkPhysicalDevice SelectPhysicalDevice() const;
        const char* FindVendorName() const;
        QueueFamilyIndices FindQueueFamilyIndices() const;
        VkCommandPool GetOrCreateThreadLocalCommandPool();

        VkInstance m_Instance;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_Properties{};
        std::set<std::string> m_SupportedExtensions{};
        const char* m_VendorName = nullptr;
        uint32_t m_GraphicsQueueIndex = std::numeric_limits<uint32_t>::max();
        std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos{};
        std::set<std::string> m_EnabledExtensions{};
        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        std::map<std::thread::id, VkCommandPool> m_ThreadLocalCommandPools{};
    };

}