#include "pch.h"
#include "VulkanSwapchain.h"

#ifdef DODO_WINDOWS
#   include <vulkan/vulkan_win32.h>
#endif

#include "Vulkan.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "Core/Application.h"
#include "Renderer/Renderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static VkSurfaceKHR CreatePlatformSurface(void* windowHandle, VkInstance instance)
        {
            VkSurfaceKHR surface = VK_NULL_HANDLE;
#ifdef DODO_WINDOWS
            VkWin32SurfaceCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            createInfo.hwnd = static_cast<HWND>(windowHandle);
            createInfo.hinstance = GetModuleHandleW(nullptr);;
            DODO_VK_RESULT(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));
#else
            ASSERT(false, "Platform not supported!");
#endif
            return surface;
        }

        static VkPresentModeKHR ConvertToVkPresentMode(VSyncMode vsyncMode)
        {
            switch (vsyncMode)
            {
                case VSyncMode::Disable: return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case VSyncMode::Enable:  return VK_PRESENT_MODE_FIFO_KHR;
                case VSyncMode::Mailbox: return VK_PRESENT_MODE_MAILBOX_KHR;
                default: DODO_ASSERT(false, "V-Sync mode not supported!");
            }

            return VK_PRESENT_MODE_MAX_ENUM_KHR;
        }

    }

    ////////////////////////////////////////////////////////////////
    // VULKAN SWAPCHAIN ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static PFN_vkGetPhysicalDeviceSurfaceSupportKHR      pfnGetPhysicalDeviceSurfaceSupportKHR      = nullptr;
    static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfnGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
    static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      pfnGetPhysicalDeviceSurfaceFormatsKHR      = nullptr;
    static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR pfnGetPhysicalDeviceSurfacePresentModesKHR = nullptr;

    static PFN_vkCreateSwapchainKHR    pfnCreateSwapchainKHR    = nullptr;
    static PFN_vkDestroySwapchainKHR   pfnDestroySwapchainKHR   = nullptr;
    static PFN_vkGetSwapchainImagesKHR pfnGetSwapchainImagesKHR = nullptr;
    static PFN_vkAcquireNextImageKHR   pfnAcquireNextImageKHR   = nullptr;
    static PFN_vkQueuePresentKHR       pfnQueuePresentKHR       = nullptr;


    VulkanSwapchain::VulkanSwapchain(void* windowHandle, uint32_t width, uint32_t height)
        : m_Width (width )
        , m_Height(height)
    {
        m_Instance = VulkanContext::GetCurrentInstance();
        m_Device   = VulkanContext::GetCurrentDevice();
        m_Surface  = Utils::CreatePlatformSurface(windowHandle, m_Instance);

        // Get proc. addresses.
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceFormatsKHR);
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfacePresentModesKHR);
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceSupportKHR);

        GET_VK_DEVICE_PROC_ADDR(m_Device->GetNativeDevice(), CreateSwapchainKHR);
        GET_VK_DEVICE_PROC_ADDR(m_Device->GetNativeDevice(), DestroySwapchainKHR);
        GET_VK_DEVICE_PROC_ADDR(m_Device->GetNativeDevice(), GetSwapchainImagesKHR);
        GET_VK_DEVICE_PROC_ADDR(m_Device->GetNativeDevice(), AcquireNextImageKHR);
        GET_VK_DEVICE_PROC_ADDR(m_Device->GetNativeDevice(), QueuePresentKHR);

        const Ref<VulkanPhysicalDevice> videoAdapter = m_Device->GetPhysicalDevice();
        m_GraphicsQueueIndex = videoAdapter->GetQueueFamilyIndices().Graphics.value();

        // Get available queues.
        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(videoAdapter->GetNativePhysicalDevice(), &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(videoAdapter->GetNativePhysicalDevice(), &queueCount, queues.data());

        // Find present queue.
        // Prefer a queue that supports both present & graphics!
        uint32_t queueIndex = 0;
        for (size_t i = 0; i < queues.size(); i++)
        {
            VkBool32 supportsPresent = false;
            pfnGetPhysicalDeviceSurfaceSupportKHR(videoAdapter->GetNativePhysicalDevice(), queueIndex, m_Surface, &supportsPresent);
            if (supportsPresent && queueIndex == m_GraphicsQueueIndex)
            {
                m_PresentQueueIndex = queueIndex;
                break;
            }

            queueIndex++;
        }

        // No queue found that supports both present & graphics?
        // Find any that supports present.
        if (!m_PresentQueueIndex.has_value())
        {
            for (size_t i = 0; i < queues.size(); i++)
            {
                VkBool32 supportsPresent = false;
                pfnGetPhysicalDeviceSurfaceSupportKHR(videoAdapter->GetNativePhysicalDevice(), queueIndex, m_Surface, &supportsPresent);
                if (supportsPresent)
                {
                    m_PresentQueueIndex = queueIndex;
                    break;
                }

                queueIndex++;
            }
        }

        // Get present queue.
        DODO_ASSERT(m_PresentQueueIndex.has_value(), "Present queue not found!");
        vkGetDeviceQueue(m_Device->GetNativeDevice(), m_PresentQueueIndex.value(), 0, &m_PresentQueue);

        // (Re)create swapchain.
        RecreateSwapchain();
    }

    void VulkanSwapchain::BeginFrame()
    {
        static constexpr uint64_t defaultFenceTimeout = std::numeric_limits<uint64_t>::max();
        // Wait for the fence associated with the current frame to
        // signal completion.
        DODO_VK_RESULT(vkWaitForFences(m_Device->GetNativeDevice(),
                                       1,
                                       &m_WaitFences.at(m_FrameIndex),
                                       VK_TRUE,
                                       defaultFenceTimeout));

        // TODO(me): Release resources.
        m_ImageIndex = AcquireNextImage();
        DODO_VK_RESULT(vkResetCommandBuffer(m_CmdBuffers.at(m_FrameIndex), 0));
        DODO_VK_RESULT(vkResetFences(m_Device->GetNativeDevice(), 1, &m_WaitFences.at(m_FrameIndex)));
    }

    void VulkanSwapchain::OnResize(uint32_t width, uint32_t height)
    {
        m_Width  = width;
        m_Height = height;
        m_NeedsResize = true;
    }

    void VulkanSwapchain::Present()
    {
        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_Semaphores.ImageAvailable;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_Semaphores.RenderComplete;
        submitInfo.pCommandBuffers = &m_CmdBuffers.at(m_FrameIndex);
        submitInfo.commandBufferCount = 1;
        DODO_VK_RESULT(vkQueueSubmit(m_Device->GetGraphicsQueue(), 1, &submitInfo, m_WaitFences.at(m_FrameIndex)));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_Swapchain;
        presentInfo.pImageIndices = &m_ImageIndex;
        presentInfo.pWaitSemaphores = &m_Semaphores.RenderComplete;
        presentInfo.waitSemaphoreCount = 1;
        VkResult result = pfnQueuePresentKHR(m_Device->GetGraphicsQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_NeedsResize)
        {
            m_NeedsResize = false;
            DODO_VK_RESULT(vkDeviceWaitIdle(m_Device->GetNativeDevice()));
            RecreateSwapchain();
        }
        else
        {
            DODO_VK_RESULT(result);
        }

        m_FrameIndex = (m_FrameIndex + 1) % Renderer::GetSettings().FramesInFlight;
    }

    void VulkanSwapchain::Destroy()
    {
        DODO_VK_RESULT(vkDeviceWaitIdle(m_Device->GetNativeDevice()));

        // Destroy semaphore(s).
        if (m_Semaphores.ImageAvailable)
        {
            vkDestroySemaphore(m_Device->GetNativeDevice(), m_Semaphores.ImageAvailable, nullptr);
        }

        if (m_Semaphores.RenderComplete)
        {
            vkDestroySemaphore(m_Device->GetNativeDevice(), m_Semaphores.RenderComplete, nullptr);
        }

        // Destroy wait fence(s).
        for (auto waitFence : m_WaitFences)
        {
            vkDestroyFence(m_Device->GetNativeDevice(), waitFence, nullptr);
        }

        // Destroy command pool(s).
        if (m_CmdPool)
        {
            vkDestroyCommandPool(m_Device->GetNativeDevice(), m_CmdPool, nullptr);
        }

        // Destroy framebuffers.
        for (auto framebuffer : m_Framebuffers)
        {
            vkDestroyFramebuffer(m_Device->GetNativeDevice(), framebuffer, nullptr);
        }

        // Destroy render pass.
        if (m_RenderPass)
        {
            vkDestroyRenderPass(m_Device->GetNativeDevice(), m_RenderPass, nullptr);
        }

        // Destroy image views.
        for (auto imageView : m_ImageViews)
        {
            vkDestroyImageView(m_Device->GetNativeDevice(), imageView, nullptr);
        }

        // Destroy swapchain.
        if (m_Swapchain)
        {
            pfnDestroySwapchainKHR(m_Device->GetNativeDevice(), m_Swapchain, nullptr);
        }

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        DODO_VK_RESULT(vkDeviceWaitIdle(m_Device->GetNativeDevice()));
    }

    void VulkanSwapchain::RecreateSwapchain()
    {
        // Query support details & validate.
        QuerySupportDetails();
        DODO_ASSERT(AreSupportDetailsAdequate(), "Swapchain support details are not adequate!");

        m_SurfaceFormat = SelectSurfaceFormat();
        m_PresentMode = SelectPresentMode();
        m_Extent = SelectExtent();

        const VkSurfaceCapabilitiesKHR& capabilities = m_SupportDetails.Capabilities;
        m_ImageCount = capabilities.minImageCount + 1;
        if ((capabilities.maxImageCount > 0) && (m_ImageCount > capabilities.maxImageCount))
        {
            m_ImageCount = capabilities.maxImageCount;
        }

        // Vulkan needs to know how the graphics & present queue
        // will communicate when they don't share the same index.
        VkSharingMode imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        std::vector<uint32_t> queueFamilyIndices{};
        if (m_GraphicsQueueIndex != m_PresentQueueIndex.value())
        {
            imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            queueFamilyIndices.push_back(m_GraphicsQueueIndex);
            queueFamilyIndices.push_back(m_PresentQueueIndex.value());
        }

        // Create swapchain.
        VkSwapchainKHR oldSwapchain = m_Swapchain;
        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = m_Surface;
        swapchainCreateInfo.minImageCount = m_ImageCount;
        swapchainCreateInfo.imageFormat = m_SurfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = m_Extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageSharingMode = imageSharingMode;
        swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        swapchainCreateInfo.preTransform = m_SupportDetails.Capabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = m_PresentMode;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.oldSwapchain = oldSwapchain;
        DODO_VK_RESULT(pfnCreateSwapchainKHR(m_Device->GetNativeDevice(), &swapchainCreateInfo, nullptr, &m_Swapchain));
        if (oldSwapchain)
        {
            pfnDestroySwapchainKHR(m_Device->GetNativeDevice(), oldSwapchain, nullptr);
        }

        // Create images & views.
        for (auto imageView : m_ImageViews)
        {
            vkDestroyImageView(m_Device->GetNativeDevice(), imageView, nullptr);
        }

        m_Images.clear();
        m_ImageViews.clear();

        pfnGetSwapchainImagesKHR(m_Device->GetNativeDevice(), m_Swapchain, &m_ImageCount, nullptr);
        m_Images.resize(m_ImageCount);
        m_ImageViews.resize(m_Images.size());
        pfnGetSwapchainImagesKHR(m_Device->GetNativeDevice(), m_Swapchain, &m_ImageCount, m_Images.data());

        for (size_t i = 0; i < m_Images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_Images.at(i);
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_SurfaceFormat.format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            DODO_VK_RESULT(vkCreateImageView(m_Device->GetNativeDevice(), &createInfo, nullptr, &m_ImageViews.at(i)));
        }

        // Create render pass.
        if (m_RenderPass)
        {
            vkDestroyRenderPass(m_Device->GetNativeDevice(), m_RenderPass, nullptr);
        }

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_SurfaceFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference attachmentRef{};
        attachmentRef.attachment = 0;
        attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        DODO_VK_RESULT(vkCreateRenderPass(m_Device->GetNativeDevice(), &renderPassInfo, nullptr, &m_RenderPass));

        // Create framebuffer(s)
        for (auto framebuffer : m_Framebuffers)
        {
            vkDestroyFramebuffer(m_Device->GetNativeDevice(), framebuffer, nullptr);
        }

        m_Framebuffers.clear();
        m_Framebuffers.resize(m_ImageViews.size());

        for (size_t i = 0; i < m_ImageViews.size(); i++)
        {
            VkImageView attachments[] = { m_ImageViews.at(i) };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width  = m_Extent.width;
            framebufferInfo.height = m_Extent.height;
            framebufferInfo.layers = 1;
            DODO_VK_RESULT(vkCreateFramebuffer(m_Device->GetNativeDevice(), &framebufferInfo, nullptr, &m_Framebuffers.at(i)));
        }

        // Create command pool(s) & buffers.
        if (m_CmdPool)
        {
            vkDestroyCommandPool(m_Device->GetNativeDevice(), m_CmdPool, nullptr);
        }

        VkCommandPoolCreateInfo cmdPoolInfo{};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmdPoolInfo.queueFamilyIndex = m_GraphicsQueueIndex;
        DODO_VK_RESULT(vkCreateCommandPool(m_Device->GetNativeDevice(), &cmdPoolInfo, nullptr, &m_CmdPool));

        m_CmdBuffers.clear();
        const uint32_t framesInFlight = Renderer::GetSettings().FramesInFlight;
        m_CmdBuffers.resize(framesInFlight);

        for (size_t i = 0; i < framesInFlight; i++)
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = m_CmdPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            DODO_VK_RESULT(vkAllocateCommandBuffers(m_Device->GetNativeDevice(), &allocInfo, &m_CmdBuffers.at(i)));
        }

        // Create semaphores.
        if (m_Semaphores.ImageAvailable)
        {
            vkDestroySemaphore(m_Device->GetNativeDevice(), m_Semaphores.ImageAvailable, nullptr);
        }

        if (m_Semaphores.RenderComplete)
        {
            vkDestroySemaphore(m_Device->GetNativeDevice(), m_Semaphores.RenderComplete, nullptr);
        }

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        DODO_VK_RESULT(vkCreateSemaphore(m_Device->GetNativeDevice(), &semaphoreInfo, nullptr, &m_Semaphores.ImageAvailable));
        DODO_VK_RESULT(vkCreateSemaphore(m_Device->GetNativeDevice(), &semaphoreInfo, nullptr, &m_Semaphores.RenderComplete));

        // Create wait fences.
        for (auto waitFence : m_WaitFences)
        {
            vkDestroyFence(m_Device->GetNativeDevice(), waitFence, nullptr);
        }

        m_WaitFences.clear();
        m_WaitFences.resize(framesInFlight);

        for (size_t i = 0; i < framesInFlight; i++)
        {
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            DODO_VK_RESULT(vkCreateFence(m_Device->GetNativeDevice(), &fenceInfo, nullptr, &m_WaitFences.at(i)));
        }
    }

    void VulkanSwapchain::QuerySupportDetails()
    {
        VkPhysicalDevice videoAdapter = m_Device->GetPhysicalDevice()->GetNativePhysicalDevice();

        // Get capabilities.
        pfnGetPhysicalDeviceSurfaceCapabilitiesKHR(videoAdapter, m_Surface, &m_SupportDetails.Capabilities);

        // Get surface formats.
        uint32_t formatCount = 0;
        pfnGetPhysicalDeviceSurfaceFormatsKHR(videoAdapter, m_Surface, &formatCount, nullptr);
        if (formatCount > 0)
        {
            m_SupportDetails.SurfaceFormats.resize(formatCount);
            pfnGetPhysicalDeviceSurfaceFormatsKHR(videoAdapter, m_Surface, &formatCount, m_SupportDetails.SurfaceFormats.data());
        }

        // Get present modes.
        uint32_t modeCount = 0;
        pfnGetPhysicalDeviceSurfacePresentModesKHR(videoAdapter, m_Surface, &modeCount, nullptr);
        if (modeCount > 0)
        {
            m_SupportDetails.PresentModes.resize(modeCount);
            pfnGetPhysicalDeviceSurfacePresentModesKHR(videoAdapter, m_Surface, &modeCount, m_SupportDetails.PresentModes.data());
        }
    }

    bool VulkanSwapchain::AreSupportDetailsAdequate() const
    {
        return !m_SupportDetails.SurfaceFormats.empty() && !m_SupportDetails.PresentModes.empty();
    }

    VkSurfaceFormatKHR VulkanSwapchain::SelectSurfaceFormat() const
    {
        VkSurfaceFormatKHR requestedFormat{};
        requestedFormat.format     = VK_FORMAT_B8G8R8A8_SRGB;
        requestedFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        // Target surface format has to be compatible with ImGui when enabled!
        if (Application::GetCurrent().GetSpecs().EnableImGui)
        {
            requestedFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
        }

        for (const auto& format : m_SupportDetails.SurfaceFormats)
        {
            if (format.format == requestedFormat.format && format.colorSpace == requestedFormat.colorSpace)
            {
                return format;
            }
        }

        // Get any surface format.
        return m_SupportDetails.SurfaceFormats[0];
    }

    VkPresentModeKHR VulkanSwapchain::SelectPresentMode() const
    {
        VkPresentModeKHR requestedMode = Utils::ConvertToVkPresentMode(Renderer::GetSettings().VSyncMode);
        // Target present mode has to be compatible with ImGui when enabled!
        if (Application::GetCurrent().GetSpecs().EnableImGui)
        {
            requestedMode = VK_PRESENT_MODE_FIFO_KHR;
        }

        for (VkPresentModeKHR mode : m_SupportDetails.PresentModes)
        {
            if (mode == requestedMode)
            {
                return mode;
            }
        }

        // Get any present mode.
        return m_SupportDetails.PresentModes[0];
    }

    VkExtent2D VulkanSwapchain::SelectExtent() const
    {
        const VkSurfaceCapabilitiesKHR& capabilities = m_SupportDetails.Capabilities;
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        VkExtent2D extent{};
        extent.width  = std::clamp(m_Width , capabilities.minImageExtent.width , capabilities.maxImageExtent.width );
        extent.height = std::clamp(m_Height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return extent;
    }

    uint32_t VulkanSwapchain::AcquireNextImage()
    {
        uint32_t imageIndex = 0;
        DODO_VK_RESULT(pfnAcquireNextImageKHR(m_Device->GetNativeDevice(),
                                              m_Swapchain,
                                              UINT64_MAX,
                                              m_Semaphores.ImageAvailable,
                                              nullptr,
                                              &imageIndex));
        return imageIndex;
    }

}