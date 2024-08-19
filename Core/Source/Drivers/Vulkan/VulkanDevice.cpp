#include "pch.h"
#include "VulkanDevice.h"
#include "VulkanCommon.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN DEVICE ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    VulkanDevice::VulkanDevice(VkInstance instance)
    {
        // Request extensions.
        m_PhysicalDevice = Ref<VulkanPhysicalDevice>::Create(instance);
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
        deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        const auto& queueCreateInfos       = m_PhysicalDevice->GetQueueCreateInfos();
        deviceInfo.queueCreateInfoCount    = (uint32_t)queueCreateInfos.size();
        deviceInfo.pQueueCreateInfos       = queueCreateInfos.data();
        deviceInfo.enabledExtensionCount   = (uint32_t)enabledExtensions.size();
        deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
        DODO_VK_RESULT(vkCreateDevice(m_PhysicalDevice->GetVulkanPhysicalDevice(), &deviceInfo, nullptr, &m_Device));

        // Get queue(s).
        const auto& queueFamilyIndices = m_PhysicalDevice->GetQueueFamilyIndices();
        m_GraphicsQueueIndex = queueFamilyIndices.Graphics.value();
        vkGetDeviceQueue(m_Device, m_GraphicsQueueIndex, 0, &m_GraphicsQueue);
    }

    VkCommandBuffer VulkanDevice::AllocateThreadLocalCommandBuffer(bool begin)
    {
        VkCommandBufferAllocateInfo createInfo{};
        createInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        createInfo.commandPool        = GetOrCreateThreadLocalCommandPool();
        createInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
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

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        FlushCommandBuffer(commandBuffer, m_GraphicsQueue);
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
    {
        // End command buffer recording.
        DODO_VK_RESULT(vkEndCommandBuffer(commandBuffer));

        // Submit to graphics queue.
        VkSubmitInfo submitInfo{};
        submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &commandBuffer;
        VkFenceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence    = VK_NULL_HANDLE;
        DODO_VK_RESULT(vkCreateFence(m_Device, &createInfo, nullptr, &fence));
        DODO_VK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

        // Wait for the fence to signal.
        static constexpr uint64_t defaultFenceTimeout = std::numeric_limits<uint64_t>::max();
        DODO_VK_RESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, defaultFenceTimeout));

        // Free resources.
        vkDestroyFence(m_Device, fence, nullptr);
        vkFreeCommandBuffers(m_Device, GetOrCreateThreadLocalCommandPool(), 1, &commandBuffer);
    }

    void VulkanDevice::Destroy()
    {
        vkDestroyDevice(m_Device, nullptr);
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
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = m_GraphicsQueueIndex;
        VkCommandPool commandPool   = VK_NULL_HANDLE;
        DODO_VK_RESULT(vkCreateCommandPool(m_Device, &createInfo, nullptr, &commandPool));
        m_ThreadLocalCommandPools.insert({ threadId, commandPool });
        return commandPool;
    }

}