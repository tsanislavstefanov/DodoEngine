#pragma once

#include <vulkan/vulkan.h>

#include "Renderer/Swapchain.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // SWAPCHAIN SUPPORT DETAILS ///////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR Capabilities{};
        std::vector<VkSurfaceFormatKHR> SurfaceFormats{};
        std::vector<VkPresentModeKHR> PresentModes{};
    };

    ////////////////////////////////////////////////////////////////
    // FORWARD DECLARATIONS ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanDevice;

    ////////////////////////////////////////////////////////////////
    // VULKAN SWAPCHAIN ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanSwapchain : public Swapchain
    {
    public:
        VulkanSwapchain(void* windowHandle, uint32_t width, uint32_t height);

        uint32_t GetImageCount() const { return m_ImageCount; }
        VkRenderPass GetRenderPass() const { return m_RenderPass; }

        void BeginFrame() override;
        void OnResize(uint32_t width, uint32_t height) override;
        void Present() override;
        void Destroy() override;

    private:
        void RecreateSwapchain();
        void QuerySupportDetails();
        bool AreSupportDetailsAdequate() const;
        VkSurfaceFormatKHR SelectSurfaceFormat() const;
        VkPresentModeKHR SelectPresentMode() const;
        VkExtent2D SelectExtent() const;
        uint32_t AcquireNextImage();

        VkInstance m_Instance = VK_NULL_HANDLE;
        Ref<VulkanDevice> m_Device = nullptr;

        uint32_t m_Width  = 0;
        uint32_t m_Height = 0;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

        uint32_t m_GraphicsQueueIndex = UINT32_MAX;
        std::optional<uint32_t> m_PresentQueueIndex{};
        VkQueue m_PresentQueue = VK_NULL_HANDLE;

        SwapchainSupportDetails m_SupportDetails{};
        VkSurfaceFormatKHR m_SurfaceFormat {};
        VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkExtent2D m_Extent{};
        uint32_t m_ImageCount = 0;
        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> m_Images{};
        std::vector<VkImageView> m_ImageViews{};
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> m_Framebuffers{};
        VkCommandPool m_CmdPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_CmdBuffers{};

        struct
        {
            VkSemaphore ImageAvailable{};
            VkSemaphore RenderComplete{};
        }
        m_Semaphores;

        std::vector<VkFence> m_WaitFences{};
        uint32_t m_ImageIndex = 0;
        bool m_NeedsResize = true;
        uint32_t m_FrameIndex = 0;
    };

}