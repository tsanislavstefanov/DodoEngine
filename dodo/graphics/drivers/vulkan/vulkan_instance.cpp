#include "pch.h"
#include "vulkan.h"
#include "vulkan_instance.h"
#include "core/window.h"

namespace Dodo {

    namespace Utils {

        static constexpr const char* stringify(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity)
        {
            switch (message_severity)
            {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : { return "Verbose"; }
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : { return "Warning"; }
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   : { return "Error";   }
            }

            return "Unknown";
        }

        static constexpr const char* stringify(VkDebugUtilsMessageTypeFlagsEXT message_type)
        {
            switch (message_type)
            {
                case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     : { return "General";     }
                case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  : { return "Validation";  }
                case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : { return "Performance"; }
            }

            return "Unknown";
        }

    }

    VulkanInstance::VulkanInstance(const Window& target_window)
    {
        DODO_VERIFY(is_driver_version_supported(api_version));

        query_supported_extensions();
        add_extension_request(VK_KHR_SURFACE_EXTENSION_NAME, true);
        add_extension_request(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false);
        const auto required_extensions = target_window.get_required_vulkan_instance_extensions();
        for (const auto& extension : required_extensions)
            add_extension_request(extension, true);

        validate_requested_extensions();
        query_supported_layers();
        validation_layer_found_ = find_validation_layer();

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Dodo Engine";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Dodo";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = api_version;
        VkInstanceCreateInfo instance_create_info{};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;
        instance_create_info.enabledLayerCount = validation_layer_found_ ? 1 : 0;
        instance_create_info.ppEnabledLayerNames = validation_layer_found_ ? &validation_layer_name : nullptr;
        instance_create_info.enabledExtensionCount = (uint32_t)enabled_extensions_.size();
        instance_create_info.ppEnabledExtensionNames = enabled_extensions_.data();
        DODO_VERIFY_VK_RESULT(vkCreateInstance(&instance_create_info, nullptr, &instance_));

        if (validation_layer_found_)
            create_debug_messenger();
    }

    VulkanInstance::~VulkanInstance()
    {
        if (validation_layer_found_)
            debug_messenger_procedures_.destroy(instance_, debug_messenger_, nullptr);

        vkDestroyInstance(instance_, nullptr);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::report_validation_message(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            VkDebugUtilsMessageTypeFlagsEXT message_type,
            const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
            void *user_data)
    {
        DODO_LOG_TRACE_TAG("Renderer", "Vulkan validation report...");
        DODO_LOG_TRACE_TAG("Renderer", "    Severity: {0}.", Utils::stringify(message_severity));
        DODO_LOG_TRACE_TAG("Renderer", "    Type: {0}.", Utils::stringify(message_type));
        DODO_LOG_TRACE_TAG("Renderer", "    Message: {0}.", callback_data->pMessage);
        return VK_FALSE;
    }

    bool VulkanInstance::is_driver_version_supported(uint32_t minimum_supported_version) const
    {
        uint32_t driver_version = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceVersion(&driver_version));
        return driver_version >= minimum_supported_version;
    }

    void VulkanInstance::query_supported_extensions()
    {
        uint32_t extension_count = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
        std::vector<VkExtensionProperties> extensions(extension_count);
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()));
        for (const auto& extension : extensions)
            supported_extensions_.insert(extension.extensionName);
    }

    void VulkanInstance::add_extension_request(const std::string& name, bool is_required)
    {
        DODO_ASSERT(
            !requested_extensions_.contains(name),
            "Vulkan instance extension [{0}] already requested!",
            name);

        requested_extensions_.insert({ name, is_required });
    }

    void VulkanInstance::validate_requested_extensions()
    {
        for (const auto& [name, is_required] : requested_extensions_)
        {
            if (!supported_extensions_.contains(name))
            {
                if (is_required)
                {
                    DODO_ASSERT(false, "Required Vulkan instance extension[{0}] not supported!", name);
                }
                else
                {
                    DODO_LOG_WARNING_TAG("Renderer", "Vulkan instance extension [{0}] not supported!", name);
                    continue;
                }
            }

            enabled_extensions_.push_back(name.c_str());
        }
    }

    void VulkanInstance::query_supported_layers()
    {
        uint32_t layer_count = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
        std::vector<VkLayerProperties> layers(layer_count);
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceLayerProperties(&layer_count, layers.data()));
        for (const auto& layer : layers)
            supported_layers_.insert(layer.layerName);
    }

    bool VulkanInstance::find_validation_layer() const
    {
        return supported_extensions_.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
            && supported_layers_.contains(validation_layer_name);
    }

    void VulkanInstance::create_debug_messenger()
    {
        debug_messenger_procedures_.create =
            fetch_procedure<PFN_vkCreateDebugUtilsMessengerEXT>(
                "vkCreateDebugUtilsMessengerEXT");

        VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{};
        messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messenger_create_info.pfnUserCallback = VulkanInstance::report_validation_message;
        DODO_VERIFY_VK_RESULT(debug_messenger_procedures_.create(
            instance_, &messenger_create_info, nullptr, &debug_messenger_));

        debug_messenger_procedures_.destroy =
            fetch_procedure<PFN_vkDestroyDebugUtilsMessengerEXT>(
                "vkDestroyDebugUtilsMessengerEXT");
    }

}