#include "pch.h"
#include "Vulkan.h"
#include "VulkanDevice.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN DEVICE ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    VulkanDevice::VulkanDevice(VkInstance instance)
        : m_Instance(instance)
    {
        m_PhysicalDevice = SelectPhysicalDevice();
        DODO_ASSERT(m_PhysicalDevice, "No GPU found!");
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
        
        uint32_t extensionCount = 0;
        DODO_VK_RESULT(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        DODO_VK_RESULT(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, extensions.data()));
        for (const auto& extension : extensions)
        {
            m_SupportedExtensions.insert(extension.extensionName);
        }

        m_VendorName = FindVendorName();
        LOG_CORE_INFO("Selected GPU...");
        LOG_CORE_INFO("    Vendor: {0}.", m_VendorName);
        LOG_CORE_INFO("    Device name: {0}.", m_Properties.deviceName);
        LOG_CORE_INFO("    API version: {0}.{1}.{2}.", VK_API_VERSION_MAJOR(m_Properties.apiVersion), VK_API_VERSION_MINOR(m_Properties.apiVersion), VK_API_VERSION_PATCH(m_Properties.apiVersion));

        auto queueFamilyIndices = FindQueueFamilyIndices();
        DODO_ASSERT(queueFamilyIndices.Graphics.has_value(), "Graphics queue not found!");
        m_GraphicsQueueIndex = queueFamilyIndices.Graphics.value();

        const float defaultQueuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = m_GraphicsQueueIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &defaultQueuePriority;
        m_QueueCreateInfos.push_back(queueCreateInfo);

        std::map<std::string, bool> requestedExtensions{};
        requestedExtensions.insert({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, true });

        for (auto& [name, required] : requestedExtensions)
        {
            if (!m_SupportedExtensions.contains(name))
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

        // This step is required to prepare the data
        // in a Vulkan friendly format.
        std::vector<const char*> enabledExtensions{};
        for (const auto& extension : m_EnabledExtensions)
        {
            enabledExtensions.push_back(extension.c_str());
        }

        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(m_QueueCreateInfos.size());
        deviceInfo.pQueueCreateInfos = m_QueueCreateInfos.data();
        deviceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
        deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
        DODO_VK_RESULT(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device));
        vkGetDeviceQueue(m_Device, m_GraphicsQueueIndex, 0, &m_GraphicsQueue);
    }

    VkCommandBuffer VulkanDevice::AllocateThreadLocalCommandBuffer(bool begin)
    {
        VkCommandBufferAllocateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        createInfo.commandPool = GetOrCreateThreadLocalCommandPool();
        createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        createInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        DODO_VK_RESULT(vkAllocateCommandBuffers(m_Device, &createInfo, &commandBuffer));
        if (begin)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            DODO_VK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        }

        return commandBuffer;
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer cmdBuffer)
    {
        FlushCommandBuffer(cmdBuffer, m_GraphicsQueue);
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer cmdBuffer, VkQueue queue)
    {
        DODO_VK_RESULT(vkEndCommandBuffer(cmdBuffer));

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence = VK_NULL_HANDLE;
        DODO_VK_RESULT(vkCreateFence(m_Device, &fenceInfo, nullptr, &fence));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        DODO_VK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

        const uint64_t defaultFenceTimeout = std::numeric_limits<uint64_t>::max();
        DODO_VK_RESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, defaultFenceTimeout));

        vkDestroyFence(m_Device, fence, nullptr);
        vkFreeCommandBuffers(m_Device, GetOrCreateThreadLocalCommandPool(), 1, &cmdBuffer);
    }

    void VulkanDevice::Destroy()
    {
        DODO_VK_RESULT(vkDeviceWaitIdle(m_Device));
        vkDestroyDevice(m_Device, nullptr);
    }

    VkPhysicalDevice VulkanDevice::SelectPhysicalDevice() const
    {
        uint32_t physicalDeviceCount = 0;
        DODO_VK_RESULT(vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr));
        DODO_ASSERT(physicalDeviceCount != 0, "GPU that support Vulkan not found, is a driver installed?");
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        DODO_VK_RESULT(vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.data()));
        for (auto candidate : physicalDevices)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(candidate, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                return candidate;
            }
        }

        return physicalDevices.back();
    }

    const char* VulkanDevice::FindVendorName() const
    {
        struct Vendor
        {
            uint32_t ID = 0;
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
                return vendor.Name;
            }
        }

        return "Unknown";
    }

    VulkanDevice::QueueFamilyIndices VulkanDevice::FindQueueFamilyIndices() const
    {
        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, queues.data());
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

        return result;
    }

    VkCommandPool VulkanDevice::GetOrCreateThreadLocalCommandPool()
    {
        const auto threadId = std::this_thread::get_id();
        auto found = m_ThreadLocalCommandPools.find(threadId);
        if (found != m_ThreadLocalCommandPools.end())
        {
            return found->second;
        }

        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = m_GraphicsQueueIndex;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        DODO_VK_RESULT(vkCreateCommandPool(m_Device, &createInfo, nullptr, &commandPool));
        m_ThreadLocalCommandPools.insert({ threadId, commandPool });
        return commandPool;
    }

}