#pragma once

#include "graphics/swapchain.h"
#include "vulkan_instance.h"
#include "vulkan_device.h"
#include "graphics/vsync_mode.h"

namespace Dodo {

    class Window;

    class VulkanSwapchain : public Swapchain
    {
    public:
        VulkanSwapchain(
                Ref<VulkanInstance> instance,
                Ref<VulkanDevice> device,
                const Window& target_window,
                VSyncMode vsync_mode);

        ~VulkanSwapchain();

        void begin_frame() override;
        void end_frame() override;
        void on_resize(uint32_t width, uint32_t height) override;

    private:
        struct SupportDetails
        {
            VkSurfaceCapabilitiesKHR capabilities{};
            std::vector<VkSurfaceFormatKHR> formats{};
            std::vector<VkPresentModeKHR> present_modes{};
        };

        struct Frame
        {
            VkCommandPool cmd_pool = VK_NULL_HANDLE;
            VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
            VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
            VkSemaphore render_complete_semaphore = VK_NULL_HANDLE;
            VkFence wait_fence = VK_NULL_HANDLE;
        };

        void load_surface_and_swapchain_procedures();
        std::optional<uint32_t> find_present_queue_family_index() const;
        void recreate_swapchain(uint32_t width, uint32_t height);
        void create_swapchain(uint32_t width, uint32_t height);
        SupportDetails query_support_details() const;
        VkSurfaceFormatKHR select_surface_format(const SupportDetails& support_details) const;
        VkPresentModeKHR select_present_mode(const SupportDetails& support_details) const;
        VkResult acquire_next_image_for(const Frame& frame, uint32_t* image_index);

        static constexpr auto frames_in_flight = 3;
        Ref<VulkanInstance> instance_ = nullptr;
        Ref<VulkanDevice> device_ = nullptr;
        VSyncMode vsync_mode_ = VSyncMode::none;
        VkSurfaceKHR surface_ = VK_NULL_HANDLE;
        struct {
            PFN_vkGetPhysicalDeviceSurfaceSupportKHR get_support = nullptr;
            PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR get_capabilities = nullptr;
            PFN_vkGetPhysicalDeviceSurfaceFormatsKHR get_formats = nullptr;
            PFN_vkGetPhysicalDeviceSurfacePresentModesKHR get_present_modes = nullptr;
        } surface_procedures_;

        struct {
            PFN_vkCreateSwapchainKHR create = nullptr;
            PFN_vkGetSwapchainImagesKHR get_images = nullptr;
            PFN_vkAcquireNextImageKHR acquire_next_image = nullptr;
            PFN_vkQueuePresentKHR present = nullptr;
            PFN_vkDestroySwapchainKHR destroy = nullptr;
        } swapchain_procedures_;

        std::optional<uint32_t> present_queue_index_{};
        VkQueue present_queue_ = VK_NULL_HANDLE;
        uint32_t width_ = 0;
        uint32_t height_ = 0;
        VkExtent2D extent_{};
        uint32_t image_count_ = 0;
        VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
        std::vector<VkImage> images_{};
        std::vector<VkImageView> image_views_{};
        VkRenderPass render_pass_ = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> framebuffers_{};
        std::vector<Frame> frames_{};
        uint32_t frame_index_ = 0;
        uint32_t image_index_ = 0;
        bool needs_resize_ = true;

        // TODO: remove this.
        void RecordTestCommands();
    };

}