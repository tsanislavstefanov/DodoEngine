#include "pch.h"
#include "vulkan.h"
#include "vulkan_swapchain.h"
#include "core/window.h"

namespace Dodo {

    namespace Utils {

        static VkPresentModeKHR to_vulkan_present_mode(VSyncMode vsync_mode)
        {
            switch (vsync_mode)
            {
                case VSyncMode::disabled:
                    return VK_PRESENT_MODE_IMMEDIATE_KHR;

                case VSyncMode::enabled:
                    return VK_PRESENT_MODE_FIFO_KHR;

                case VSyncMode::mailbox:
                    return VK_PRESENT_MODE_MAILBOX_KHR;

                default:
                    DODO_ASSERT(false, "VSync mode not supported!");
                    break;
            }

            return VK_PRESENT_MODE_MAX_ENUM_KHR;
        }

    }

    VulkanSwapchain::VulkanSwapchain(
            Ref<VulkanInstance> instance,
            Ref<VulkanDevice> device,
            const Window& target_window,
            VSyncMode vsync_mode)
        : instance_(instance)
        , device_(device)
        , vsync_mode_(vsync_mode)
    {
        surface_ = target_window.create_vulkan_surface(instance_->get_vulkan_instance());
        load_surface_and_swapchain_procedures();

        present_queue_index_ = find_present_queue_family_index();
        DODO_ASSERT(present_queue_index_.has_value(), "Present queue not found!");
        vkGetDeviceQueue(device_->get_vulkan_device(), present_queue_index_.value(), 0, &present_queue_);

        create_swapchain(target_window.width(), target_window.height());
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(device_->get_vulkan_device()));
        if (swapchain_)
            swapchain_procedures_.destroy(device_->get_vulkan_device(), swapchain_, nullptr);

        if (surface_)
            vkDestroySurfaceKHR(instance_->get_vulkan_instance(), surface_, nullptr);

        for (size_t i = 0; i < image_count_; i++)
            vkDestroyImageView(device_->get_vulkan_device(), image_views_.at(i), nullptr);

        if (render_pass_)
            vkDestroyRenderPass(device_->get_vulkan_device(), render_pass_, nullptr);

        for (size_t i = 0; i < image_count_; i++)
            vkDestroyFramebuffer(device_->get_vulkan_device(), framebuffers_.at(i), nullptr);

