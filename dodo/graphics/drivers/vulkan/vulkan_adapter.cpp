#include "pch.h"
#include "vulkan.h"
#include "vulkan_adapter.h"

namespace Dodo {

    VulkanAdapter::VulkanAdapter(Ref<VulkanInstance> instance)
        : instance_(instance)
    {
        query_available_gpus();
        m_PhysicalDevice = select_suitable_gpu();
        DODO_ASSERT(m_PhysicalDevice, "No suitable GPU found!");
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties_);

        query_supported_queue_families();
        graphics_queue_index_ = find_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
        DODO_ASSERT(graphics_queue_index_.has_value(), "Graphics queue not found!");

        query_supported_extensions();
    }

    bool VulkanAdapter::is_extension_supported(const std::string& name) const
    {
        return m_SupportedExtensions.contains(name);
    }

    std::string VulkanAdapter::vendor_name() const
    {
        static std::map<uint32_t, std::string> vendor_table = {
            { 0x1002, "AMD"      },
            { 0x1010, "ImgTec"   },
            { 0x10DE, "NVIDIA"   },
            { 0x13B5, "ARM"      },
            { 0x5143, "Qualcomm" },
            { 0x8086, "Intel"    }
        };

        const auto vendor = vendor_table.find(properties_.vendorID);
        return (vendor != vendor_table.end())
            ? vendor_table.at(properties_.vendorID)
            : "Unknown";
    }

    VkPhysicalDevice VulkanAdapter::get_vulkan_physical_device() const
    {
        return m_PhysicalDevice;
    }

    const VkPhysicalDeviceProperties& VulkanAdapter::vulkan_physical_device_properties() const
    {
        return properties_;
    }

    const std::vector<VkQueueFamilyProperties>& VulkanAdapter::supported_vulkan_queue_families() const
    {
        return m_SupportedQueueFamilies;
    }

    uint32_t VulkanAdapter::graphics_queue_index() const
    {
        return graphics_queue_index_.value();
    }

    void VulkanAdapter::query_available_gpus()
    {
        uint32_t gpu_count = 0;
        DODO_VERIFY_VK_RESULT(vkEnumeratePhysicalDevices(
            instance_->get_vulkan_instance(), &gpu_count, nullptr));

        DODO_ASSERT(gpu_count != 0, "GPU that support Vulkan not found!");
        available_gpus_.resize(gpu_count);
        DODO_VERIFY_VK_RESULT(vkEnumeratePhysicalDevices(
            instance_->get_vulkan_instance(), &gpu_count, available_gpus_.data()));
    }

    VkPhysicalDevice VulkanAdapter::select_suitable_gpu() const
    {
        for (auto candidate : available_gpus_)
        {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(candidate, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                return candidate;
        }

        DODO_LOG_WARNING_TAG("Renderer", "No discrete GPU found!");
        return available_gpus_.back();
    }

    void VulkanAdapter::query_supported_queue_families()
    {
        uint32_t queue_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_count, nullptr);
        m_SupportedQueueFamilies.resize(queue_count);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_count, m_SupportedQueueFamilies.data());
    }

    std::optional<uint32_t> VulkanAdapter::find_queue_family_index(VkQueueFlags queueFamily) const
    {
        uint32_t queueIndex = 0;
        for (const auto& queue : m_SupportedQueueFamilies)
        {
            if (queue.queueFlags & queueFamily)
                return queueIndex;

            queueIndex++;
        }

        return {};
    }

    void VulkanAdapter::query_supported_extensions()
    {
        uint32_t extensionCount = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        DODO_VERIFY_VK_RESULT(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, extensions.data()));
        for (const auto& extension : extensions)
            m_SupportedExtensions.insert(extension.extensionName);
    }

}