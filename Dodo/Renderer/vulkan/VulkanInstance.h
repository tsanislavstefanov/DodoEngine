#pragma once

namespace Dodo {

    class RenderWindow;

    class VulkanInstance : public RefCounted
    {
    public:
        VulkanInstance(const RenderWindow& targetWindow);
        ~VulkanInstance();

        inline uint32_t GetAPIVersion() const { return APIVersion; }
        inline VkInstance GetVulkanInstance() const { return m_Instance; }

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL ReportValidationMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData);

        bool IsDriverVersionSupported(uint32_t minimumSupportedVersion) const;
        void QuerySupportedExtensions();
        void AddExtensionRequest(const std::string& name, bool isRequired);
        void ValidateRequestedExtensions();
        void QuerySupportedLayers();
        bool FindValidationLayer() const;

        static constexpr uint32_t APIVersion = VK_API_VERSION_1_0;
        static constexpr const char* ValidationLayerName = "VK_LAYER_KHRONOS_validation";
        std::set<std::string> m_SupportedExtensions{};
        std::map<std::string, bool> m_RequestedExtensions{};
        std::vector<const char*> m_EnabledExtensions{};
        std::set<std::string> m_SupportedLayers{};
        bool m_ValidationLayerFound = false;
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT = nullptr;
    };

}