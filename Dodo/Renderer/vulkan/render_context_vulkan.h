#pragma once

#include "vulkan_utils.h"
#include "renderer/render_context.h"

namespace Dodo {

    class RenderContextVulkan : public RenderContext {
    public:
        struct SurfaceVulkan {
            VkSurfaceKHR surface = nullptr;
            uint32_t width = 0;
            uint32_t height = 0;
            VSyncMode vsync_mode = VSyncMode::none;
            bool needs_resize = false;
        };

        RenderContextVulkan();
        virtual ~RenderContextVulkan() override;

        SurfaceHandle surface_create(Display::WindowId window, const SurfaceSpecifications& surface_specs, const void* platform_data) override;
        void surface_set_size(SurfaceHandle surface, uint32_t width, uint32_t height) override;
        void surface_destroy(SurfaceHandle surface) override;
        Ref<Renderer> renderer_create(size_t device_type) const override;

        uint32_t get_minimum_supported_api_version() const;
        VkInstance get_instance() const;
        VkPhysicalDevice get_physical_device(size_t index) const;
        uint32_t get_queue_family_count(size_t device_index) const;
        const VkQueueFamilyProperties& get_queue_family(size_t device_index, size_t queue_index) const;

    protected:
        virtual const char* _get_platform_surface_extension() const = 0;

        std::map<Display::WindowId, SurfaceHandle> _surfaces{};

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL _report_validation_message(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

        bool _is_driver_version_supported(uint32_t minimum_supported_version) const;
        void _initialize_instance();
        void _request_extension(const std::string& name, bool is_required);
        void _query_adapters_and_queue_families();
        uint32_t _minimum_supported_api_version = VK_API_VERSION_1_0;
        std::map<std::string, bool> _requested_extensions{};
        std::vector<const char*> _enabled_extensions{};
        const char* _validation_layer_name = "VK_LAYER_KHRONOS_validation";
        bool _debug_utils_extension_enabled = false;
        VkInstance _instance = nullptr;
        VkDebugUtilsMessengerEXT _debug_messenger = nullptr;
        PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT = nullptr;
        std::vector<VkPhysicalDevice> _physical_devices{};
        std::vector<std::vector<VkQueueFamilyProperties>> _queue_families{};
    };

}