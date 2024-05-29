#pragma once

#include <vulkan/vulkan.h>

#include "Renderer/Swapchain.h"

namespace Dodo {

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
        VkResult AcquireNextImage(uint32_t* imageIndex);

        void RecordTestCommands();

        VkInstance m_Instance = VK_NULL_HANDLE;
        Ref<VulkanDevice> m_Device = nullptr;

        uint32_t m_Width = 0, m_Height = 0;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
        bool m_NeedsResize = true;

        uint32_t m_GraphicsQueueIndex = UINT32_MAX;
        std::optional<uint32_t> m_PresentQueueIndex{};
        VkQueue m_PresentQueue = VK_NULL_HANDLE;

        VkSurfaceFormatKHR m_SurfaceFormat{};
        VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkExtent2D m_Extent{};
        uint32_t m_ImageCount = 0;
        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

        std::vector<VkImage> m_Images{};
        std::vector<VkImageView> m_ImageViews{};
        uint32_t m_ImageIndex = 0;
        
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> m_Framebuffers{};

        VkCommandPool m_CommandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_CommandBuffers{};

        struct
        {
            std::vector<VkSemaphore> ImageAvailable{};
            std::vector<VkSemaphore> RenderComplete{};
        }
        m_Semaphores;
        std::vector<VkFence> m_WaitFences{};
        uint32_t m_FrameIndex = 0;
    };

}