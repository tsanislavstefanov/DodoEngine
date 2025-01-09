#include "pch.h"

#ifdef DODO_VULKAN

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

            return RenderContext::Adapter::Type::unknown;
        }

    }

    RenderContextVulkan::~RenderContextVulkan() {
        if (_debug_utils_extension_enabled) {
            _functions.DestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, nullptr);
        }

        vkDestroyInstance(_instance, nullptr);
    }

    void RenderContextVulkan::initialize() {
        DODO_ASSERT(_is_driver_version_supported(_desired_api_version));
        _initialize_instance();
        _query_adapters_and_queue_families();
    }

    void RenderContextVulkan::on_event(Display::Event& e) {
        switch (e.type) {
            case Display::Event::Type::window_resize: {
                surface_on_resize(_surfaces.at(e.window_id), e.resized.width, e.resized.height);
                break;
            }

            default:
                break;
        }
    }

    RenderContext::Type RenderContextVulkan::get_type() const {
        return Type::vulkan;
    }

    SurfaceHandle RenderContextVulkan::surface_create(Display::WindowId window_id, const SurfaceSpecifications& surface_specs, const void* platform_data) {
        DODO_ASSERT(false);
        return SurfaceHandle(nullptr);
    }

    void RenderContextVulkan::surface_on_resize(SurfaceHandle surface_handle, uint32_t width, uint32_t height) {
        Surface* surface = reinterpret_cast<Surface*>(surface_handle.handle);
        surface->width = width;
        surface->height = height;
        surface->needs_resize = true;
    }

    void RenderContextVulkan::surface_destroy(SurfaceHandle surface_handle) {
        const auto found = std::ranges::find_if(_surfaces, [surface_handle](const auto& item) -> bool {
            return item.second == surface_handle;
        });

        if (found != _surfaces.end()) {
            _surfaces.erase(found);
            auto surface = reinterpret_cast<Surface*>(surface_handle.handle);
            vkDestroySurfaceKHR(_instance, surface->surface_vk, nullptr);
            delete surface;
        }
    }

    Ref<Renderer> RenderContextVulkan::renderer_create() {
        return Ref<RendererVulkan>::create(this);
    }

    uint32_t RenderContextVulkan::adapter_get_count() const {
        return static_cast<uint32_t>(_adapters.size());
    }

    const RenderContext::Adapter& RenderContextVulkan::adapter_get(size_t index) const {
        return _adapters.at(index);
    }

    uint32_t RenderContextVulkan::supported_api_version_get() const {
        return _desired_api_version;
    }

    VkInstance RenderContextVulkan::instance_get() const {
        return _instance;
    }

    const RenderContextVulkan::Functions& RenderContextVulkan::functions_get() const {
        return _functions;
    }

    VkPhysicalDevice RenderContextVulkan::physical_device_get(size_t device_index) const {
        return _physical_devices.at(device_index);
    }

    uint32_t RenderContextVulkan::queue_family_get_count(size_t device_index) const {
        return static_cast<uint32_t>(_queue_families.at(device_index).size());
    }

    const VkQueueFamilyProperties& RenderContextVulkan::queue_family_get(size_t device_index, size_t queue_index) const {
        return _queue_families.at(device_index).at(queue_index);
    }

    bool RenderContextVulkan::queue_family_supports_present(VkPhysicalDevice physical_device, uint32_t queue_family_index, SurfaceHandle surface_handle) const {
        DODO_ASSERT(surface_handle);
        auto surface = reinterpret_cast<RenderContextVulkan::Surface*>(surface_handle.handle);
        VkBool32 supports_present = false;
        DODO_ASSERT_VK_RESULT(_functions.GetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface->surface_vk, &supports_present));
        return supports_present;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL RenderContextVulkan::_report_validation_message(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
        DODO_LOG_TRACE_TAG("Renderer", "Vulkan validation report...");
        DODO_LOG_TRACE_TAG("Renderer", "    Severity: {0}.", Utils::to_string_message_severity(message_severity));
        DODO_LOG_TRACE_TAG("Renderer", "    Type: {0}.", Utils::to_string_message_type(message_type));
        DODO_LOG_TRACE_TAG("Renderer", "    Message: {0}.", callback_data->pMessage);
        return VK_FALSE;
    }

    bool RenderContextVulkan::_is_driver_version_supported(uint32_t desired_api_version) const {
        uint32_t driver_version = 0;
        DODO_ASSERT_VK_RESULT(vkEnumerateInstanceVersion(&driver_version));
        if (driver_version < desired_api_version) {
            DODO_LOG_ERROR_TAG("Renderer", "Vulkan driver version is out of date...");
            DODO_LOG_ERROR_TAG("Renderer", "    Minimum required version: {0}.", Utils::to_string_vulkan_version(desired_api_version));
            DODO_LOG_ERROR_TAG("Renderer", "    Installed version: {0}.", Utils::to_string_vulkan_version(driver_version));
            return false;
        }

        return true;
    }

    void RenderContextVulkan::_initialize_instance() {
        std::set<std::string> supported_extensions = {};
        uint32_t extension_count = 0;
        DODO_ASSERT_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
        std::vector<VkExtensionProperties> extensions(extension_count);
        DODO_ASSERT_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data()));
        for (const auto& extension : extensions) {
            supported_extensions.insert(extension.extensionName);
        }

        _request_extension(VK_KHR_SURFACE_EXTENSION_NAME, true);
        _request_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false);
        _request_extension(_get_platform_surface_extension(), true);

        for (const auto& [name, is_required] : _requested_extensions) {
            if (!supported_extensions.contains(name)) {
                if (is_required) {
                    DODO_LOG_ERROR_TAG("Renderer", "{0} required but not supported!", name);
                    DODO_ASSERT(false);
                }
                else {
                    DODO_LOG_WARNING_TAG("Renderer", "{0} not supported!", name);
                    continue;
                }
            }

            _enabled_extensions.push_back(name.c_str());
        }

        std::set<std::string> supported_layers = {};
        uint32_t layer_count = 0;
        DODO_ASSERT_VK_RESULT(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
        std::vector<VkLayerProperties> layers(layer_count);
        DODO_ASSERT_VK_RESULT(vkEnumerateInstanceLayerProperties(&layer_count, layers.data()));
        for (const auto& layer : layers) {
            supported_layers.insert(layer.layerName);
        }

        if (supported_extensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            _debug_utils_extension_enabled = supported_layers.contains(_validation_layer_name);
        }

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Dodo Engine";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Dodo";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = _desired_api_version;
        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &app_info;
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(_debug_utils_extension_enabled ? 1 : 0);
        instance_create_info.ppEnabledLayerNames = _debug_utils_extension_enabled ? &_validation_layer_name : nullptr;
        instance_create_info.enabledExtensionCount = static_cast<uint32_t>(_enabled_extensions.size());
        instance_create_info.ppEnabledExtensionNames = _enabled_extensions.data();
        DODO_ASSERT_VK_RESULT(vkCreateInstance(&instance_create_info, nullptr, &_instance));

        _functions.CreateDebugUtilsMessengerEXT  = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT >(vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT" ));
        _functions.DestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
        _functions.GetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(_instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
        _functions.GetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(_instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
        _functions.GetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(_instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
        _functions.GetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(_instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
        _functions.GetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(vkGetInstanceProcAddr(_instance, "vkGetDeviceProcAddr"));

        if (_debug_utils_extension_enabled) {
            VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = {};
            messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            messenger_create_info.pfnUserCallback = RenderContextVulkan::_report_validation_message;
            DODO_ASSERT_VK_RESULT(_functions.CreateDebugUtilsMessengerEXT(_instance, &messenger_create_info, nullptr, &_debug_messenger));
        }
    }

    void RenderContextVulkan::_request_extension(const std::string& name, bool is_required) {
        DODO_ASSERT(!_requested_extensions.contains(name));
        _requested_extensions.emplace(name, is_required);
    }

    void RenderContextVulkan::_query_adapters_and_queue_families() {
        _physical_devices.clear();
        _adapters.clear();
        _queue_families.clear();

        uint32_t physical_device_count = 0;
        DODO_ASSERT_VK_RESULT(vkEnumeratePhysicalDevices(_instance, &physical_device_count, nullptr));
        DODO_ASSERT(physical_device_count != 0);
        _physical_devices.resize(physical_device_count);
        DODO_ASSERT_VK_RESULT(vkEnumeratePhysicalDevices(_instance, &physical_device_count, _physical_devices.data()));

        for (size_t i = 0; i < _physical_devices.size(); i++) {
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

            VkPhysicalDeviceProperties properties = {};
            vkGetPhysicalDeviceProperties(_physical_devices.at(i), &properties);

            Adapter adapter = {};
            adapter.name = properties.deviceName;
            adapter.type = Utils::convert_to_adapter_type(properties.deviceType);
            adapter.vendor = vendors.contains(properties.vendorID) ? vendors.at(properties.vendorID) : Adapter::Vendor::unknown;
            _adapters.push_back(std::move(adapter));

            uint32_t queue_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(_physical_devices.at(i), &queue_count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_family(queue_count);
            vkGetPhysicalDeviceQueueFamilyProperties(_physical_devices.at(i), &queue_count, queue_family.data());
            _queue_families.push_back(std::move(queue_family));
        }
    }

}

#endif