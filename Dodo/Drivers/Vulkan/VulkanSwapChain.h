#pragma once

#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "Renderer/SwapChain.h"
#include "Renderer/VSyncMode.h"

namespace Dodo {

    class Window;

    class VulkanSwapChain : public SwapChain
    {
    public:
        VulkanSwapChain(Ref<VulkanInstance> instance, Ref<VulkanDevice> device, const Window& targetWindow, VSyncMode vsyncMode);
        ~VulkanSwapChain();

        void BeginFrame() override;
        void EndFrame() override;
        void OnResize(uint32_t width, uint32_t height) override;

    private:
        struct SupportDetails
        {
            VkSurfaceCapabilitiesKHR Capabilities{};
            std::vector<VkSurfaceFormatKHR> Formats{};
            std::vector<VkPresentModeKHR> PresentModes{};
        };

        struct Frame
        {
            VkCommandPool CommandPool = VK_NULL_HANDLE;
            VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
            VkSemaphore ImageAvailable = VK_NULL_HANDLE;
            VkSemaphore RenderComplete = VK_NULL_HANDLE;
            VkFence WaitFence = VK_NULL_HANDLE;
        };

        std::optional<uint32_t> FindPresentQueueFamilyIndex() const;
        void ReCreateSwapChain(uint32_t width, uint32_t height);
        void CreateSwapChain(uint32_t width, uint32_t height);
        SupportDetails QuerySupportDetails() const;
        VkSurfaceFormatKHR SelectSurfaceFormat(const SupportDetails& supportDetails) const;
        VkPresentModeKHR SelectPresentMode(const SupportDetails& supportDetails) const;
        VkResult AcquireNextImage(const Frame& frame, uint32_t* imageIndex);

        static constexpr auto FramesInFlight = 3;
        Ref<VulkanInstance> m_Instance = nullptr;
        Ref<VulkanDevice> m_Device = nullptr;
        VSyncMode m_VSyncMode = VSyncMode::None;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

        PFN_vkGetPhysicalDeviceSurfaceSupportKHR pfnGetPhysicalDeviceSurfaceSupportKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfnGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR pfnGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR pfnGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
        PFN_vkCreateSwapchainKHR pfnCreateSwapchainKHR = nullptr;
        PFN_vkGetSwapchainImagesKHR pfnGetSwapchainImagesKHR = nullptr;
        PFN_vkAcquireNextImageKHR pfnAcquireNextImageKHR = nullptr;
        PFN_vkQueuePresentKHR pfnQueuePresentKHR = nullptr;
        PFN_vkDestroySwapchainKHR pfnDestroySwapchainKHR = nullptr;
        std::optional<uint32_t> m_PresentQueueIndex{};
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        VkExtent2D m_Extent{};
        uint32_t m_ImageCount = 0;
        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
        std::vector<VkImage> m_Images{};
        std::vector<VkImageView> m_ImageViews{};
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> m_Framebuffers{};
        std::vector<Frame> m_Frames{};
        uint32_t m_FrameIndex = 0;
        uint32_t m_ImageIndex = 0;
        bool m_NeedsResize = true;

        // TODO: remove this.
        void RecordTestCommands();
    };

}