#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanInstance.h"
#include "Core/Window.h"

namespace Dodo {

    namespace Utils {

        static constexpr const char* ConvertMessageSeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity)
        {
            switch (messageSeverity)
            {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : { return "Verbose"; }
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : { return "Warning"; }
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   : { return "Error";   }
            }

            return "Unknown";
        }

        static constexpr const char* ConvertMessageTypeToString(VkDebugUtilsMessageTypeFlagsEXT messageType)
        {
            switch (messageType)
            {
                case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     : { return "General";     }
                case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  : { return "Validation";  }
                case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : { return "Performance"; }
            }

            return "Unknown";
        }

    }

    VulkanInstance::VulkanInstance(const RenderWindow& targetWindow)
    {
        DODO_VERIFY(IsDriverVersionSupported(APIVersion));

        QuerySupportedExtensions();
        AddExtensionRequest(VK_KHR_SURFACE_EXTENSION_NAME, true);
        AddExtensionRequest(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false);
        for (const std::string& extension : targetWindow.GetRequiredVulkanInstanceExtensions())
            AddExtensionRequest(extension, true);
        ValidateRequestedExtensions();

        QuerySupportedLayers();
        m_ValidationLayerFound = FindValidationLayer();

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Dodo Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Dodo";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = APIVersion;
        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledLayerCount = m_ValidationLayerFound ? 1 : 0;
        instanceCreateInfo.ppEnabledLayerNames = m_ValidationLayerFound ? &ValidationLayerName : nullptr;
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_EnabledExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = m_EnabledExtensions.data();
        DODO_VERIFY_VK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance));

        if (m_ValidationLayerFound)
        {
            DODO_GET_VK_INSTANCE_PROC_ADDR(m_Instance, CreateDebugUtilsMessengerEXT);
            VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
            messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            messengerCreateInfo.pfnUserCallback = VulkanInstance::ReportValidationMessage;
            DODO_VERIFY_VK_RESULT(pfnCreateDebugUtilsMessengerEXT(m_Instance, &messengerCreateInfo, nullptr, &m_DebugMessenger));
        }
    }

    VulkanInstance::~VulkanInstance()
    {
        if (m_ValidationLayerFound)
        {
            DODO_GET_VK_INSTANCE_PROC_ADDR(m_Instance, DestroyDebugUtilsMessengerEXT);
            pfnDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_Instance, nullptr);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::ReportValidationMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData)
    {
        DODO_LOG_TRACE_TAG("Renderer", "Vulkan validation report...");
        DODO_LOG_TRACE_TAG("Renderer", "    Severity: {0}.", Utils::ConvertMessageSeverityToString(messageSeverity));
        DODO_LOG_TRACE_TAG("Renderer", "    Type: {0}.", Utils::ConvertMessageTypeToString(messageType));
        DODO_LOG_TRACE_TAG("Renderer", "    Message: {0}.", callbackData->pMessage);
        return VK_FALSE;
    }

    bool VulkanInstance::IsDriverVersionSupported(uint32_t minimumSupportedVersion) const
    {
        uint32_t driverVersion = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceVersion(&driverVersion));
        if (driverVersion < minimumSupportedVersion)
        {
            DODO_LOG_ERROR_TAG("Renderer", "Vulkan driver version is out of date...");
            DODO_LOG_ERROR_TAG("Renderer", "    Minimum required version: {0}.", Utils::ConvertVulkanVersionToString(minimumSupportedVersion));
            DODO_LOG_ERROR_TAG("Renderer", "    Installed version: {0}.", Utils::ConvertVulkanVersionToString(driverVersion));
            return false;
        }

        return true;
    }

    void VulkanInstance::QuerySupportedExtensions()
    {
        uint32_t extensionCount = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()));
        for (const VkExtensionProperties& extension : extensions)
            m_SupportedExtensions.insert(extension.extensionName);
    }

    void VulkanInstance::AddExtensionRequest(const std::string& name, bool isRequired)
    {
        DODO_ASSERT(!m_RequestedExtensions.contains(name), "Vulkan instance extension [{0}] already requested!", name);
        m_RequestedExtensions.insert({ name, isRequired });
    }

    void VulkanInstance::ValidateRequestedExtensions()
    {
        for (const auto& [name, isRequired] : m_RequestedExtensions)
        {
            if (!m_SupportedExtensions.contains(name))
            {
                if (isRequired)
                {
                    DODO_ASSERT(false, "Required Vulkan instance extension[{0}] not supported!", name);
                }
                else
                {
                    DODO_LOG_WARNING_TAG("Renderer", "Vulkan instance extension [{0}] not supported!", name);
                    continue;
                }
            }

            m_EnabledExtensions.push_back(name.c_str());
        }
    }

    void VulkanInstance::QuerySupportedLayers()
    {
        uint32_t layerCount = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
        std::vector<VkLayerProperties> layers(layerCount);
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()));
        for (const VkLayerProperties& layer : layers)
            m_SupportedLayers.insert(layer.layerName);
    }

    bool VulkanInstance::FindValidationLayer() const
    {
        return m_SupportedExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) && m_SupportedLayers.contains(ValidationLayerName);
    }

}