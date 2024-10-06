#pragma once

namespace Dodo {

    class Window;

    class VulkanInstance : public RefCounted
    {
    public:
        VulkanInstance(const Window& target_window);
        ~VulkanInstance();

        template<typename Procedure>
        Procedure fetch_procedure(const std::string& name) const
        {
            auto procedure = vkGetInstanceProcAddr(instance_, name.c_str());
            DODO_ASSERT(procedure, "Vulkan instance procedure [{0}] not found!", name);
            return reinterpret_cast<Procedure>(procedure);
        }

        uint32_t get_api_version() const { return api_version; }
        VkInstance get_vulkan_instance() const { return instance_; }

    private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL report_validation_message(
                VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                VkDebugUtilsMessageTypeFlagsEXT message_type,
                const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                void* user_data);

        bool is_driver_version_supported(uint32_t minimum_supported_version) const;
        void query_supported_extensions();
        void add_extension_request(const std::string& name, bool is_required);
        void validate_requested_extensions();
        void query_supported_layers();
        bool find_validation_layer() const;
        void create_debug_messenger();

        static constexpr auto api_version = VK_API_VERSION_1_0;
        static constexpr auto validation_layer_name = "VK_LAYER_KHRONOS_validation";
        std::set<std::string> supported_extensions_{};
        std::map<std::string, bool> requested_extensions_{};
        std::vector<const char*> enabled_extensions_{};
        std::set<std::string> supported_layers_{};
        bool validation_layer_found_ = false;
        VkInstance instance_ = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
        struct {
            PFN_vkCreateDebugUtilsMessengerEXT create = nullptr;
            PFN_vkDestroyDebugUtilsMessengerEXT destroy = nullptr;
        } debug_messenger_procedures_{};
    };

}