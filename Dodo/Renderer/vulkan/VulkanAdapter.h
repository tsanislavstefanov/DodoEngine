#pragma once

#include "VulkanInstance.h"

namespace Dodo {

    class VulkanAdapter : public RefCounted
    {
    public:
        VulkanAdapter(Ref<VulkanInstance> instance);

        inline bool IsExtensionSupported(const std::string& name) const { return m_SupportedExtensions.contains(name); }
        std::string GetVendorName() const;
        inline VkPhysicalDevice GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
        inline const std::vector<VkQueueFamilyProperties>& GetSupportedVulkanQueueFamilies() const { return m_SupportedQueueFamilies; }
        inline uint32_t GetGraphicsQueueIndex() const { return m_GraphicsQueueIndex.value(); }

    private:
        void QueryAvailableGPUs();
        VkPhysicalDevice SelectSuitableGPU() const;
        void QuerySupportedQueueFamilies();
        std::optional<uint32_t> FindQueueFamilyIndex(VkQueueFlags family) const;
        void QuerySupportedExtensions();

        Ref<VulkanInstance> m_Instance = nullptr;
        std::vector<VkPhysicalDevice> m_AvailableGPUs{};
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_Properties{};
        std::vector<VkQueueFamilyProperties> m_SupportedQueueFamilies{};
        std::optional<uint32_t> m_GraphicsQueueIndex{};
        std::set<std::string> m_SupportedExtensions{};
    };

}