#include "pch.h"
#include "Vulkan.h"
#include "VulkanDevice.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN ADAPTER //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    VulkanPhysicalDevice::VulkanPhysicalDevice(VkInstance instance)
    {
        uint32_t physicalDeviceCount = 0;
        DODO_VK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
        DODO_ASSERT(physicalDeviceCount != 0, "GPU that support Vulkan not found, is a driver installed?");
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        DODO_VK_RESULT(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

        // Prefer discrete GPU, due to performance reasons.
        for (const auto& candidate : physicalDevices)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(candidate, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                m_PhysicalDevice = candidate;
                m_Properties = properties;
                break;
            }
        }

        // Get first available GPU as fallback.
        if (!m_PhysicalDevice)
        {
            LOG_CORE_WARNING("No discrete GPU found!");
            m_PhysicalDevice = physicalDevices.back();
            vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
        }

        DODO_ASSERT(m_PhysicalDevice, "No GPU found!");
        // Get supported extensions.
        uint32_t extensionCount = 0;
        DODO_VK_RESULT(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        DODO_VK_RESULT(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, extensions.data()));
        for (const auto& extension : extensions)
        {
            m_SupportedExtensions.insert(extension.extensionName);
        }

        // Get vendor.
        struct Vendor
        {
            uint32_t    ID   = 0;
            const char* Name = nullptr;
        };

        Vendor vendors[] = {
                { 0x1002, "AMD"      },
                { 0x1010, "ImgTec"   },
                { 0x10DE, "NVIDIA"   },
                { 0x13B5, "ARM"      },
                { 0x5143, "Qualcomm" },
                { 0x8086, "Intel"    }
        };

        for (const auto& vendor : vendors)
        {
            if (vendor.ID == m_Properties.vendorID)
            {
                m_VendorName = vendor.Name;
                break;
            }
        }

        LOG_CORE_INFO("Selected GPU...");
        LOG_CORE_INFO("    Vendor: {0}.", m_VendorName);
        LOG_CORE_INFO("    Name: {0}.", m_Properties.deviceName);
        LOG_CORE_INFO("    API version: {0}.{1}.{2}.", VK_API_VERSION_MAJOR(m_Properties.apiVersion), VK_API_VERSION_MINOR(m_Properties.apiVersion), VK_API_VERSION_PATCH(m_Properties.apiVersion));

        // Get queue family indices.
        m_QueueFamilyIndices = FindQueueFamilyIndices();
        if (m_QueueFamilyIndices.Graphics.has_value())
        {
            const float defaultQueuePriority = 1.0f;
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.Graphics.value();
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &defaultQueuePriority;
            m_QueueCreateInfos.push_back(queueCreateInfo);
        }
    }

    QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilyIndices() const
    {
        // Get available queues.
        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, queues.data());

        // Find family indices.
        QueueFamilyIndices result{};
        uint32_t queueIndex = 0;
        for (const auto& queue : queues)
        {
            if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                result.Graphics = queueIndex;
                break;
            }

            queueIndex++;
        }

        DODO_ASSERT(result.Graphics.has_value(), "Graphics queue not found!");
        return result;
    }

    bool VulkanPhysicalDevice::IsExtensionSupported(const std::string& extensionName) const
    {
        return m_SupportedExtensions.contains(extensionName);
    }

    ////////////////////////////////////////////////////////////////
    // VULKAN DEVICE ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    VulkanDevice::VulkanDevice(VkInstance instance)
    {
        m_PhysicalDevice = Ref<VulkanPhysicalDevice>::Create(instance);
        const auto& queueCreateInfos = m_PhysicalDevice->GetQueueCreateInfos();

        // Request extensions.
        std::map<std::string, bool> requestedExtensions{};
        requestedExtensions.insert({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, true });

        // Enable requested & supported extensions.
        for (auto& [name, required] : requestedExtensions)
        {
            if (!m_PhysicalDevice->IsExtensionSupported(name))
            {
                if (required) // Required extension not supported?
                {
                    DODO_ASSERT(false, "Required instance extension {0} not found, is a driver installed?", name);
                }
                else // Optional extension not supported?
                {
                    LOG_CORE_WARNING("Optional instance extension {0} not found!", name);
                    continue;
                }
            }

            m_EnabledExtensions.insert(name);
        }

        // Prepare enabled extensions for Vulkan.
        std::vector<const char*> enabledExtensions{};
        for (const auto& extension : m_EnabledExtensions)
        {
            enabledExtensions.push_back(extension.c_str());
        }

        // Create device.
        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
        deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
        DODO_VK_RESULT(vkCreateDevice(m_PhysicalDevice->GetNativePhysicalDevice(), &deviceInfo, nullptr, &m_Device));

        // Get queue(s).
        const auto& queueFamilyIndices = m_PhysicalDevice->GetQueueFamilyIndices();
        vkGetDeviceQueue(m_Device, queueFamilyIndices.Graphics.value(), 0, &m_GraphicsQueue);
    }

    void VulkanDevice::Destroy()
    {
        vkDestroyDevice(m_Device, nullptr);
    }

}