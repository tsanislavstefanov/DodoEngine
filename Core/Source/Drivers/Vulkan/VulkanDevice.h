#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // QUEUE FAMILY INDICES ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> Graphics{};
    };

    ////////////////////////////////////////////////////////////////
    // VULKAN PHYSICAL DEVICE //////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanPhysicalDevice : public RefCounted
    {
    public:
        VulkanPhysicalDevice(VkInstance instance);

        VkPhysicalDevice GetNativePhysicalDevice() const { return m_PhysicalDevice; }
        std::string GetVendorName () const { return m_VendorName; }
        const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }
        const std::vector<VkDeviceQueueCreateInfo>& GetQueueCreateInfos() const { return m_QueueCreateInfos; }

        bool IsExtensionSupported(const std::string& extensionName) const;

    private:
        QueueFamilyIndices FindQueueFamilyIndices() const;

        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_Properties{};
        std::set<std::string> m_SupportedExtensions{};
        const char* m_VendorName = "Unknown";
        QueueFamilyIndices m_QueueFamilyIndices{};
        std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos{};
    };

    ////////////////////////////////////////////////////////////////
    // VULKAN DEVICE ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanDevice : public RefCounted
    {
    public:
        VulkanDevice(VkInstance instance);

        Ref<VulkanPhysicalDevice> GetPhysicalDevice() const { return m_PhysicalDevice; }
        VkDevice GetNativeDevice () const { return m_Device; }
        VkQueue  GetGraphicsQueue() const { return m_GraphicsQueue; }

        void Destroy();

    private:
        Ref<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;
        std::set<std::string> m_EnabledExtensions{};
        VkDevice m_Device = VK_NULL_HANDLE;
        VkQueue  m_GraphicsQueue = VK_NULL_HANDLE;
    };

}