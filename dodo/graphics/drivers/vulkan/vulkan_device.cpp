#include "pch.h"
#include "vulkan.h"
#include "vulkan_device.h"

namespace Dodo {

    VulkanDevice::VulkanDevice(Ref<VulkanInstance> instance, Ref<VulkanAdapter> adapter)
        : instance_(instance), adapter_(adapter)
    {
        initialize_queue_create_infos();
        request_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);
        validate_requested_extensions();

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = (uint32_t)queue_create_infos_.size();
        device_create_info.pQueueCreateInfos = queue_create_infos_.data();
        device_create_info.enabledExtensionCount = (uint32_t)enabled_extensions_.size();
        device_create_info.ppEnabledExtensionNames = enabled_extensions_.data();
        DODO_VERIFY_VK_RESULT(vkCreateDevice(adapter_->get_vulkan_physical_device(), &device_create_info, nullptr, &device_));

        vkGetDeviceQueue(device_, adapter_->graphics_queue_index(), 0, &graphics_queue_);
    }

    VulkanDevice::~VulkanDevice()
    {
        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(device_));
        vkDestroyDevice(device_, nullptr);
    }

    VkCommandBuffer VulkanDevice::allocate_thread_local_cmd_buffer(bool begin_immediately /* = true */)
    {
        VkCommandBufferAllocateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        create_info.commandPool = get_or_create_thread_local_cmd_pool();
        create_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        create_info.commandBufferCount = 1;
        VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
        DODO_VERIFY_VK_RESULT(vkAllocateCommandBuffers(device_, &create_info, &cmd_buffer));
        if (begin_immediately)
        {
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            DODO_VERIFY_VK_RESULT(vkBeginCommandBuffer(cmd_buffer, &begin_info));
        }

        return cmd_buffer;
    }

    void VulkanDevice::flush_cmd_buffer(VkCommandBuffer cmd_buffer)
    {
        flush_cmd_buffer(cmd_buffer, graphics_queue_);
    }

    void VulkanDevice::flush_cmd_buffer(VkCommandBuffer cmd_buffer, VkQueue queue)
    {
        DODO_VERIFY_VK_RESULT(vkEndCommandBuffer(cmd_buffer));

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence = VK_NULL_HANDLE;
        DODO_VERIFY_VK_RESULT(vkCreateFence(device_, &fence_info, nullptr, &fence));

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buffer;
        DODO_VERIFY_VK_RESULT(vkQueueSubmit(queue, 1, &submit_info, fence));

        constexpr auto default_timeout = std::numeric_limits<uint64_t>::max();
        DODO_VERIFY_VK_RESULT(vkWaitForFences(device_, 1, &fence, VK_TRUE, default_timeout));

        vkDestroyFence(device_, fence, nullptr);
        vkFreeCommandBuffers(device_, get_or_create_thread_local_cmd_pool(), 1, &cmd_buffer);
    }

    Ref<VulkanAdapter> VulkanDevice::adapter() const
    {
        return adapter_;
    }

    VkDevice VulkanDevice::get_vulkan_device() const
    {
        return device_;
    }

    VkQueue VulkanDevice::vulkan_graphics_queue() const
    {
        return graphics_queue_;
    }

    void VulkanDevice::initialize_queue_create_infos()
    {
        static constexpr auto default_queue_priority = 1.0f;
        auto& queue_create_info = queue_create_infos_.emplace_back();
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = adapter_->graphics_queue_index();
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &default_queue_priority;
    }

    void VulkanDevice::request_extension(const std::string& name, bool is_required)
    {
        DODO_ASSERT(
            !requested_extensions_.contains(name),
            "Vulkan device extension [{0}] already requested!",
            name);

        requested_extensions_.insert({ name, is_required });
    }

    void VulkanDevice::validate_requested_extensions()
    {
        for (const auto& [name, is_required] : requested_extensions_)
        {
            if (!adapter_->is_extension_supported(name))
            {
                if (is_required)
                {
                    DODO_ASSERT(false, "Required Vulkan device extension[{0}] not supported!", name);
                }
                else
                {
                    DODO_LOG_WARNING_TAG("Renderer", "Vulkan device extension [{0}] not supported!", name);
                    continue;
                }
            }

            enabled_extensions_.push_back(name.c_str());
        }
    }

    VkCommandPool VulkanDevice::get_or_create_thread_local_cmd_pool()
    {
        const auto this_thread_id = std::this_thread::get_id();
        auto found = thread_local_cmd_pools_.find(this_thread_id);
        if (found != thread_local_cmd_pools_.end())
        {
            return found->second;
        }

        VkCommandPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_create_info.queueFamilyIndex = adapter_->graphics_queue_index();
        VkCommandPool command_pool = VK_NULL_HANDLE;
        DODO_VERIFY_VK_RESULT(vkCreateCommandPool(device_, &pool_create_info, nullptr, &command_pool));
        thread_local_cmd_pools_.insert({ this_thread_id, command_pool });
        return command_pool;
    }

}