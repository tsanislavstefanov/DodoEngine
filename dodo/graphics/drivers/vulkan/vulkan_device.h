#pragma once

#include "vulkan_adapter.h"
#include "vulkan_instance.h"

namespace Dodo {

    class VulkanDevice : public RefCounted
    {
    public:
        VulkanDevice(Ref<VulkanInstance> instance, Ref<VulkanAdapter> adapter);
        ~VulkanDevice();

        template<typename Procedure>
        Procedure load_procedure(const std::string& name) const
        {
            auto procedure = vkGetDeviceProcAddr(device_, name.c_str());
            DODO_ASSERT(procedure, "Vulkan device procedure [{0}] not found!", name);
            return reinterpret_cast<Procedure>(procedure);
        }

        VkCommandBuffer allocate_thread_local_cmd_buffer(bool begin_immediately = true);
        void flush_cmd_buffer(VkCommandBuffer cmd_buffer);
        void flush_cmd_buffer(VkCommandBuffer cmd_buffer, VkQueue queue);
        Ref<VulkanAdapter> adapter() const;
        VkDevice get_vulkan_device() const;
        VkQueue vulkan_graphics_queue() const;
    private:
        void initialize_queue_create_infos();
        void request_extension(const std::string& name, bool is_required);
        void validate_requested_extensions();
        VkCommandPool get_or_create_thread_local_cmd_pool();

        Ref<VulkanInstance> instance_ = nullptr;
        Ref<VulkanAdapter> adapter_ = nullptr;
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos_{};
        std::map<std::string, bool> requested_extensions_{};
        std::vector<const char*> enabled_extensions_{};
        VkDevice device_ = VK_NULL_HANDLE;
        VkQueue graphics_queue_ = VK_NULL_HANDLE;
        std::map<std::thread::id, VkCommandPool> thread_local_cmd_pools_{};
    };

}