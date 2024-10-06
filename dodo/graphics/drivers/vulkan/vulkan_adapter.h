#pragma once

#include "vulkan_instance.h"

namespace Dodo {

    class VulkanAdapter : public RefCounted
    {
    public:
        VulkanAdapter(Ref<VulkanInstance> instance);

        bool is_extension_supported(const std::string& name) const;
        std::string vendor_name() const;
        VkPhysicalDevice get_vulkan_physical_device() const;
        const VkPhysicalDeviceProperties& vulkan_physical_device_properties() const;
        const std::vector<VkQueueFamilyProperties>& supported_vulkan_queue_families() const;
        uint32_t graphics_queue_index() const;

    private:
        void query_available_gpus();
        VkPhysicalDevice select_suitable_gpu() const;
        void query_supported_queue_families();
        std::optional<uint32_t> find_queue_family_index(VkQueueFlags family) const;
        void query_supported_extensions();

        Ref<VulkanInstance> instance_ = nullptr;
        std::vector<VkPhysicalDevice> available_gpus_{};
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties properties_{};
        std::vector<VkQueueFamilyProperties> m_SupportedQueueFamilies{};
        std::optional<uint32_t> graphics_queue_index_{};
        std::set<std::string> m_SupportedExtensions{};
    };

}