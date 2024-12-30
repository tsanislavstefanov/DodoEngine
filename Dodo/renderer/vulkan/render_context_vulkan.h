#pragma once

#ifdef DODO_VULKAN

#include "vulkan_utils.h"
#include "renderer/render_context.h"

namespace Dodo {

    class RenderContextVulkan : public RenderContext {
    public:
        struct Functions {
            // Debug messenger.
            PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT = nullptr;
            PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = nullptr;

            // Surface.
            PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR = nullptr;
            PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
            PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
            PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR = nullptr;

            // Device.
            PFN_vkGetDeviceProcAddr GetDeviceProcAddr = nullptr;
        };

        struct SurfaceVulkan {
            VkSurfaceKHR surface = nullptr;
            uint32_t width = 0;
            uint32_t height = 0;
            VSyncMode vsync_mode = VSyncMode::none;
            bool needs_resize = false;
        };

        RenderContextVulkan() = default;
        virtual ~RenderContextVulkan() override;

        void initialize() override;
        void on_event(Display::Event& e) override;
        Type get_type() const;

        SurfaceHandle surface_create(Display::WindowId window_id, const SurfaceSpecifications& surface_specs, const void* platform_data) override;
        void surface_on_resize(SurfaceHandle surface_handle, uint32_t width, uint32_t height) override;
        void surface_destroy(SurfaceHandle surface_handle) override;

        Ref<Renderer> renderer_create();

        uint32_t adapter_get_count() const;
        const Adapter& adapter_get(size_t index) const;

        uint32_t supported_api_version_get() const;
        VkInstance instance_get() const;
        const Functions& functions_get() const;
        VkPhysicalDevice physical_device_get(size_t device_index) const;
        uint32_t queue_family_get_count(size_t device_index) const;
        const VkQueueFamilyProperties& queue_family_get(size_t device_index, size_t queue_index) const;
        bool queue_family_supports_present(VkPhysicalDevice physical_device, uint32_t queue_family_index, VkSurfaceKHR surface) const;

    protected:
        virtual const char* _get_platform_surface_extension() const = 0;

        std::map<Display::WindowId, SurfaceHandle> _surfaces = {};

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL _report_validation_message(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

        bool _is_driver_version_supported(uint32_t minimum_supported_version) const;
        void _initialize_instance();
        void _request_extension(const std::string& name, bool is_required);
        void _query_adapters_and_queue_families();

        uint32_t _desired_api_version = VK_API_VERSION_1_0;
        std::map<std::string, bool> _requested_extensions = {};
        std::vector<const char*> _enabled_extensions = {};
        const char* _validation_layer_name = "VK_LAYER_KHRONOS_validation";
        bool _debug_utils_extension_enabled = false;
        VkInstance _instance = nullptr;
        Functions _functions = {};
        VkDebugUtilsMessengerEXT _debug_messenger = nullptr;
        std::vector<VkPhysicalDevice> _physical_devices = {};
        std::vector<Adapter> _adapters = {};
        std::vector<std::vector<VkQueueFamilyProperties>> _queue_families = {};
    };

}

#endif
