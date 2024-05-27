#include "pch.h"
#include "VulkanContext.h"

#ifdef DODO_WINDOWS
#   include <vulkan/vulkan_win32.h>
#endif

#include "Vulkan.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static const char* ConvertMessageSeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity)
        {
            switch (messageSeverity)
            {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "Verbose";
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    return "Info";
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "Warning";
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   return "Error";
                default:                                              return "Unknown";
            }
        }

        static const char* ConvertMessageTypeToString(VkDebugUtilsMessageTypeFlagsEXT messageType)
        {
            switch (messageType)
            {
                case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:     return "General";
                case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:  return "Validation";
                case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "Performance";
                default:                                              return "Unknown";
            }
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL ReportDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                                 void* userData)
        {
            (void)userData;
            LOG_CORE_DEBUG("Vulkan validation report...");
            LOG_CORE_DEBUG("    Severity: {0}.", Utils::ConvertMessageSeverityToString(messageSeverity));
            LOG_CORE_DEBUG("    Type: {0}."    , Utils::ConvertMessageTypeToString(messageType));
            LOG_CORE_DEBUG("    Message: {0}." , callbackData->pMessage);
            return VK_FALSE;
        }

        static bool CheckDriverVersionSupport(uint32_t minimumSupportedVersion)
        {
            uint32_t driverVersion = 0;
            DODO_VK_RESULT(vkEnumerateInstanceVersion(&driverVersion));
            if (driverVersion < minimumSupportedVersion)
            {
                LOG_CORE_FATAL("Vulkan driver version is out of date...");
                LOG_CORE_FATAL("    Installed driver version: {0}.{1}.{2}.", VK_API_VERSION_MAJOR(driverVersion), VK_API_VERSION_MINOR(driverVersion), VK_API_VERSION_PATCH(driverVersion));
                LOG_CORE_FATAL("    Minimum supported driver version: {0}.{1}.{2}.", VK_API_VERSION_MAJOR(minimumSupportedVersion), VK_API_VERSION_MINOR(minimumSupportedVersion), VK_API_VERSION_PATCH(minimumSupportedVersion));
                return false;
            }

            return true;
        }

        static std::string GetPlatformSurfaceExtension()
        {
#ifdef DODO_WINDOWS
            return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif
            DODO_ASSERT(false, "Platform not supported!");
        }

    }

    ////////////////////////////////////////////////////////////////
    // VULKAN DEBUG MESSENGER //////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static PFN_vkCreateDebugUtilsMessengerEXT  pfnCreateDebugUtilsMessengerEXT  = nullptr;
    static PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT = nullptr;

    VulkanDebugMessenger::VulkanDebugMessenger(VkInstance instance)
        : m_Instance(instance)
    {
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, CreateDebugUtilsMessengerEXT );
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, DestroyDebugUtilsMessengerEXT);
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = Utils::ReportDebugMessage;
        DODO_VK_RESULT(pfnCreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_Messenger));
    }

    void VulkanDebugMessenger::Destroy()
    {
        pfnDestroyDebugUtilsMessengerEXT(m_Instance, m_Messenger, nullptr);
    }

    ////////////////////////////////////////////////////////////////
    // VULKAN CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    VulkanContext* VulkanContext::s_Context = nullptr;

    VulkanContext::VulkanContext()
    {
        DODO_ASSERT(!s_Context, "Vulkan context already exists!");
        s_Context = this;

        DODO_VERIFY(Utils::CheckDriverVersionSupport(m_ApiVersion));

        // Get supported extensions.
        uint32_t extensionCount = 0;
        DODO_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        DODO_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()));
        for (const auto& extension : extensions)
        {
            m_SupportedExtensions.insert(extension.extensionName);
        }

        // Request extensions.
        std::map<std::string, bool> requestedExtensions{};
        requestedExtensions.insert({ VK_KHR_SURFACE_EXTENSION_NAME, true });
        requestedExtensions.insert({ Utils::GetPlatformSurfaceExtension(), true });
        requestedExtensions.insert({ VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false });

        // Enable requested & supported extensions.
        for (const auto& [name, required] : requestedExtensions)
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

        // Find validation layer(s).
        if (m_EnabledExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
        {
            uint32_t layerCount = 0;
            DODO_VK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
            std::vector<VkLayerProperties> layers(layerCount);
            DODO_VK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()));
            for (const auto& layer : layers)
            {
                if (std::strcmp(m_LayerName, layer.layerName) != 0)
                {
                    m_LayerFound = true;
                    break;
                }
            }
        }

        // Prepare enabled extensions for Vulkan.
        std::vector<const char*> enabledExtensions{};
        for (const auto& extension : m_EnabledExtensions)
        {
            enabledExtensions.push_back(extension.c_str());
        }

        // Create instance.
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Dodo Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Dodo";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = m_ApiVersion;
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = m_LayerFound ? 1 : 0;
        createInfo.ppEnabledLayerNames = m_LayerFound ? &m_LayerName : nullptr;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        DODO_VK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_Instance));

        // Create debug messenger.
        if (m_LayerFound)
        {
            m_DebugMessenger = Ref<VulkanDebugMessenger>::Create(m_Instance);
        }

        // Create logical device.
        m_Device = Ref<VulkanDevice>::Create(m_Instance);
    }

    void VulkanContext::Destroy()
    {
        m_Device->Destroy();
        if (m_LayerFound)
        {
            m_DebugMessenger->Destroy();
        }

        vkDestroyInstance(m_Instance, nullptr);
    }

}