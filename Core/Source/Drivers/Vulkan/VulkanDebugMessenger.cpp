#include "pch.h"
#include "VulkanDebugMessenger.h"
#include "VulkanCommon.h"

namespace Dodo {
    
    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static const char* ConvertMessageSeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity)
        {
            switch (messageSeverity)
            {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : return "Verbose";
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    : return "Info";
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : return "Warning";
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   : return "Error";
                default                                              : return "Unknown";
            }

            return "";
        }

        static const char* ConvertMessageTypeToString(VkDebugUtilsMessageTypeFlagsEXT messageType)
        {
            switch (messageType)
            {
                case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     : return "General";
                case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  : return "Validation";
                case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : return "Performance";
                default                                              : return "Unknown";
            }

            return "";
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL ReportDebugMessage(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                                 void* userData)
        {
            (void)userData; // Unused.
            LOG_CORE_DEBUG("Vulkan validation report...");
            LOG_CORE_DEBUG("    Severity: {0}.", ConvertMessageSeverityToString(messageSeverity));
            LOG_CORE_DEBUG("    Type: {0}."    , ConvertMessageTypeToString(messageType));
            LOG_CORE_DEBUG("    Message: {0}." , callbackData->pMessage);
            return VK_FALSE;
        }

    }

    ////////////////////////////////////////////////////////////////
    // VULKAN DEBUG MESSENGER DATA /////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static PFN_vkCreateDebugUtilsMessengerEXT  pfnCreateDebugUtilsMessengerEXT  = nullptr;
    static PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT = nullptr;

    ////////////////////////////////////////////////////////////////
    // VULKAN DEBUG MESSENGER //////////////////////////////////////
    ////////////////////////////////////////////////////////////////

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

}