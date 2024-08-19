#pragma once

#include <vulkan/vulkan.h>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // QUEUE FAMILY INDICES ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> Graphics{};
    };

    ////////////////////////////////////////////////////////////////
    // VULKAN PHYSICAL DEVICE //////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanPhysicalDevice : public RefCounted
    {
    public:
        VulkanPhysicalDevice(VkInstance instance);

        bool IsExtensionSupported(const std::string& extensionName) const;

        VkPhysicalDevice GetVulkanPhysicalDevice() const
        {
            return m_PhysicalDevice;
        }
        
        std::string GetVendorName () const
        {
            return m_VendorName;
        }
        
        const QueueFamilyIndices& GetQueueFamilyIndices() const
        {
            return m_QueueFamilyIndices;
        }

        const std::vector<VkDeviceQueueCreateInfo>& GetQueueCreateInfos() const
        {
            return m_QueueCreateInfos;
        }

    private:
        QueueFamilyIndices FindQueueFamilyIndices() const;

        VkPhysicalDevice                     m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties           m_Properties{};
        std::set<std::string>                m_SupportedExtensions{};
        const char*                          m_VendorName     = "Unknown";
        QueueFamilyIndices                   m_QueueFamilyIndices{};
        std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos{};
    };

}