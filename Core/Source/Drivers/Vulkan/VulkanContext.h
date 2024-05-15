#pragma once

#include <vulkan/vulkan.h>

#include "Renderer/RenderContext.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN CONTEXT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanContext : public RenderContext
    {
    public:
        VulkanContext();

        // Inherited via [RenderContext].
        void PrepareBuffers() override;

        void Resize(uint32_t width, uint32_t height) override;

        void SwapBuffers() override;

        void Dispose() override;

    private:
        static VulkanContext* s_Context;

        ////////////////////////////////////////////////////////////
        // VULKAN INSTANCE /////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct VulkanInstance
        {
            uint32_t                    ApiVersion = VK_API_VERSION_1_0;
            std::set<std::string>       SupportedExtensions{};
            std::map<std::string, bool> RequestedExtensions{};
            std::set<std::string>       EnabledExtensions{};
            VkInstance                  Handle     = VK_NULL_HANDLE;
        };

        void InitInstance();

        void DisposeInstance() const;

        VulkanInstance m_Instance{};
        const char*    m_LayerName  = "VK_LAYER_KHRONOS_validation";
        bool           m_LayerFound = false;

        ////////////////////////////////////////////////////////////
        // VULKAN FUNCTIONS ////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct VulkanFunctions
        {
            // DebugMessenger.
            PFN_vkCreateDebugUtilsMessengerEXT  pfnCreateDebugUtilsMessengerEXT  = nullptr;
            PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT = nullptr;

            // Device.
            PFN_vkGetPhysicalDeviceSurfaceSupportKHR      pfnGetPhysicalDeviceSurfaceSupportKHR      = nullptr;
            PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfnGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
            PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      pfnGetPhysicalDeviceSurfaceFormatsKHR      = nullptr;
            PFN_vkGetPhysicalDeviceSurfacePresentModesKHR pfnGetPhysicalDeviceSurfacePresentModesKHR = nullptr;

            // Swapchain.
            PFN_vkCreateSwapchainKHR    pfnCreateSwapchainKHR    = nullptr;
            PFN_vkGetSwapchainImagesKHR pfnGetSwapchainImagesKHR = nullptr;
            PFN_vkAcquireNextImageKHR   pfnAcquireNextImageKHR   = nullptr;
            PFN_vkQueuePresentKHR       pfnQueuePresentKHR       = nullptr;
            PFN_vkDestroySwapchainKHR   pfnDestroySwapchainKHR   = nullptr;
        };

        VulkanFunctions m_Funcs{};

        ////////////////////////////////////////////////////////////
        // VULKAN DEBUG MESSENGER //////////////////////////////////
        ////////////////////////////////////////////////////////////
        
        struct VulkanDebugMessenger
        {
            static VKAPI_ATTR VkBool32 VKAPI_CALL Report(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                         void* userData);

            VkDebugUtilsMessengerEXT Handle = VK_NULL_HANDLE;
        };

        void InitDebugMessenger();

        void DisposeDebugMessenger() const;

        VulkanDebugMessenger m_DebugMessenger{};

        ////////////////////////////////////////////////////////////
        // VULKAN SURFACE //////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct VulkanSurface
        {
            uint32_t     Width  = 0;
            uint32_t     Height = 0;
            VkSurfaceKHR Handle = nullptr;
        };

        void InitSurface();

        static std::string GetSurfaceExtension();

        void DisposeSurface() const;

        VulkanSurface m_Surface{};

        ////////////////////////////////////////////////////////////
        // VULKAN ADAPTER //////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct VulkanAdapter
        {
            VkPhysicalDevice           Handle     = VK_NULL_HANDLE;
            VkPhysicalDeviceProperties Properties{};
            std::set<std::string>      SupportedExtensions{};
            const char*                VendorName = "Unknown";
        };

        void InitAdapter();

        VulkanAdapter m_Adapter{};

        ////////////////////////////////////////////////////////////
        // VULKAN QUEUE ////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct VulkanQueue
        {
            std::optional<uint32_t> FamilyIndex{};
            VkQueue                 Handle = VK_NULL_HANDLE;
        };

        void FindQueues();

        VulkanQueue m_GraphicsQueue{};

        ////////////////////////////////////////////////////////////
        // VULKAN DEVICE ///////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct VulkanDevice
        {
            std::map<std::string, bool>          RequestedExtensions{};
            std::set<std::string>                EnabledExtensions{};
            std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos{};
            VkDevice                             Handle = VK_NULL_HANDLE;
        };

        void InitDevice();

        void DisposeDevice() const;

        VulkanDevice m_Device{};

        ////////////////////////////////////////////////////////////
        // VULKAN SWAPCHAIN ////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct VulkanSwapchain
        {
            VkSurfaceCapabilitiesKHR        Capabilities{};
            std::vector<VkSurfaceFormatKHR> SurfaceFormats{};
            std::vector<VkPresentModeKHR>   PresentModes{};
            VkSurfaceFormatKHR              SurfaceFormat{};
            VkPresentModeKHR                PresentMode = VK_PRESENT_MODE_FIFO_KHR;
            VkExtent2D                      Extent{};
            uint32_t                        ImageCount  = 0;
            VulkanQueue                     PresentQueue{};
            VkSwapchainKHR                  Handle      = VK_NULL_HANDLE;
            std::vector<VkImage>            Images{};
            std::vector<VkImageView>        ImageViews{};
            VkRenderPass                    RenderPass  = VK_NULL_HANDLE;
            std::vector<VkFramebuffer>      Framebuffers{};
            VkCommandPool                   CmdPool     = VK_NULL_HANDLE;
            std::vector<VkCommandBuffer>    CmdBuffers{};
            std::vector<VkSemaphore>        ImageAvailableSemaphores{};
            std::vector<VkSemaphore>        RenderCompleteSemaphores{};
            std::vector<VkFence>            WaitFences{};
            uint32_t                        FrameIndex  = 0;
            uint32_t                        ImageIndex  = 0;
            bool                            NeedsResize = true;
        };

        void InitSwapchain();

        void RecreateSwapchain();

        void QuerySwapchainSupportDetails();

        bool AreSwapchainSupportDetailsAdequate() const;

        VkSurfaceFormatKHR SelectSwapchainSurfaceFormat() const;

        VkPresentModeKHR SelectSwapchainPresentMode() const;

        VkExtent2D SelectSwapchainExtent() const;

        void FindPresentQueue();

        void DisposeSwapchain() const;

        VulkanSwapchain m_Swapchain{};
    };

}