#include "pch.h"
#include "VulkanSwapchain.h"

#ifdef DODO_WINDOWS
#   include <vulkan/vulkan_win32.h>
#endif

#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanRenderContext.h"
#include "Core/Application.h"
#include "Core/Window.h"

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
            createInfo.hinstance = GetModuleHandleW(nullptr);
            createInfo.hwnd = static_cast<HWND>(windowHandle);
            DODO_VK_RESULT(vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface));
#endif
            return surface;
        }

        static VkPresentModeKHR ConvertToVkPresentMode(VSyncMode vsyncMode)
        {
            switch (vsyncMode)
            {
                case VSyncMode::Disable : return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case VSyncMode::Enable  : return VK_PRESENT_MODE_FIFO_KHR;
                case VSyncMode::Mailbox : return VK_PRESENT_MODE_MAILBOX_KHR;
                default                 : DODO_ASSERT(false, "V-Sync mode not supported!");
            }

            return VK_PRESENT_MODE_MAX_ENUM_KHR;
        }

    }

    ////////////////////////////////////////////////////////////////
    // VULKAN EXTENSION FUNCTIONS //////////////////////////////////
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

    ////////////////////////////////////////////////////////////////
    // VULKAN SWAPCHAIN ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////


    VulkanSwapchain::VulkanSwapchain()
    {
        const Ref<Window>& targetWindow = Application::GetCurrent().GetWindow();
        m_Width    = targetWindow->GetWidth ();
        m_Height   = targetWindow->GetHeight();
        m_Instance = VulkanRenderContext::GetVulkanInstance();
        m_Device   = VulkanRenderContext::GetCurrentDevice();
        m_Surface  = Utils::CreatePlatformSurface(targetWindow->GetHandle(), m_Instance);

        GET_VK_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceFormatsKHR);
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfacePresentModesKHR);
        GET_VK_INSTANCE_PROC_ADDR(m_Instance, GetPhysicalDeviceSurfaceSupportKHR);

        GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), CreateSwapchainKHR);
        GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), DestroySwapchainKHR);
        GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), GetSwapchainImagesKHR);
        GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), AcquireNextImageKHR);
        GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), QueuePresentKHR);

        const Ref<VulkanPhysicalDevice>& physicalDevice = m_Device->GetPhysicalDevice();
        m_GraphicsQueueIndex = physicalDevice->GetQueueFamilyIndices().Graphics.value();

        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->GetVulkanPhysicalDevice(), &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice->GetVulkanPhysicalDevice(), &queueCount, queues.data());
        // Find present queue.
        // Prefer a queue that supports both present & graphics!
        uint32_t queueIndex = 0;
        for (size_t i = 0; i < queues.size(); i++)
        {
            VkBool32 supportsPresent = false;
            pfnGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->GetVulkanPhysicalDevice(), queueIndex, m_Surface, &supportsPresent);
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
                pfnGetPhysicalDeviceSurfaceSupportKHR(physicalDevice->GetVulkanPhysicalDevice(), queueIndex, m_Surface, &supportsPresent);
                if (supportsPresent)
                {
                    m_PresentQueueIndex = queueIndex;
                    break;
                }

                queueIndex++;
            }
        }

        DODO_ASSERT(m_PresentQueueIndex.has_value (), "Present queue not found!");
        vkGetDeviceQueue(m_Device->GetVulkanDevice(), m_PresentQueueIndex.value(), 0, &m_PresentQueue);
        RecreateSwapchain();
    }

    void VulkanSwapchain::RecordTestCommands()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_CommandBuffers.at(m_FrameIndex), &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass;
        renderPassInfo.framebuffer = m_Framebuffers.at(m_ImageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_Extent;

        VkClearValue clearColor = {{{1.0f, 0.6f, 0.3f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_CommandBuffers.at(m_FrameIndex), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) m_Extent.width;
        viewport.height = (float) m_Extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(m_CommandBuffers.at(m_FrameIndex), 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_Extent;
        vkCmdSetScissor(m_CommandBuffers.at(m_FrameIndex), 0, 1, &scissor);

        vkCmdEndRenderPass(m_CommandBuffers.at(m_FrameIndex));

        if (vkEndCommandBuffer(m_CommandBuffers.at(m_FrameIndex)) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    VulkanSwapchain::VulkanSwapchain(void *windowHandle, uint32_t windowWidth, uint32_t windowHeight)
    {
    }

    void VulkanSwapchain::BeginFrame()
    {
        static constexpr uint64_t defaultFenceTimeout = std::numeric_limits<uint64_t>::max();
        // Wait for the fence associated with the current frame to signal completion.
        DODO_VK_RESULT(vkWaitForFences(m_Device->GetVulkanDevice(),
                                       1,
                                       &m_WaitFences.at(m_FrameIndex),
                                       VK_TRUE,
                                       defaultFenceTimeout))

        // TODO: Release resources.
        const VkResult result = AcquireNextImage(&m_ImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            DODO_VK_RESULT(vkDeviceWaitIdle(m_Device->GetVulkanDevice()));
            RecreateSwapchain();
            return;
        }
        else
        {
            DODO_VK_RESULT(result);
        }

        // Only reset the fence when submitting work to avoid a deadlock.
        // Online: https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation
        DODO_VK_RESULT(vkResetFences(m_Device->GetVulkanDevice(), 1, &m_WaitFences.at(m_FrameIndex)));
        DODO_VK_RESULT(vkResetCommandBuffer(m_CommandBuffers.at(m_FrameIndex), 0));

        RecordTestCommands();
    }

    void VulkanSwapchain::OnResize_RenderThread(uint32_t width, uint32_t height)
    {
        m_Width = width, m_Height = height;
        m_NeedsResize = true;
    }

    void VulkanSwapchain::Present_RenderThread()
    {
        const VkPipelineStageFlags waitStage = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = &m_ImageAvailable.at(m_FrameIndex);
        submitInfo.pWaitDstStageMask    = &waitStage;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = &m_RenderComplete.at(m_FrameIndex);
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &m_CommandBuffers.at(m_FrameIndex);
        DODO_VK_RESULT(vkQueueSubmit(m_Device->GetGraphicsVulkanQueue(),
                                     1,
                                     &submitInfo,
                                     m_WaitFences.at(m_FrameIndex)));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = &m_RenderComplete.at(m_FrameIndex);
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &m_Swapchain;
        presentInfo.pImageIndices      = &m_ImageIndex;
        const VkResult result          = pfnQueuePresentKHR(m_Device->GetGraphicsVulkanQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_NeedsResize)
        {
            m_NeedsResize = false;
            vkDeviceWaitIdle(m_Device->GetVulkanDevice());
            RecreateSwapchain();
        }
        else
        {
            DODO_VK_RESULT(result);
        }

        const auto& app = Application::GetCurrent();
        m_FrameIndex = (m_FrameIndex + 1) % app.GetSpecs().RenderSettings.ConcurrentFrameCount;
    }

    void VulkanSwapchain::Destroy()
    {
        vkDeviceWaitIdle(m_Device->GetVulkanDevice());
        // Destroy semaphores & wait fences.
        const auto& app = Application::GetCurrent();
        const uint32_t framesInFlight = app.GetSpecs().RenderSettings.ConcurrentFrameCount;
        for (size_t i = 0; i < framesInFlight; i++)
        {
            vkDestroySemaphore(m_Device->GetVulkanDevice(), m_ImageAvailable.at(i), nullptr);
            vkDestroySemaphore(m_Device->GetVulkanDevice(), m_RenderComplete.at(i), nullptr);
            vkDestroyFence(m_Device->GetVulkanDevice(), m_WaitFences.at(i), nullptr);
        }

        // Destroy command pool(s).
        if (m_CommandPool)
        {
            vkDestroyCommandPool(m_Device->GetVulkanDevice(), m_CommandPool, nullptr);
        }

        // Destroy render pass.
        if (m_RenderPass)
        {
            vkDestroyRenderPass(m_Device->GetVulkanDevice(), m_RenderPass, nullptr);
        }

        // Destroy framebuffers & image views.
        for (size_t i = 0; i < m_ImageCount; i++)
        {
            vkDestroyFramebuffer(m_Device->GetVulkanDevice(), m_Framebuffers.at(i), nullptr);
            vkDestroyImageView(m_Device->GetVulkanDevice(), m_ImageViews.at(i), nullptr);
        }

        // Destroy swapchain.
        if (m_Swapchain)
        {
            pfnDestroySwapchainKHR(m_Device->GetVulkanDevice(), m_Swapchain, nullptr);
        }

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDeviceWaitIdle(m_Device->GetVulkanDevice());
    }

    void VulkanSwapchain::RecreateSwapchain()
    {
        const VkPhysicalDevice physicalDevice = m_Device->GetPhysicalDevice()->GetVulkanPhysicalDevice();
        // Get surface capabilities.
        VkSurfaceCapabilitiesKHR surfaceCaps{};
        pfnGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfaceCaps);

        // Get surface formats.
        std::vector<VkSurfaceFormatKHR> formats{};
        uint32_t formatCount = 0;
        pfnGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr);
        if (formatCount > 0)
        {
            formats.resize(formatCount);
            pfnGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, formats.data());
        }

        // Get present modes.
        std::vector<VkPresentModeKHR> modes{};
        uint32_t modeCount = 0;
        pfnGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &modeCount, nullptr);
        if (modeCount > 0)
        {
            modes.resize(modeCount);
            pfnGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &modeCount, modes.data());
        }
        
        const auto& app = Application::GetCurrent();
        const bool  enableImGui = app.GetSpecs().EnableImGui;
        // Select surface format & color space.
        // Surface format has to be compatible with ImGui when enabled!
        m_SurfaceFormat = formats.back();
        VkSurfaceFormatKHR targetFormat{};
        targetFormat.format     = enableImGui ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_B8G8R8A8_SRGB;
        targetFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        for (const auto& format : formats)
        {
            if (format.format == targetFormat.format && format.colorSpace == targetFormat.colorSpace)
            {
                m_SurfaceFormat = format;
            }
        }

        // Select present mode.
        // Present mode has to be compatible with ImGui when enabled!
        m_PresentMode = modes.back();
        const VkPresentModeKHR targetMode = enableImGui ? VK_PRESENT_MODE_FIFO_KHR : Utils::ConvertToVkPresentMode(app.GetSpecs().RenderSettings.VSyncMode);
        for (const auto mode : modes)
        {
            if (mode == targetMode)
            {
                m_PresentMode = mode;
            }
        }
        
        // Select extent.
        m_Extent = surfaceCaps.currentExtent;
        if (surfaceCaps.currentExtent.width == std::numeric_limits<uint32_t>::max())
        {
            VkExtent2D extent{};
            extent.width  = std::clamp(m_Width , surfaceCaps.minImageExtent.width , surfaceCaps.maxImageExtent.width );
            extent.height = std::clamp(m_Height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);
            m_Extent = extent;
        }

        // Incompatible capabilities / extent.
        if (m_Extent.width == 0 || m_Extent.height == 0)
        {
            return;
        }

        // Get image count.
        m_ImageCount = surfaceCaps.minImageCount + 1;
        if ((surfaceCaps.maxImageCount > 0) && (m_ImageCount > surfaceCaps.maxImageCount))
        {
            m_ImageCount = surfaceCaps.maxImageCount;
        }

        // Create swapchain.
        VkSwapchainKHR oldSwapchain = m_Swapchain;
        {
            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = m_Surface;
            createInfo.minImageCount = m_ImageCount;
            createInfo.imageFormat = m_SurfaceFormat.format;
            createInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
            createInfo.imageExtent = m_Extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            // Images can be used across multiple queue families without explicit ownership transfers.
            // Online: https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
            if (m_GraphicsQueueIndex != m_PresentQueueIndex.value())
            {
                const std::vector<uint32_t> queueFamilyIndices = { m_GraphicsQueueIndex, m_PresentQueueIndex.value() };
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
                createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
            }
            
            createInfo.preTransform = surfaceCaps.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = m_PresentMode;
            createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = oldSwapchain;
            DODO_VK_RESULT(pfnCreateSwapchainKHR(m_Device->GetVulkanDevice(), &createInfo, nullptr, &m_Swapchain));
            if (oldSwapchain)
            {
                pfnDestroySwapchainKHR(m_Device->GetVulkanDevice(), oldSwapchain, nullptr);
            }
        }

        // Create images & views.
        for (auto imageView : m_ImageViews)
        {
            vkDestroyImageView(m_Device->GetVulkanDevice(), imageView, nullptr);
        }

        m_Images.clear();
        m_ImageViews.clear();

        pfnGetSwapchainImagesKHR(m_Device->GetVulkanDevice(), m_Swapchain, &m_ImageCount, nullptr);
        m_Images.resize(m_ImageCount);
        m_ImageViews.resize(m_Images.size());
        pfnGetSwapchainImagesKHR(m_Device->GetVulkanDevice(), m_Swapchain, &m_ImageCount, m_Images.data());
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
            DODO_VK_RESULT(vkCreateImageView(m_Device->GetVulkanDevice(), &createInfo, nullptr, &m_ImageViews.at(i)));
        }
        
        // Create render pass.
        {
            if (m_RenderPass)
            {
                vkDestroyRenderPass(m_Device->GetVulkanDevice(), m_RenderPass, nullptr);
            }

            VkAttachmentDescription colorAttachment{};
            colorAttachment.format          = m_SurfaceFormat.format;
            colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference attachmentRef{};
            attachmentRef.attachment = 0;
            attachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &attachmentRef;

            VkSubpassDependency dependency{};
            dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass    = 0;
            dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            VkRenderPassCreateInfo createInfo{};
            createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            createInfo.attachmentCount = 1;
            createInfo.pAttachments    = &colorAttachment;
            createInfo.subpassCount    = 1;
            createInfo.pSubpasses      = &subpass;
            createInfo.dependencyCount = 1;
            createInfo.pDependencies   = &dependency;
            DODO_VK_RESULT(vkCreateRenderPass(m_Device->GetVulkanDevice(), &createInfo, nullptr, &m_RenderPass));
        }

        // Create framebuffer(s)
        for (auto framebuffer : m_Framebuffers)
        {
            vkDestroyFramebuffer(m_Device->GetVulkanDevice(), framebuffer, nullptr);
        }

        m_Framebuffers.clear();
        m_Framebuffers.resize(m_ImageViews.size());
        for (size_t i = 0; i < m_ImageCount; i++)
        {
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = m_RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments    = &m_ImageViews.at(i);
            framebufferInfo.width           = m_Extent.width;
            framebufferInfo.height          = m_Extent.height;
            framebufferInfo.layers          = 1;
            DODO_VK_RESULT(vkCreateFramebuffer(m_Device->GetVulkanDevice(), &framebufferInfo, nullptr, &m_Framebuffers.at(i)));
        }

        // Create command pool(s) & buffers.
        {
            if (m_CommandPool)
            {
                vkDestroyCommandPool(m_Device->GetVulkanDevice(), m_CommandPool, nullptr);
            }

            VkCommandPoolCreateInfo createInfo{};
            createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            createInfo.queueFamilyIndex = m_GraphicsQueueIndex;
            DODO_VK_RESULT(vkCreateCommandPool(m_Device->GetVulkanDevice(), &createInfo, nullptr, &m_CommandPool));
        }

        m_CommandBuffers.clear();
        const uint32_t framesInFlight = app.GetSpecs().RenderSettings.ConcurrentFrameCount;
        m_CommandBuffers.resize(framesInFlight);
        for (size_t i = 0; i < m_CommandBuffers.size(); i++)
        {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool        = m_CommandPool;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            DODO_VK_RESULT(vkAllocateCommandBuffers(m_Device->GetVulkanDevice(), &allocInfo, &m_CommandBuffers.at(i)));
        }
        
        // Create semaphores.
        for (auto semaphore : m_ImageAvailable)
        {
            vkDestroySemaphore(m_Device->GetVulkanDevice(), semaphore, nullptr);
        }

        for (auto semaphore : m_RenderComplete)
        {
            vkDestroySemaphore(m_Device->GetVulkanDevice(), semaphore, nullptr);
        }

        m_ImageAvailable.clear();
        m_RenderComplete.clear();
        m_ImageAvailable.resize(framesInFlight);
        m_RenderComplete.resize(m_ImageAvailable.size());
        for (size_t i = 0; i < framesInFlight; i++)
        {
            VkSemaphoreCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            DODO_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &createInfo, nullptr, &m_ImageAvailable.at(i)));
            DODO_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &createInfo, nullptr, &m_RenderComplete.at(i)));
        }

        // Create wait fences.
        for (auto waitFence : m_WaitFences)
        {
            vkDestroyFence(m_Device->GetVulkanDevice(), waitFence, nullptr);
        }

        m_WaitFences.clear();
        m_WaitFences.resize(framesInFlight);
        for (size_t i = 0; i < framesInFlight; i++)
        {
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            DODO_VK_RESULT(vkCreateFence(m_Device->GetVulkanDevice(), &fenceInfo, nullptr, &m_WaitFences.at(i)));
        }
    }

    VkResult VulkanSwapchain::AcquireNextImage(uint32_t* imageIndex)
    {
        return pfnAcquireNextImageKHR(m_Device->GetVulkanDevice(),
                                      m_Swapchain,
                                      UINT64_MAX,
                                      m_ImageAvailable.at(m_FrameIndex),
                                      nullptr,
                                      imageIndex);
    }

}