        for (size_t i = 0; i < frames_.size(); i++)
        {
            auto& frame = frames_.at(i);
            vkDestroyCommandPool(device_->get_vulkan_device(), frame.cmd_pool, nullptr);
            vkDestroySemaphore(device_->get_vulkan_device(), frame.image_available_semaphore, nullptr);
            vkDestroySemaphore(device_->get_vulkan_device(), frame.render_complete_semaphore, nullptr);
            vkDestroyFence(device_->get_vulkan_device(), frame.wait_fence, nullptr);
        }

        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(device_->get_vulkan_device()));
    }

    void VulkanSwapchain::begin_frame()
    {
        constexpr auto default_timeout = std::numeric_limits<uint64_t>::max();
        auto& frame = frames_.at(frame_index_);

        Stopwatch wait_stopwatch{};
        DODO_VERIFY_VK_RESULT(vkWaitForFences(
            device_->get_vulkan_device(), 1, &frame.wait_fence, VK_TRUE, default_timeout));

        performance_stats_.gpu_wait_time = wait_stopwatch.milliseconds();
        VkResult result = acquire_next_image_for(frame, &image_index_);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swapchain(width_, height_);
            return;
        }
        else if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR))
        {
            DODO_VERIFY_VK_RESULT(result);
        }

        // Only reset the fence when submitting work to
        // avoid a deadlock.
        // Online: https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation
        DODO_VERIFY_VK_RESULT(vkResetFences(device_->get_vulkan_device(), 1, &frame.wait_fence));
        DODO_VERIFY_VK_RESULT(vkResetCommandBuffer(frame.cmd_buffer, 0));

        // TODO: remove this.
        RecordTestCommands();
    }

    void VulkanSwapchain::end_frame()
    {
        auto& frame = frames_.at(frame_index_);
        const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &frame.image_available_semaphore;
        submit_info.pWaitDstStageMask = waitStages;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &frame.render_complete_semaphore;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &frame.cmd_buffer;
        DODO_VERIFY_VK_RESULT(vkQueueSubmit(
            device_->vulkan_graphics_queue(), 1, &submit_info, frame.wait_fence));

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &frame.render_complete_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain_;
        present_info.pImageIndices = &image_index_;
        const VkResult result = swapchain_procedures_.present(device_->vulkan_graphics_queue(), &present_info);
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR) || needs_resize_)
        {
            needs_resize_ = false;
            recreate_swapchain(width_, height_);
        }
        else
        {
            DODO_VERIFY_VK_RESULT(result);
        }

        frame_index_ = (frame_index_ + 1) % frames_in_flight;
    }

    void VulkanSwapchain::on_resize(uint32_t width, uint32_t height)
    {
        width_ = width;
        height_ = height;
        needs_resize_ = true;
    }

    void VulkanSwapchain::load_surface_and_swapchain_procedures()
    {
        surface_procedures_.get_support =
            instance_->fetch_procedure<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(
                "vkGetPhysicalDeviceSurfaceSupportKHR");

        surface_procedures_.get_capabilities =
            instance_->fetch_procedure<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(
                "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

        surface_procedures_.get_formats =
            instance_->fetch_procedure<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(
                "vkGetPhysicalDeviceSurfaceFormatsKHR");

        surface_procedures_.get_present_modes =
            instance_->fetch_procedure<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(
                "vkGetPhysicalDeviceSurfacePresentModesKHR");

        swapchain_procedures_.create =
            device_->load_procedure<PFN_vkCreateSwapchainKHR>(
                "vkCreateSwapchainKHR");

        swapchain_procedures_.get_images =
            device_->load_procedure<PFN_vkGetSwapchainImagesKHR>(
                "vkGetSwapchainImagesKHR");

        swapchain_procedures_.acquire_next_image =
            device_->load_procedure<PFN_vkAcquireNextImageKHR>(
                "vkAcquireNextImageKHR");

        swapchain_procedures_.present =
            device_->load_procedure<PFN_vkQueuePresentKHR>(
                "vkQueuePresentKHR");

        swapchain_procedures_.destroy =
            device_->load_procedure<PFN_vkDestroySwapchainKHR>(
                "vkDestroySwapchainKHR");
    }

    std::optional<uint32_t> VulkanSwapchain::find_present_queue_family_index() const
    {
        auto adapter = device_->adapter();
        const auto graphics_queue_index = adapter->graphics_queue_index();
        VkBool32 supports_present = VK_FALSE;
        surface_procedures_.get_support(
            adapter->get_vulkan_physical_device(), graphics_queue_index, surface_, &supports_present);

        // This is ideal scenario. Always prefer queue that
        // supports graphics and present!
        if (supports_present)
            return graphics_queue_index;

        const auto& queue_families = adapter->supported_vulkan_queue_families();
        for (uint32_t queue_index = 0; queue_index < queue_families.size(); queue_index++)
        {
            // Waste of time, because we already tried the
            // graphics queue!
            if (queue_index == graphics_queue_index)
                continue;

            surface_procedures_.get_support(
                adapter->get_vulkan_physical_device(), queue_index, surface_, &supports_present);

            if (supports_present)
                return queue_index;
        }

        return {};
    }

    void VulkanSwapchain::recreate_swapchain(uint32_t width, uint32_t height)
    {
        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(device_->get_vulkan_device()));
        create_swapchain(width, height);
        DODO_VERIFY_VK_RESULT(vkDeviceWaitIdle(device_->get_vulkan_device()));
    }

    void VulkanSwapchain::create_swapchain(uint32_t width, uint32_t height)
    {
        width_ = width;
        height_ = height;

        const auto support_details = query_support_details();
        const auto surface_format  = select_surface_format(support_details);
        const auto present_mode    = select_present_mode(support_details);
        const auto capabilities    = support_details.capabilities;

        extent_ = capabilities.currentExtent;
        if (extent_.width == std::numeric_limits<uint32_t>::max())
        {
            extent_.width  = std::clamp(width_ , capabilities.minImageExtent.width , capabilities.maxImageExtent.width );
            extent_.height = std::clamp(height_, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        const auto is_extent_compatible = (extent_.width != 0) && (extent_.height != 0);
        if (!is_extent_compatible)
            return;

        // To avoid waiting on driver to complete operations.
        image_count_ = capabilities.minImageCount + 1;
        const auto image_count_exceeds_max_images =
            (capabilities.maxImageCount > 0) && (image_count_ > capabilities.maxImageCount);

        if (image_count_exceeds_max_images)
            image_count_ = capabilities.maxImageCount;

        VkSwapchainKHR old_swapchain = swapchain_;
        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = surface_;
        swapchain_create_info.minImageCount = image_count_;
        swapchain_create_info.imageFormat = surface_format.format;
        swapchain_create_info.imageColorSpace = surface_format.colorSpace;
        swapchain_create_info.imageExtent = extent_;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // Images can be used across multiple queue families
        // without explicit ownership transfers.
        // Online: https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
        const auto graphics_queue_index = device_->adapter()->graphics_queue_index();
        if (graphics_queue_index != present_queue_index_.value())
        {
            std::array<uint32_t, 2> queue_family_indices = { graphics_queue_index, present_queue_index_.value() };
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = (uint32_t)queue_family_indices.size();
            swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();
        }

        swapchain_create_info.preTransform = capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = present_mode;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = old_swapchain;
        DODO_VERIFY_VK_RESULT(swapchain_procedures_.create(
            device_->get_vulkan_device(), &swapchain_create_info, VK_NULL_HANDLE, &swapchain_));

        if (old_swapchain)
            swapchain_procedures_.destroy(
                device_->get_vulkan_device(), old_swapchain, VK_NULL_HANDLE);

        for (auto image_view : image_views_)
            vkDestroyImageView(device_->get_vulkan_device(), image_view, VK_NULL_HANDLE);

        images_.clear();
        image_views_.clear();
        swapchain_procedures_.get_images(device_->get_vulkan_device(), swapchain_, &image_count_, VK_NULL_HANDLE);
        images_.resize(image_count_);
        image_views_.resize(image_count_);
        swapchain_procedures_.get_images(device_->get_vulkan_device(), swapchain_, &image_count_, images_.data());
        for (size_t i = 0; i < image_count_; i++)
        {
            VkImageViewCreateInfo image_view_create_info{};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = images_.at(i);
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = surface_format.format;
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            DODO_VERIFY_VK_RESULT(vkCreateImageView(
                device_->get_vulkan_device(), &image_view_create_info, VK_NULL_HANDLE, &image_views_.at(i)));
        }

        if (render_pass_)
            vkDestroyRenderPass(device_->get_vulkan_device(), render_pass_, VK_NULL_HANDLE);

        VkAttachmentDescription color_attachment{};
        color_attachment.format = surface_format.format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_reference{};
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_reference;

        VkSubpassDependency subpass_dependency{};
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dependency.dstSubpass = 0;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcAccessMask = 0;
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = 1;
        render_pass_create_info.pAttachments = &color_attachment;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 1;
        render_pass_create_info.pDependencies = &subpass_dependency;
        DODO_VERIFY_VK_RESULT(vkCreateRenderPass(
            device_->get_vulkan_device(), &render_pass_create_info, VK_NULL_HANDLE, &render_pass_));

        for (auto framebuffer : framebuffers_)
            vkDestroyFramebuffer(device_->get_vulkan_device(), framebuffer, VK_NULL_HANDLE);

        framebuffers_.clear();
        framebuffers_.resize(image_count_);
        for (size_t i = 0; i < image_count_; i++)
        {
            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass = render_pass_;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = &image_views_.at(i);
            framebuffer_create_info.width = extent_.width;
            framebuffer_create_info.height = extent_.height;
            framebuffer_create_info.layers = 1;
            DODO_VERIFY_VK_RESULT(vkCreateFramebuffer(
                device_->get_vulkan_device(), &framebuffer_create_info, VK_NULL_HANDLE, &framebuffers_.at(i)));
        }

        for (auto& frame : frames_)
        {
            vkDestroyCommandPool(device_->get_vulkan_device(), frame.cmd_pool, VK_NULL_HANDLE);
            vkDestroySemaphore(device_->get_vulkan_device(), frame.image_available_semaphore, VK_NULL_HANDLE);
            vkDestroySemaphore(device_->get_vulkan_device(), frame.render_complete_semaphore, VK_NULL_HANDLE);
            vkDestroyFence(device_->get_vulkan_device(), frame.wait_fence, VK_NULL_HANDLE);
        }

        frames_.clear();
        frames_.resize(frames_in_flight);
        for (size_t i = 0; i < frames_in_flight; i++)
        {
            Frame& frame = frames_.at(i);
            VkCommandPoolCreateInfo cmd_pool_create_info{};
            cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            cmd_pool_create_info.queueFamilyIndex = graphics_queue_index;
            DODO_VERIFY_VK_RESULT(vkCreateCommandPool(
                device_->get_vulkan_device(), &cmd_pool_create_info, VK_NULL_HANDLE, &frame.cmd_pool));

            VkCommandBufferAllocateInfo cmd_buffer_allocate_info{};
            cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmd_buffer_allocate_info.commandPool = frame.cmd_pool;
            cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmd_buffer_allocate_info.commandBufferCount = 1;
            DODO_VERIFY_VK_RESULT(vkAllocateCommandBuffers(
                device_->get_vulkan_device(), &cmd_buffer_allocate_info, &frame.cmd_buffer));

            VkSemaphoreCreateInfo semaphore_create_info{};
            semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            DODO_VERIFY_VK_RESULT(vkCreateSemaphore(
                device_->get_vulkan_device(), &semaphore_create_info, VK_NULL_HANDLE, &frame.image_available_semaphore));

            DODO_VERIFY_VK_RESULT(vkCreateSemaphore(
                device_->get_vulkan_device(), &semaphore_create_info, VK_NULL_HANDLE, &frame.render_complete_semaphore));

            VkFenceCreateInfo fence_create_info{};
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            DODO_VERIFY_VK_RESULT(vkCreateFence(
                device_->get_vulkan_device(), &fence_create_info, VK_NULL_HANDLE, &frame.wait_fence));
        }
    }

    VulkanSwapchain::SupportDetails VulkanSwapchain::query_support_details() const
    {
        const auto physical_device = device_->adapter()->get_vulkan_physical_device();
        SupportDetails support_details{};
        surface_procedures_.get_capabilities(
            physical_device, surface_, &support_details.capabilities);

        uint32_t format_count = 0;
        surface_procedures_.get_formats(physical_device, surface_, &format_count, VK_NULL_HANDLE);
        if (format_count > 0)
        {
            support_details.formats.resize(format_count);
            surface_procedures_.get_formats(
                physical_device, surface_, &format_count, support_details.formats.data());
        }

        uint32_t preset_mode_count = 0;
        surface_procedures_.get_present_modes(physical_device, surface_, &preset_mode_count, VK_NULL_HANDLE);
        if (preset_mode_count > 0)
        {
            support_details.present_modes.resize(preset_mode_count);
            surface_procedures_.get_present_modes(
                physical_device, surface_, &preset_mode_count, support_details.present_modes.data());
        }

        return support_details;
    }

    VkSurfaceFormatKHR VulkanSwapchain::select_surface_format(const SupportDetails& support_details) const
    {
        VkSurfaceFormatKHR target_format{};
        target_format.format = VK_FORMAT_B8G8R8A8_SRGB;
        target_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        for (const auto& format : support_details.formats)
            if (format.format == target_format.format
                    && format.colorSpace == target_format.colorSpace)
                return format;

        return support_details.formats.back();
    }

    VkPresentModeKHR VulkanSwapchain::select_present_mode(const SupportDetails& support_details) const
    {
        const auto target_present_mode = Utils::to_vulkan_present_mode(vsync_mode_);
        for (const auto present_mode : support_details.present_modes)
            if (present_mode == target_present_mode)
                return present_mode;

        return support_details.present_modes.back();
    }

    VkResult VulkanSwapchain::acquire_next_image_for(const Frame& frame, uint32_t* image_index)
    {
        constexpr auto default_timeout = std::numeric_limits<uint64_t>::max();
        return swapchain_procedures_.acquire_next_image(
            device_->get_vulkan_device(),
            swapchain_,
            default_timeout,
            frame.image_available_semaphore,
            VK_NULL_HANDLE,
            image_index);
    }

    void VulkanSwapchain::RecordTestCommands()
    {
        auto& frame = frames_.at(frame_index_);

        VkCommandBufferBeginInfo cmd_buffer_begin_info{};
        cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        DODO_VERIFY_VK_RESULT(vkBeginCommandBuffer(frame.cmd_buffer, &cmd_buffer_begin_info));

        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = render_pass_;
        render_pass_begin_info.framebuffer = framebuffers_.at(image_index_);
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = extent_;

        VkClearValue clear_color = {{{ 1.0f, 0.3f, 0.15f, 1.0f }}};
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues    = &clear_color;
        vkCmdBeginRenderPass(frame.cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)extent_.width;
        viewport.height = (float)extent_.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(frame.cmd_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent_;
        vkCmdSetScissor(frame.cmd_buffer, 0, 1, &scissor);
        vkCmdEndRenderPass(frame.cmd_buffer);
        DODO_VERIFY_VK_RESULT(vkEndCommandBuffer(frame.cmd_buffer));
    }

}