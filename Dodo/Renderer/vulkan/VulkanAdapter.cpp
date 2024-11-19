#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanAdapter.h"

namespace Dodo {

    VulkanAdapter::VulkanAdapter(Ref<VulkanInstance> instance)
        : m_Instance{instance}
    {
        QueryAvailableGPUs();
        m_PhysicalDevice = SelectSuitableGPU();
        DODO_ASSERT(m_PhysicalDevice, "No suitable GPU found!");
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);

        QuerySupportedQueueFamilies();
        m_GraphicsQueueIndex = FindQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        DODO_ASSERT(m_GraphicsQueueIndex.has_value(), "Graphics queue not found!");

        QuerySupportedExtensions();
    }

    std::string VulkanAdapter::GetVendorName() const
    {
        static std::map<uint32_t, std::string> vendorTable = {
            { 0x1002, "AMD"      },
            { 0x1010, "ImgTec"   },
            { 0x10DE, "NVIDIA"   },
            { 0x13B5, "ARM"      },
            { 0x5143, "Qualcomm" },
            { 0x8086, "Intel"    }
        };

        return vendorTable.contains(m_Properties.vendorID) ? vendorTable.at(m_Properties.vendorID) : "Unknown";
    }

    void VulkanAdapter::QueryAvailableGPUs()
    {
        uint32_t gpuCount = 0;
        DODO_VERIFY_VK_RESULT(vkEnumeratePhysicalDevices(m_Instance->GetVulkanInstance(), &gpuCount, nullptr));
        DODO_ASSERT(gpuCount != 0, "GPU that support Vulkan not found!");
        m_AvailableGPUs.resize(gpuCount);
        DODO_VERIFY_VK_RESULT(vkEnumeratePhysicalDevices(m_Instance->GetVulkanInstance(), &gpuCount, m_AvailableGPUs.data()));
    }

    VkPhysicalDevice VulkanAdapter::SelectSuitableGPU() const
    {
        for (auto candidate : m_AvailableGPUs)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(candidate, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                return candidate;
        }

        DODO_LOG_WARNING_TAG("Renderer", "No discrete GPU found!");
        return m_AvailableGPUs.back();
    }

    void VulkanAdapter::QuerySupportedQueueFamilies()
    {
        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, nullptr);
        m_SupportedQueueFamilies.resize(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, m_SupportedQueueFamilies.data());
    }

    std::optional<uint32_t> VulkanAdapter::FindQueueFamilyIndex(VkQueueFlags queueFamily) const
    {
        uint32_t queueIndex = 0;
        for (const auto& queue : m_SupportedQueueFamilies)
        {
            if (queue.queueFlags & queueFamily)
                return queueIndex;

            queueIndex++;
        }

        return std::nullopt;
    }

    void VulkanAdapter::QuerySupportedExtensions()
    {
        uint32_t extensionCount = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        DODO_VERIFY_VK_RESULT(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, extensions.data()));
        for (const auto& extension : extensions)
            m_SupportedExtensions.insert(extension.extensionName);
    }

}