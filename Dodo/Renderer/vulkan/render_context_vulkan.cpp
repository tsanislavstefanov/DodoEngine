#include "pch.h"
#include "render_context_vulkan.h"
#include "renderer_vulkan.h"

namespace Dodo {

    namespace Utils {

        static constexpr const char* to_string_message_severity(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity) {
            switch (message_severity) {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "Verbose";
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "Warning";
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT  : return "Error";
                default: break;
            }

            return "Unknown";
        }

        static constexpr const char* to_string_message_type(VkDebugUtilsMessageTypeFlagsEXT message_type) {
            switch (message_type) {
                case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    : return "General";
                case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : return "Validation";
                case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "Performance";
                default: break;
            }

            return "Unknown";
        }

        static constexpr RenderContext::Adapter::Type convert_to_adapter_type(VkPhysicalDeviceType type) {
            switch (type) {
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU  : return RenderContext::Adapter::Type::discrete;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return RenderContext::Adapter::Type::integrated;
                default: break;
            }

            return RenderContext::Adapter::Type::none;
        }

    }

    RenderContextVulkan::RenderContextVulkan() {
        DODO_LOG_INFO_TAG("Renderer", "Renderer backend: Vulkan!");
        DODO_VERIFY(_is_driver_version_supported(_minimum_supported_api_version));
        _initialize_instance();
        _query_adapters_and_queue_families();
    }

    RenderContextVulkan::~RenderContextVulkan() {
        if (_debug_utils_extension_enabled) {
            DODO_GET_VK_INSTANCE_PROC_ADDR(_instance, DestroyDebugUtilsMessengerEXT);
            pfnDestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
        }

        vkDestroyInstance(_instance, nullptr);
    }

    SurfaceHandle RenderContextVulkan::surface_create(Display::WindowId window, const SurfaceSpecifications& surface_specs, const void* platform_data) {
        DODO_ASSERT(false, "Surface can only be created by platform agnostic context!");
        return SurfaceHandle{};
    }

    Ref<Renderer> RenderContextVulkan::renderer_create(size_t device_type) const
    {
        return Ref<Renderer>();
    }

    void RenderContextVulkan::surface_set_size(SurfaceHandle surface, uint32_t width, uint32_t height) {
        SurfaceVulkan* surface_vk = reinterpret_cast<SurfaceVulkan*>(surface.handle);
        surface_vk->width  = width;
        surface_vk->height = height;
        surface_vk->needs_resize = true;
    }

    void RenderContextVulkan::surface_destroy(SurfaceHandle surface) {
        const auto found = std::ranges::find_if(_surfaces, [surface](const auto& item) -> bool {
            return item.second == surface;
        });

        if (found != _surfaces.end()) {
            _surfaces.erase(found);
            auto surface_vk = reinterpret_cast<SurfaceVulkan*>(surface.handle);
            vkDestroySurfaceKHR(_instance, surface_vk->surface, nullptr);
        }
    }

    Ref<Renderer> RenderContextVulkan::renderer_create(Adapter::Type adapter_type) const {
        return Ref<Renderer>::create(this, adapter_type);
    }

    uint32_t RenderContextVulkan::get_minimum_supported_api_version() const {
        return _minimum_supported_api_version;
    }

    VkInstance RenderContextVulkan::get_instance() const {
        return _instance;
    }

    VkPhysicalDevice RenderContextVulkan::get_physical_device(size_t index) const {
        return _physical_devices.at(index);
    }

    uint32_t RenderContextVulkan::get_queue_family_count(size_t device_index) const {
        return _queue_families.at(device_index).size();
    }

    const VkQueueFamilyProperties& RenderContextVulkan::get_queue_family(size_t device_index, size_t queue_index) const {
        return _queue_families.at(device_index).at(queue_index);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL RenderContextVulkan::_report_validation_message(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
        DODO_LOG_TRACE_TAG("Renderer", "Vulkan validation report...");
        DODO_LOG_TRACE_TAG("Renderer", "    Severity: {0}.", Utils::to_string_message_severity(message_severity));
        DODO_LOG_TRACE_TAG("Renderer", "    Type: {0}.", Utils::to_string_message_type(message_type));
        DODO_LOG_TRACE_TAG("Renderer", "    Message: {0}.", callback_data->pMessage);
        return VK_FALSE;
    }

    bool RenderContextVulkan::_is_driver_version_supported(uint32_t minimum_supported_version) const {
        uint32_t driver_version = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceVersion(&driver_version));
        if (driver_version < minimum_supported_version) {
            DODO_LOG_ERROR_TAG("Renderer", "Vulkan driver version is out of date...");
            DODO_LOG_ERROR_TAG("Renderer", "    Minimum required version: {0}.", Utils::to_string_vulkan_version(minimum_supported_version));
            DODO_LOG_ERROR_TAG("Renderer", "    Installed version: {0}.", Utils::to_string_vulkan_version(driver_version));
            return false;
        }

        return true;
    }

