#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanSwapChain.h"
#include "Core/Window.h"

namespace Dodo {

    namespace Utils {

        static VkPresentModeKHR ConvertToVulkanPresentMode(VSyncMode vsync_mode)
        {
            switch (vsync_mode)
            {
                case VSyncMode::Disabled: return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case VSyncMode::Enabled:  return VK_PRESENT_MODE_FIFO_KHR;
                case VSyncMode::Mailbox:  return VK_PRESENT_MODE_MAILBOX_KHR;
                default: break;
            }

            DODO_VERIFY(false);
            return VK_PRESENT_MODE_MAX_ENUM_KHR;
        }

    }

    VulkanSwapChain::VulkanSwapChain(Ref<VulkanInstance> instance, Ref<VulkanDevice> device, const Window& targetWindow, VSyncMode vsyncMode)
        : m_Instance(instance)
        , m_Device(device)
        , m_VSyncMode(vsyncMode)
    {
        m_Surface = targetWindow.CreateVulkanSurface(m_Instance->GetVulkanInstance());

        DODO_GET_VK_INSTANCE_PROC_ADDR(m_Instance->GetVulkanInstance(), GetPhysicalDeviceSurfaceCapabilitiesKHR);
        DODO_GET_VK_INSTANCE_PROC_ADDR(m_Instance->GetVulkanInstance(), GetPhysicalDeviceSurfaceFormatsKHR);
        DODO_GET_VK_INSTANCE_PROC_ADDR(m_Instance->GetVulkanInstance(), GetPhysicalDeviceSurfacePresentModesKHR);
        DODO_GET_VK_INSTANCE_PROC_ADDR(m_Instance->GetVulkanInstance(), GetPhysicalDeviceSurfaceSupportKHR);

        DODO_GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), CreateSwapchainKHR);
        DODO_GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), GetSwapchainImagesKHR);
        DODO_GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), AcquireNextImageKHR);
        DODO_GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), QueuePresentKHR);
        DODO_GET_VK_DEVICE_PROC_ADDR(m_Device->GetVulkanDevice(), DestroySwapchainKHR);

        m_PresentQueueIndex = FindPresentQueueFamilyIndex();
        DODO_ASSERT(m_PresentQueueIndex.has_value(), "Present queue not found!");
        vkGetDeviceQueue(m_Device->GetVulkanDevice(), m_PresentQueueIndex.value(), 0, &m_PresentQueue);

        CreateSwapChain(targetWindow.GetWidth(), targetWindow.GetHeight());
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(m_Device->GetVulkanDevice()));
        if (m_SwapChain)
            pfnDestroySwapchainKHR(m_Device->GetVulkanDevice(), m_SwapChain, nullptr);

        if (m_Surface)
            vkDestroySurfaceKHR(m_Instance->GetVulkanInstance(), m_Surface, nullptr);

        for (size_t i = 0; i < m_ImageCount; i++)
            vkDestroyImageView(m_Device->GetVulkanDevice(), m_ImageViews.at(i), nullptr);

        if (m_RenderPass)
            vkDestroyRenderPass(m_Device->GetVulkanDevice(), m_RenderPass, nullptr);

        for (size_t i = 0; i < m_ImageCount; i++)
            vkDestroyFramebuffer(m_Device->GetVulkanDevice(), m_Framebuffers.at(i), nullptr);

        for (size_t i = 0; i < m_Frames.size(); i++)
        {
            Frame& frame = m_Frames.at(i);
            vkDestroyCommandPool(m_Device->GetVulkanDevice(), frame.CommandPool, nullptr);
            vkDestroySemaphore(m_Device->GetVulkanDevice(), frame.ImageAvailable, nullptr);
            vkDestroySemaphore(m_Device->GetVulkanDevice(), frame.RenderComplete, nullptr);
            vkDestroyFence(m_Device->GetVulkanDevice(), frame.WaitFence, nullptr);
        }

        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(m_Device->GetVulkanDevice()));
    }

    void VulkanSwapChain::BeginFrame()
    {
        constexpr uint64_t defaultTimeout = std::numeric_limits<uint64_t>::max();
        Frame& frame = m_Frames.at(m_FrameIndex);

        Stopwatch waitStopwatch{};
        DODO_VERIFY_VK_RESULT(vkWaitForFences(m_Device->GetVulkanDevice(), 1, &frame.WaitFence, VK_TRUE, defaultTimeout));
        m_PerformanceStats.GPUWaitTime = waitStopwatch.GetMilliseconds();

        VkResult result = AcquireNextImage(frame, &m_ImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            ReCreateSwapChain(m_Width, m_Height);
            result = AcquireNextImage(frame, &m_ImageIndex);
        }

        DODO_VERIFY_VK_RESULT(result);
        // Only reset the fence when submitting work to
        // avoid a deadlock.
        // Online: https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation
        DODO_VERIFY_VK_RESULT(vkResetFences(m_Device->GetVulkanDevice(), 1, &frame.WaitFence));
        DODO_VERIFY_VK_RESULT(vkResetCommandBuffer(frame.CommandBuffer, 0));

        // TODO: remove this.
        RecordTestCommands();
    }

    void VulkanSwapChain::EndFrame()
    {
        Frame& frame = m_Frames.at(m_FrameIndex);
        const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &frame.ImageAvailable;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &frame.RenderComplete;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &frame.CommandBuffer;
        DODO_VERIFY_VK_RESULT(vkQueueSubmit(m_Device->GetVulkanGraphicsQueue(), 1, &submitInfo, frame.WaitFence));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &frame.RenderComplete;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_SwapChain;
        presentInfo.pImageIndices = &m_ImageIndex;
        const VkResult result = pfnQueuePresentKHR(m_Device->GetVulkanGraphicsQueue(), &presentInfo);
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR) || m_NeedsResize)
        {
            m_NeedsResize = false;
            ReCreateSwapChain(m_Width, m_Height);
        }
        else
        {
            DODO_VERIFY_VK_RESULT(result);
        }

        m_FrameIndex = (m_FrameIndex + 1) % FramesInFlight;
    }

    void VulkanSwapChain::OnResize(uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;
        m_NeedsResize = true;
    }

    std::optional<uint32_t> VulkanSwapChain::FindPresentQueueFamilyIndex() const
    {
        Ref<VulkanAdapter> adapter = m_Device->GetAdapter();
        const uint32_t graphicsQueueIndex = adapter->GetGraphicsQueueIndex();
        VkBool32 supportsPresent = VK_FALSE;
        // This is ideal scenario. Always prefer queue that
        // supports graphics and present!
        pfnGetPhysicalDeviceSurfaceSupportKHR(adapter->GetVulkanPhysicalDevice(), graphicsQueueIndex, m_Surface, &supportsPresent);
        if (supportsPresent)
            return graphicsQueueIndex;

        for (uint32_t queueIndex = 0; queueIndex < adapter->GetSupportedVulkanQueueFamilies().size(); queueIndex++)
        {
            // Waste of time, because we already tried the
            // graphics queue!
            if (queueIndex == graphicsQueueIndex)
                continue;

            pfnGetPhysicalDeviceSurfaceSupportKHR(adapter->GetVulkanPhysicalDevice(), queueIndex, m_Surface, &supportsPresent);
            if (supportsPresent)
                return queueIndex;
        }

        return std::nullopt;
    }

    void VulkanSwapChain::ReCreateSwapChain(uint32_t width, uint32_t height)
    {
        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(m_Device->GetVulkanDevice()));
        CreateSwapChain(width, height);
        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(m_Device->GetVulkanDevice()));
    }

    void VulkanSwapChain::CreateSwapChain(uint32_t width, uint32_t height)
    {
        m_Width = width;
        m_Height = height;

        const SupportDetails supportDetails = QuerySupportDetails();
        const VkSurfaceFormatKHR surfaceFormat = SelectSurfaceFormat(supportDetails);
        const VkPresentModeKHR presentMode = SelectPresentMode(supportDetails);
        const VkSurfaceCapabilitiesKHR capabilities = supportDetails.Capabilities;

        m_Extent = capabilities.currentExtent;
        if (m_Extent.width == std::numeric_limits<uint32_t>::max())
        {
            m_Extent.width  = std::clamp(m_Width , capabilities.minImageExtent.width , capabilities.maxImageExtent.width );
            m_Extent.height = std::clamp(m_Height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        if ((m_Extent.width == 0) || (m_Extent.height == 0))
            return;

        m_ImageCount = capabilities.minImageCount + 1;
        if ((capabilities.maxImageCount > 0) && (m_ImageCount > capabilities.maxImageCount))
            m_ImageCount = capabilities.maxImageCount;

        VkSwapchainKHR oldSwapChain = m_SwapChain;
        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = m_Surface;
        swapChainCreateInfo.minImageCount = m_ImageCount;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = m_Extent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // Images can be used across multiple queue families
        // without explicit ownership transfers.
        // Online: https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
        const uint32_t graphicsQueueIndex = m_Device->GetAdapter()->GetGraphicsQueueIndex();
        if (graphicsQueueIndex != m_PresentQueueIndex.value())
        {
            std::array<uint32_t, 2> queueFamilyIndexes = { graphicsQueueIndex, m_PresentQueueIndex.value() };
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndexes.size());
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndexes.data();
        }

        swapChainCreateInfo.preTransform = capabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = oldSwapChain;
        DODO_VERIFY_VK_RESULT(pfnCreateSwapchainKHR(m_Device->GetVulkanDevice(), &swapChainCreateInfo, VK_NULL_HANDLE, &m_SwapChain));

        if (oldSwapChain)
            pfnDestroySwapchainKHR(m_Device->GetVulkanDevice(), oldSwapChain, VK_NULL_HANDLE);

        for (auto imageView : m_ImageViews)
            vkDestroyImageView(m_Device->GetVulkanDevice(), imageView, VK_NULL_HANDLE);

        m_Images.clear();
        m_ImageViews.clear();
        pfnGetSwapchainImagesKHR(m_Device->GetVulkanDevice(), m_SwapChain, &m_ImageCount, VK_NULL_HANDLE);
        m_Images.resize(m_ImageCount);
        m_ImageViews.resize(m_ImageCount);
        pfnGetSwapchainImagesKHR(m_Device->GetVulkanDevice(), m_SwapChain, &m_ImageCount, m_Images.data());
        for (size_t i = 0; i < m_ImageCount; i++)
        {
            VkImageViewCreateInfo imageViewcreateInfo{};
            imageViewcreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewcreateInfo.image = m_Images.at(i);
            imageViewcreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewcreateInfo.format = surfaceFormat.format;
            imageViewcreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewcreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewcreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewcreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewcreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewcreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewcreateInfo.subresourceRange.levelCount = 1;
            imageViewcreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewcreateInfo.subresourceRange.layerCount = 1;
            DODO_VERIFY_VK_RESULT(vkCreateImageView(m_Device->GetVulkanDevice(), &imageViewcreateInfo, VK_NULL_HANDLE, &m_ImageViews.at(i)));
        }

        if (m_RenderPass)
            vkDestroyRenderPass(m_Device->GetVulkanDevice(), m_RenderPass, VK_NULL_HANDLE);

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = surfaceFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentReference;

        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachment;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;
        DODO_VERIFY_VK_RESULT(vkCreateRenderPass(m_Device->GetVulkanDevice(), &renderPassCreateInfo, VK_NULL_HANDLE, &m_RenderPass));

        for (auto framebuffer : m_Framebuffers)
            vkDestroyFramebuffer(m_Device->GetVulkanDevice(), framebuffer, VK_NULL_HANDLE);

        m_Framebuffers.clear();
        m_Framebuffers.resize(m_ImageCount);
        for (size_t i = 0; i < m_ImageCount; i++)
        {
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = m_RenderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = &m_ImageViews.at(i);
            framebufferCreateInfo.width = m_Extent.width;
            framebufferCreateInfo.height = m_Extent.height;
            framebufferCreateInfo.layers = 1;
            DODO_VERIFY_VK_RESULT(vkCreateFramebuffer(m_Device->GetVulkanDevice(), &framebufferCreateInfo, VK_NULL_HANDLE, &m_Framebuffers.at(i)));
        }

        for (Frame& frame : m_Frames)
        {
            vkDestroyCommandPool(m_Device->GetVulkanDevice(), frame.CommandPool, VK_NULL_HANDLE);
            vkDestroySemaphore(m_Device->GetVulkanDevice(), frame.ImageAvailable, VK_NULL_HANDLE);
            vkDestroySemaphore(m_Device->GetVulkanDevice(), frame.RenderComplete, VK_NULL_HANDLE);
            vkDestroyFence(m_Device->GetVulkanDevice(), frame.WaitFence, VK_NULL_HANDLE);
        }

        m_Frames.clear();
        m_Frames.resize(FramesInFlight);
        for (size_t i = 0; i < FramesInFlight; i++)
        {
            Frame& frame = m_Frames.at(i);
            VkCommandPoolCreateInfo commandPoolCreateInfo{};
            commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            commandPoolCreateInfo.queueFamilyIndex = graphicsQueueIndex;
            DODO_VERIFY_VK_RESULT(vkCreateCommandPool(m_Device->GetVulkanDevice(), &commandPoolCreateInfo, VK_NULL_HANDLE, &frame.CommandPool));

            VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = frame.CommandPool;
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = 1;
            DODO_VERIFY_VK_RESULT(vkAllocateCommandBuffers(m_Device->GetVulkanDevice(), &commandBufferAllocateInfo, &frame.CommandBuffer));

            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            DODO_VERIFY_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, VK_NULL_HANDLE, &frame.ImageAvailable));
            DODO_VERIFY_VK_RESULT(vkCreateSemaphore(m_Device->GetVulkanDevice(), &semaphoreCreateInfo, VK_NULL_HANDLE, &frame.RenderComplete));

            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            DODO_VERIFY_VK_RESULT(vkCreateFence(m_Device->GetVulkanDevice(), &fenceCreateInfo, VK_NULL_HANDLE, &frame.WaitFence));
        }
    }

    VulkanSwapChain::SupportDetails VulkanSwapChain::QuerySupportDetails() const
    {
        SupportDetails supportDetails{};
        const VkPhysicalDevice physicalDevice = m_Device->GetAdapter()->GetVulkanPhysicalDevice();
        pfnGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &supportDetails.Capabilities);

        uint32_t formatCount = 0;
        pfnGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, VK_NULL_HANDLE);
        if (formatCount > 0)
        {
            supportDetails.Formats.resize(formatCount);
            pfnGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, supportDetails.Formats.data());
        }

        uint32_t presentModeCount = 0;
        pfnGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, VK_NULL_HANDLE);
        if (presentModeCount > 0)
        {
            supportDetails.PresentModes.resize(presentModeCount);
            pfnGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, supportDetails.PresentModes.data());
        }

        return supportDetails;
    }

    VkSurfaceFormatKHR VulkanSwapChain::SelectSurfaceFormat(const SupportDetails& supportDetails) const
    {
        VkSurfaceFormatKHR targetFormat{};
        targetFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
        targetFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        for (const VkSurfaceFormatKHR& format : supportDetails.Formats)
            if (format.format == targetFormat.format && format.colorSpace == targetFormat.colorSpace)
                return format;

        return supportDetails.Formats.back();
    }

    VkPresentModeKHR VulkanSwapChain::SelectPresentMode(const SupportDetails& supportDetails) const
    {
        const auto targetPresentMode = Utils::ConvertToVulkanPresentMode(m_VSyncMode);
        for (const auto presentMode : supportDetails.PresentModes)
            if (presentMode == targetPresentMode)
                return presentMode;

        return supportDetails.PresentModes.back();
    }

    VkResult VulkanSwapChain::AcquireNextImage(const Frame& frame, uint32_t* imageIndex)
    {
        constexpr uint64_t defaultTimeout = std::numeric_limits<uint64_t>::max();
        return pfnAcquireNextImageKHR(m_Device->GetVulkanDevice(), m_SwapChain, defaultTimeout, frame.ImageAvailable, VK_NULL_HANDLE, imageIndex);
    }

    void VulkanSwapChain::RecordTestCommands()
    {
        auto& frame = m_Frames.at(m_FrameIndex);

        VkCommandBufferBeginInfo cmd_buffer_begin_info{};
        cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        DODO_VERIFY_VK_RESULT(vkBeginCommandBuffer(frame.CommandBuffer, &cmd_buffer_begin_info));

        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = m_RenderPass;
        render_pass_begin_info.framebuffer = m_Framebuffers.at(m_ImageIndex);
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = m_Extent;

        VkClearValue clear_color = {{{ 1.0f, 0.3f, 0.15f, 1.0f }}};
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues    = &clear_color;
        vkCmdBeginRenderPass(frame.CommandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_Extent.width;
        viewport.height = (float)m_Extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(frame.CommandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_Extent;
        vkCmdSetScissor(frame.CommandBuffer, 0, 1, &scissor);
        vkCmdEndRenderPass(frame.CommandBuffer);
        DODO_VERIFY_VK_RESULT(vkEndCommandBuffer(frame.CommandBuffer));
    }

}