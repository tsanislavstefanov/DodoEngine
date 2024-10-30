#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanDevice.h"

namespace Dodo {

    VulkanDevice::VulkanDevice(Ref<VulkanInstance> instance)
        : m_Adapter{Ref<VulkanAdapter>::Create(instance)}
    {
        static constexpr auto defaultQueuePriority = 1.0f;
        VkDeviceQueueCreateInfo& queueCreateInfo = m_QueueCreateInfos.emplace_back();
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = m_Adapter->GetGraphicsQueueIndex();
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &defaultQueuePriority;

        RequestExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);
        ValidateRequestedExtesnions();

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(m_QueueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = m_QueueCreateInfos.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_EnabledExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = m_EnabledExtensions.data();
        DODO_VERIFY_VK_RESULT(vkCreateDevice(m_Adapter->GetVulkanPhysicalDevice(), &deviceCreateInfo, nullptr, &m_Device));

        vkGetDeviceQueue(m_Device, m_Adapter->GetGraphicsQueueIndex(), 0, &m_GraphicsQueue);
    }

    VulkanDevice::~VulkanDevice()
    {
        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(m_Device));
        vkDestroyDevice(m_Device, nullptr);
    }

    VkCommandBuffer VulkanDevice::AllocateThreadLocalCommandBuffer(bool begin)
    {
        VkCommandBufferAllocateInfo commandBufferAllocInfo{};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool = GetOrCreateThreadLocalCommandPool();
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        DODO_VERIFY_VK_RESULT(vkAllocateCommandBuffers(m_Device, &commandBufferAllocInfo, &commandBuffer));
        if (begin)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            DODO_VERIFY_VK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        }

        return commandBuffer;
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer)
    {
        FlushCommandBuffer(commandBuffer, m_GraphicsQueue);
    }

    void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
    {
        DODO_VERIFY_VK_RESULT(vkEndCommandBuffer(commandBuffer));

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence = VK_NULL_HANDLE;
        DODO_VERIFY_VK_RESULT(vkCreateFence(m_Device, &fenceInfo, nullptr, &fence));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        DODO_VERIFY_VK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));

        constexpr uint64_t defaultTimeout = std::numeric_limits<uint64_t>::max();
        DODO_VERIFY_VK_RESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, defaultTimeout));

        vkDestroyFence(m_Device, fence, nullptr);
        vkFreeCommandBuffers(m_Device, GetOrCreateThreadLocalCommandPool(), 1, &commandBuffer);
    }

    void VulkanDevice::RequestExtension(const std::string& name, bool isRequired)
    {
        DODO_ASSERT(!m_RequestedExtensions.contains(name), "Vulkan device extension [{0}] already requested!", name);
        m_RequestedExtensions.insert({ name, isRequired });
    }

    void VulkanDevice::ValidateRequestedExtesnions()
    {
        for (const auto& [name, isRequired] : m_RequestedExtensions)
        {
            if (!m_Adapter->IsExtensionSupported(name))
            {
                if (isRequired)
                {
                    DODO_ASSERT(false, "Required Vulkan device extension[{0}] not supported!", name);
                }
                else
                {
                    DODO_LOG_WARNING_TAG("Renderer", "Vulkan device extension [{0}] not supported!", name);
                    continue;
                }
            }

            m_EnabledExtensions.push_back(name.c_str());
        }
    }

    VkCommandPool VulkanDevice::GetOrCreateThreadLocalCommandPool()
    {
        const std::thread::id thisThreadID = std::this_thread::get_id();
        if (!m_CommandPoolByThreadID.contains(thisThreadID))
        {
            VkCommandPoolCreateInfo poolCreateInfo{};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolCreateInfo.queueFamilyIndex = m_Adapter->GetGraphicsQueueIndex();
            VkCommandPool commandPool = VK_NULL_HANDLE;
            DODO_VERIFY_VK_RESULT(vkCreateCommandPool(m_Device, &poolCreateInfo, nullptr, &commandPool));
            m_CommandPoolByThreadID.insert({ thisThreadID, commandPool });
        }

        return m_CommandPoolByThreadID.at(thisThreadID);
    }

}