    void RenderContextVulkan::_initialize_instance() {
        std::set<std::string> supported_extensions{};
        uint32_t extension_count = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
        std::vector<VkExtensionProperties> extensions(extension_count);
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()));
        for (const auto& extension : extensions) {
            supported_extensions.insert(extension.extensionName);
        }

        _request_extension(VK_KHR_SURFACE_EXTENSION_NAME, true);
        _request_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false);
        _request_extension(_get_platform_surface_extension(), true);

        // Validate requested extensions.
        for (const auto& [name, is_required] : _requested_extensions) {
            if (!supported_extensions.contains(name)) {
                if (is_required) {
                    DODO_ASSERT(false, "Required instance extension[{0}] not supported!", name);
                }
                else {
                    DODO_LOG_WARNING_TAG("Renderer", "Instance extension [{0}] not supported!", name);
                    continue;
                }
            }

            _enabled_extensions.push_back(name.c_str());
        }

        std::set<std::string> supported_layers{};
        uint32_t layer_count = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
        std::vector<VkLayerProperties> layers(layer_count);
        DODO_VERIFY_VK_RESULT(vkEnumerateInstanceLayerProperties(&layer_count, layers.data()));
        for (const auto& layer : layers) {
            supported_layers.insert(layer.layerName);
        }

        _debug_utils_extension_enabled = supported_extensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) &&
            supported_layers.contains(_validation_layer_name);

        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Dodo Engine";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Dodo";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = _minimum_supported_api_version;
        VkInstanceCreateInfo instance_create_info{};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;
        instance_create_info.enabledLayerCount = _debug_utils_extension_enabled ? 1 : 0;
        instance_create_info.ppEnabledLayerNames = _debug_utils_extension_enabled ? &_validation_layer_name : nullptr;
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(_enabled_extensions.size());
        instance_create_info.ppEnabledExtensionNames = _enabled_extensions.data();
        DODO_VERIFY_VK_RESULT(vkCreateInstance(&instance_create_info, nullptr, &_instance));

        if (_debug_utils_extension_enabled) {
            DODO_GET_VK_INSTANCE_PROC_ADDR(_instance, CreateDebugUtilsMessengerEXT);
            VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{};
            messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            messenger_create_info.pfnUserCallback = RenderContextVulkan::_report_validation_message;
            DODO_VERIFY_VK_RESULT(pfnCreateDebugUtilsMessengerEXT(_instance, &messenger_create_info, nullptr, &_debug_messenger));
        }
    }

    void RenderContextVulkan::_request_extension(const std::string& name, bool is_required) {
        DODO_ASSERT(!_requested_extensions.contains(name), "Vulkan instance extension [{0}] already requested!", name);
        _requested_extensions.emplace(name, is_required);
    }

    void RenderContextVulkan::_query_adapters_and_queue_families() {
        _adapters.clear();
        _physical_devices.clear();
        _queue_families.clear();

        uint32_t physical_device_count = 0;
        DODO_VERIFY_VK_RESULT(vkEnumeratePhysicalDevices(_instance, &physical_device_count, nullptr));
        DODO_ASSERT(physical_device_count != 0, "GPU that support Vulkan not found!");
        _physical_devices.resize(physical_device_count);
        DODO_VERIFY_VK_RESULT(vkEnumeratePhysicalDevices(_instance, &physical_device_count, _physical_devices.data()));

        _adapters.resize(physical_device_count);
        _queue_families.resize(physical_device_count);
        for (const auto& physical_device : _physical_devices) {
            // Hard to find any information on Vulkan vendors...
            // Online: https://www.reddit.com/r/vulkan/comments/4ta9nj/is_there_a_comprehensive_list_of_the_names_and/.
            static std::map<uint32_t, Adapter::Vendor> vendors = {
                { 0x1002, Adapter::Vendor::amd },
                { 0x1010, Adapter::Vendor::img_tec },
                { 0x10DE, Adapter::Vendor::nvidia },
                { 0x13B5, Adapter::Vendor::arm },
                { 0x5143, Adapter::Vendor::qualcomm },
                { 0x8086, Adapter::Vendor::intel }
            };

            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physical_device, &properties);
            Adapter adapter{};
            adapter.name = properties.deviceName;
            adapter.type = Utils::convert_to_adapter_type(properties.deviceType);
            adapter.vendor = vendors.contains(properties.vendorID) ? vendors.at(properties.vendorID) : Adapter::Vendor::unknown;
            _adapters.push_back(std::move(adapter));

            uint32_t queue_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_family(queue_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queue_family.data());
            _queue_families.push_back(std::move(queue_family));
        }
    }

    SurfaceHandle RenderContextVulkan::surface_create(Display::WindowId window, const SurfaceSpecifications& surface_specs, const void* platform_data)
    {
        return SurfaceHandle();
    }

}