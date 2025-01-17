#include "pch.h"

#ifdef DODO_VULKAN

#include "render_device_vulkan.h"
#include "render_backend_vulkan.h"

namespace Dodo {

    namespace Utils {

        static VkPresentModeKHR convert_to_present_mode(RenderBackend::VSyncMode vsync_mode) {
            switch (vsync_mode) {
                case RenderBackend::VSyncMode::enabled : return VK_PRESENT_MODE_FIFO_KHR;
                case RenderBackend::VSyncMode::mailbox : return VK_PRESENT_MODE_MAILBOX_KHR;
                case RenderBackend::VSyncMode::disabled: return VK_PRESENT_MODE_IMMEDIATE_KHR;
                default: break;
            }

            DODO_ASSERT(false);
            return VK_PRESENT_MODE_MAX_ENUM_KHR;
        }

    }

    RenderDeviceVulkan::RenderDeviceVulkan(Ref<RenderBackendVulkan> backend) {
        DODO_ASSERT(backend);
        _backend = backend;
    }

    void RenderDeviceVulkan::initialize(size_t index) {
        _physical_device = _backend->physical_device_get(index);

        _queue_families.clear();
        const uint32_t queue_family_count = _backend->queue_family_get_count(index);
        _queue_families.resize(queue_family_count);
        for (uint32_t i = 0; i < queue_family_count; i++) {
            _queue_families.at(i) = _backend->queue_family_get(index, i);
        }

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
        _add_queue_create_infos(queue_create_infos);
        _initialize_device(queue_create_infos);
    }

    void RenderDeviceVulkan::_add_queue_create_infos(std::vector<VkDeviceQueueCreateInfo>& queue_create_infos) {
        _queues.clear();
        const size_t queue_family_count = _queue_families.size();
        _queues.resize(queue_family_count);

        const VkQueueFlags desired_queue_families = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
        for (uint32_t i = 0; i < queue_family_count; i++) {
            if ((_queue_families.at(i).queueFlags & desired_queue_families) == 0) {
                continue;
            }

            const uint32_t max_queue_count_per_family = 1;
            const uint32_t queue_count = std::min(_queue_families.at(i).queueCount, max_queue_count_per_family);
            static const float queue_priority = 1.0f;

            VkDeviceQueueCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_info.queueFamilyIndex = i;
            create_info.queueCount = queue_count;
            create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(std::move(create_info));

            _queues.at(i).resize(queue_count);
        }
    }

    void RenderDeviceVulkan::_initialize_device(std::vector<VkDeviceQueueCreateInfo>& queue_create_infos) {
        std::set<std::string> supported_extensions = {};
        uint32_t extension_count = 0;
        DODO_ASSERT_VK_RESULT(vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &extension_count, nullptr));
        std::vector<VkExtensionProperties> extensions(extension_count);
        DODO_ASSERT_VK_RESULT(vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &extension_count, extensions.data()));
        for (const auto& extension : extensions) {
            supported_extensions.insert(extension.extensionName);
        }

        _request_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);

        for (const auto& [name, is_required] : _requested_extensions) {
            if (!supported_extensions.contains(name)) {
                if (is_required) {
                    DODO_LOG_ERROR_TAG("Renderer", "{0} required but not supported!", name);
                    DODO_ASSERT(false);
                }
                else {
                    DODO_LOG_WARNING_TAG("Renderer", "{0} not supported!", name);
                    continue;
                }
            }

            _enabled_extensions.push_back(name.c_str());
        }

        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(_enabled_extensions.size());
        device_create_info.ppEnabledExtensionNames = _enabled_extensions.data();
        DODO_ASSERT_VK_RESULT(vkCreateDevice(_physical_device, &device_create_info, nullptr, &_device));

        for (uint32_t i = 0; i < _queues.size(); i++) {
            for (uint32_t j = 0; j < _queues.at(i).size(); j++) {
                vkGetDeviceQueue(_device, i, j, &_queues.at(i).at(j));
            }
        }

        const RenderBackendVulkan::Functions& backend_functions = _backend->functions_get();
        _functions.CreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(backend_functions.GetDeviceProcAddr(_device, "vkCreateSwapchainKHR"));
        _functions.GetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(backend_functions.GetDeviceProcAddr(_device, "vkGetSwapchainImagesKHR"));
        _functions.AcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(backend_functions.GetDeviceProcAddr(_device, "vkAcquireNextImageKHR"));
        _functions.QueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(backend_functions.GetDeviceProcAddr(_device, "vkQueuePresentKHR"));
        _functions.DestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(backend_functions.GetDeviceProcAddr(_device, "vkDestroySwapchainKHR"));
    }

    void RenderDeviceVulkan::_request_extension(const std::string& name, bool is_required) {
        DODO_ASSERT(!_requested_extensions.contains(name));
        _requested_extensions.insert({ name, is_required });
    }

    CommandQueueFamilyHandle RenderDeviceVulkan::command_queue_family_get(CommandQueueFamilyType command_queue_family_type, SurfaceHandle surface) {
        VkQueueFlags desired_queue_family_bits = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
        if (command_queue_family_type == COMMAND_QUEUE_FAMILY_TYPE_COMPUTE) {
            desired_queue_family_bits = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
        }
        else if (command_queue_family_type == COMMAND_QUEUE_FAMILY_TYPE_TRANSFER) {
            desired_queue_family_bits = VK_QUEUE_TRANSFER_BIT;
        }

        VkQueueFlags picked_queue_family_bits = VK_QUEUE_FLAG_BITS_MAX_ENUM;
        uint32_t picked_queue_family_index = std::numeric_limits<uint32_t>::max();
        for (uint32_t i = 0; i < _queues.size(); i++) {
            if (_queues.at(i).empty()) {
                continue;
            }

            if (surface && !_backend->queue_family_supports_present(_physical_device, i, surface)) {
                continue;
            }

            const VkQueueFlags queue_family_bits = _queue_families.at(i).queueFlags;
            const bool includes_all_bits = (queue_family_bits & desired_queue_family_bits) == desired_queue_family_bits;
            const bool has_less_bits = queue_family_bits < picked_queue_family_bits;
            if (includes_all_bits && has_less_bits) {
                picked_queue_family_bits = queue_family_bits;
                picked_queue_family_index = i;
            }
        }

        return CommandQueueFamilyHandle(picked_queue_family_index + 1);
    }

    CommandQueueHandle RenderDeviceVulkan::command_queue_create(CommandQueueFamilyHandle command_queue_family) {
        DODO_ASSERT(!command_queue_family.is_null());
        CommandQueue command_queue_info = {};
        command_queue_info.queue_family_index = command_queue_family.get_id() - 1;
        return _command_queue_owner.create(command_queue_info);
    }

    bool RenderDeviceVulkan::command_queue_execute_and_present(CommandQueueHandle p_command_queue, const std::vector<SemaphoreHandle>& p_wait_semaphores, const std::vector<CommandBufferHandle>& p_command_buffers, const std::vector<SemaphoreHandle>& p_signal_semaphores, FenceHandle p_fence, SwapChainHandle p_swap_chain) {
        DODO_ASSERT(!p_command_queue.is_null());
        CommandQueue* command_queue = _command_queue_owner.get_or_null(p_command_queue);
        if (!command_queue) {
            return false;
        }

        std::vector<VkSemaphore> wait_semaphores = {};
        std::vector<VkPipelineStageFlags> wait_stages = {};
        for (uint32_t i = 0; i < command_queue->pending_semaphores_for_execute.size(); i++) {
            const uint32_t semaphore_index = command_queue->pending_semaphores_for_execute.at(i);
            VkSemaphore wait_semaphore = command_queue->image_semaphores.at(semaphore_index);
            wait_semaphores.push_back(wait_semaphore);
            wait_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }

        command_queue->pending_semaphores_for_execute.clear();

        for (uint32_t i = 0; i < p_wait_semaphores.size(); i++) {
            VkSemaphore* wait_semaphore = _semaphore_owner.get_or_null(p_wait_semaphores.at(i));
            if (wait_semaphore) {
                wait_semaphores.push_back(*wait_semaphore);
                wait_stages.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
            }
        }

        std::vector<VkCommandBuffer> command_buffers = {};
        for (uint32_t i = 0; i < p_command_buffers.size(); i++) {
            CommandBuffer* command_buffer = _command_buffer_owner.get_or_null(p_command_buffers.at(i));
            if (command_buffer) {
                command_buffers.push_back(command_buffer->vk_command_buffer);
            }
        }

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
		submit_info.pWaitSemaphores = wait_semaphores.data();
		submit_info.pWaitDstStageMask = wait_stages.data();
		submit_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
		submit_info.pCommandBuffers = command_buffers.data();
		submit_info.signalSemaphoreCount = signal_semaphores.size();
		submit_info.pSignalSemaphores = signal_semaphores.ptr();

        VkQueue device_queue = _queues.at(command_queue->queue_family_index).at(command_queue->queue_index);

    }

    void RenderDeviceVulkan::command_queue_destroy(CommandQueueHandle command_queue) {
        DODO_ASSERT(!command_queue.is_null());
        _command_queue_owner.destroy(command_queue);
    }

    CommandPoolHandle RenderDeviceVulkan::command_pool_create(CommandQueueFamilyHandle command_queue_family) {
        DODO_ASSERT(!command_queue_family.is_null());
        VkCommandPoolCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        create_info.queueFamilyIndex = command_queue_family.get_id() - 1;
        VkCommandPool vk_command_pool = VK_NULL_HANDLE;
        DODO_ASSERT_VK_RESULT(vkCreateCommandPool(_device, &create_info, VK_NULL_HANDLE, &vk_command_pool));
        return _command_pool_owner.create(vk_command_pool);
    }

    void RenderDeviceVulkan::command_pool_destroy(CommandPoolHandle command_pool) {
        DODO_ASSERT(!command_pool.is_null());
        if (VkCommandPool* vk_command_pool = _command_pool_owner.get_or_null(command_pool)) {
            vkDestroyCommandPool(_device, vk_command_pool, VK_NULL_HANDLE);
            _command_pool_owner.destroy(command_pool);
        }
    }

    CommandBufferHandle RenderDeviceVulkan::command_buffer_create(CommandPoolHandle command_pool, CommandBufferType command_buffer_type) {
        DODO_ASSERT(!command_pool.is_null());
        VkCommandPool* vk_command_pool = _command_pool_owner.get_or_null(command_pool);
        if (!vk_command_pool) {
            return CommandBufferHandle();
        }

        VkCommandBufferLevel command_buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        if (command_buffer_type == COMMAND_BUFFER_TYPE_SECONDARY) {
            command_buffer_level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        }

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = vk_command_pool;
        alloc_info.level = command_buffer_level;
        alloc_info.commandBufferCount = 1;
        VkCommandBuffer vk_command_buffer = VK_NULL_HANDLE;
        DODO_ASSERT_VK_RESULT(vkAllocateCommandBuffers(_device, &alloc_info, &vk_command_buffer));

        CommandBuffer command_buffer_info = {};
        command_buffer_info.command_buffer_type = command_buffer_type;
        command_buffer_info.vk_command_buffer = vk_command_buffer;
        return _command_buffer_owner.create(command_buffer_info);
    }

    void RenderDeviceVulkan::command_buffer_begin(CommandBufferHandle command_buffer) {
        DODO_ASSERT(!command_buffer.is_null());
        if (CommandBuffer* command_buffer_info = _command_buffer_owner.get_or_null(command_buffer)) {
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            DODO_ASSERT_VK_RESULT(vkBeginCommandBuffer(command_buffer_info->vk_command_buffer, &begin_info));
        }
    }

    void RenderDeviceVulkan::command_buffer_end(CommandBufferHandle command_buffer) {
        DODO_ASSERT(!command_buffer.is_null());
        if (CommandBuffer* command_buffer_info = _command_buffer_owner.get_or_null(command_buffer)) {
            DODO_ASSERT_VK_RESULT(vkEndCommandBuffer(command_buffer_info->vk_command_buffer));
        }
    }

    FenceHandle RenderDeviceVulkan::fence_create() {
        VkFenceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkFence vk_fence  = VK_NULL_HANDLE;
        DODO_ASSERT_VK_RESULT(vkCreateFence(_device, &create_info, VK_NULL_HANDLE, &vk_fence));

        Fence fence = {};
        fence.vk_fence = vk_fence;
        return _fence_owner.create(fence);
    }

    void RenderDeviceVulkan::fence_wait(FenceHandle p_fence) {
        DODO_ASSERT(!p_fence.is_null());
        Fence* fence = _fence_owner.get_or_null(p_fence);
        if (!fence) {
            return;
        }

        static const auto default_timeout = std::numeric_limits<uint64_t>::max();
        DODO_ASSERT_VK_RESULT(vkWaitForFences(_device, 1, &fence->vk_fence, VK_TRUE, default_timeout));
        DODO_ASSERT_VK_RESULT(vkResetFences(_device, 1, &fence->vk_fence));
        if (!fence->command_queue_to_signal || fence->command_queue_to_signal->image_semaphores_for_fences.empty()) {
            return;
        }

        // After the fence has signaled, that the presentation engine is
        // done, we have to free the used image semaphore associated
        // with that fence.
        CommandQueue* command_queue = fence->command_queue_to_signal;
        uint32_t i = 0;
        while (i < command_queue->image_semaphores_for_fences.size()) {
            const std::pair<Fence*, uint32_t>& semaphore_for_fence = command_queue->image_semaphores_for_fences.at(i);
            if (fence == semaphore_for_fence.first) {
                command_queue->free_image_semaphores.push_back(semaphore_for_fence.second);
                command_queue->image_semaphores_for_fences.erase(command_queue->image_semaphores_for_fences.begin() + i);
            }
            else {
                i++;
            }
        }

        fence->command_queue_to_signal = nullptr;
    }

    void RenderDeviceVulkan::fence_destroy(FenceHandle fence) {
        DODO_ASSERT(!fence.is_null());
        if (VkFence* vk_fence = _fence_owner.get_or_null(fence)) {
            vkDestroyFence(_device, vk_fence, VK_NULL_HANDLE);
            _fence_owner.destroy(fence);
        }
    }

    SemaphoreHandle RenderDeviceVulkan::semaphore_create() {
        VkSemaphoreCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore vk_semaphore = VK_NULL_HANDLE;
        DODO_ASSERT_VK_RESULT(vkCreateSemaphore(_device, &create_info, VK_NULL_HANDLE, &vk_semaphore));
        return _semaphore_owner.create(vk_semaphore);
    }

    void RenderDeviceVulkan::semaphore_destroy(SemaphoreHandle semaphore) {
        DODO_ASSERT(!semaphore.is_null());
        if (VkSemaphore* vk_semaphore = _semaphore_owner.get_or_null(semaphore)) {
            vkDestroySemaphore(_device, vk_semaphore, VK_NULL_HANDLE);
            _semaphore_owner.destroy(semaphore);
        }
    }

    SwapChainHandle RenderDeviceVulkan::swap_chain_create(SurfaceHandle p_surface) {
        DODO_ASSERT(!p_surface.is_null());
        const RenderBackendVulkan::Functions& backend_functions = _backend->functions_get();
        RenderBackendVulkan::Surface* surface = _backend->surface_get(p_surface);
        if (!surface) {
            return SwapChainHandle();
        }

        uint32_t format_count = 0;
        DODO_ASSERT_VK_RESULT(backend_functions.GetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface->vk_surface, &format_count, nullptr));
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        DODO_ASSERT_VK_RESULT(backend_functions.GetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface->vk_surface, &format_count, formats.data()));

        const VkFormat desired_format = VK_FORMAT_B8G8R8A8_SRGB;
        const VkColorSpaceKHR desired_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkFormat picked_format = formats.back().format;
        VkColorSpaceKHR picked_color_space = formats.back().colorSpace;
        for (const VkSurfaceFormatKHR& format : formats) {
            if (format.format == desired_format && format.colorSpace == desired_color_space) {
                picked_format = format.format;
                picked_color_space = format.colorSpace;
                break;
            }
        }

        VkAttachmentDescription color_attachment = {};
        color_attachment.format = picked_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        VkRenderPassCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &color_attachment;
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        VkRenderPass render_pass = VK_NULL_HANDLE;
        DODO_ASSERT_VK_RESULT(vkCreateRenderPass(_device, &create_info, nullptr, &render_pass));

        SwapChain swap_chain_info = {};
        swap_chain_info.surface = p_surface;
        swap_chain_info.format = picked_format;
        swap_chain_info.color_space = picked_color_space;
        swap_chain_info.render_pass = render_pass;
        return _swap_chain_owner.create(swap_chain_info);
    }

    FramebufferHandle RenderDeviceVulkan::swap_chain_acquire_next_framebuffer(CommandQueueHandle p_command_queue, SwapChainHandle p_swap_chain, SwapChainStatus& r_swap_chain_status) {
        DODO_ASSERT(!p_command_queue.is_null());
        DODO_ASSERT(!p_swap_chain.is_null());
        CommandQueue* command_queue = _command_queue_owner.get_or_null(p_command_queue);
        SwapChain* swap_chain = _swap_chain_owner.get_or_null(p_swap_chain);
        if (!command_queue || !swap_chain) {
            r_swap_chain_status = SwapChainStatus::error;
            return FramebufferHandle();
        }

        if ((swap_chain_info->vk_swap_chain == VK_NULL_HANDLE) && _backend->surface_get_needs_resize(swap_chain_info->surface)) {
            r_swap_chain_status = SwapChainStatus::out_of_date;
            return FramebufferHandle();
        }

        uint32_t semaphore_index = 0;
        if (!command_queue->free_image_semaphores.empty()) {
            semaphore_index = command_queue->free_image_semaphores.back();
            command_queue->free_image_semaphores.pop_back();
        }
        else {
            VkSemaphoreCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VkSemaphore semaphore = VK_NULL_HANDLE;
            DODO_ASSERT_VK_RESULT(vkCreateSemaphore(_device, &create_info, VK_NULL_HANDLE, semaphore));
            semaphore_index = command_queue->image_semaphores.size();
            command_queue->image_semaphores.push_back(semaphore);
        }

        static const auto default_timeout = std::numeric_limits<uint64_t>::max();
        const VkResult result = _functions.AcquireNextImageKHR(_device, swap_chain->vk_swap_chain, default_timeout, command_queue->image_semaphores.at(semaphore_index), VK_NULL_HANDLE, &swap_chain->image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            r_swap_chain_status = SwapChainStatus::out_of_date;
            return FramebufferHandle();
        }

        // Suboptimal swap chain can still be used to successfully present
        // to the surface.
        r_swap_chain_status = SwapChainStatus::success;
        if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR)) {
            r_swap_chain_status = SwapChainStatus::error;
            return FramebufferHandle();
        }

        command_queue->pending_semaphores_for_execute.push_back(semaphore_index);
        return FramebufferHandle(swap_chain->framebuffers.at(swap_chain->image_index));
    }

    void RenderDeviceVulkan::swap_chain_resize(CommandQueueHandle p_command_queue, SwapChainHandle p_swap_chain, uint32_t p_desired_framebuffer_count) {
        DODO_ASSERT(!p_swap_chain.is_null());
        SwapChain* swap_chain = _swap_chain_owner.get_or_null(p_swap_chain);
        if (!swap_chain) {
            return;
        }

        _swap_chain_release(swap_chain);

        if (!_backend->queue_family_supports_present(_physical_device, command_queue->queue_family_index, swap_chain->surface_handle)) {
            DODO_ASSERT(false);
            DODO_LOG_ERROR_TAG("Renderer", "Surface not supported by device!");
        }

        const RenderContextVulkan::Functions& context_functions = _backend->functions_get();
        auto surface = swap_chain->surface_handle.cast_to<RenderContextVulkan::Surface*>();
        VkSurfaceCapabilitiesKHR surface_caps = {};
        DODO_ASSERT_VK_RESULT(context_functions.GetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, surface->surface_vk, &surface_caps));

        uint32_t picked_image_count = std::max(p_desired_framebuffer_count, surface_caps.minImageCount + 1);
        // A max image count of 0 means we can have any number of images.
        if (surface_caps.maxImageCount > 0) {
            picked_image_count = std::min(picked_image_count, surface_caps.maxImageCount);
        }

        VkExtent2D extent = surface_caps.currentExtent;
        if (extent.width == std::numeric_limits<uint32_t>::max()) {
            extent.width  = std::clamp(surface->width , surface_caps.minImageExtent.width , surface_caps.maxImageExtent.width );
            extent.height = std::clamp(surface->height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height);
        }

        if ((extent.width == 0) || (extent.height == 0)) {
            // We cannot create the swap chain, since the surface has no valid extent.
            return;
        }

        uint32_t present_mode_count = 0;
        context_functions.GetPhysicalDeviceSurfacePresentModesKHR(_physical_device, surface->surface_vk, &present_mode_count, nullptr);
        std::vector<VkPresentModeKHR> present_modes(present_mode_count);
        context_functions.GetPhysicalDeviceSurfacePresentModesKHR(_physical_device, surface->surface_vk, &present_mode_count, present_modes.data());
        const VkPresentModeKHR desired_present_mode = Utils::convert_to_present_mode(surface->vsync_mode);
        VkPresentModeKHR picked_present_mode = VK_PRESENT_MODE_FIFO_KHR;
        for (size_t i = 0; i < present_modes.size(); i++) {
            if (present_modes.at(i) == desired_present_mode) {
                picked_present_mode = present_modes.at(i);
                break;
            }
        }

        VkSwapchainCreateInfoKHR swap_chain_create_info = {};
        swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_chain_create_info.surface = surface->surface_vk;
        swap_chain_create_info.minImageCount = picked_image_count;
        swap_chain_create_info.imageFormat = swap_chain->format;
        swap_chain_create_info.imageColorSpace = swap_chain->color_space;
        swap_chain_create_info.imageExtent = extent;
        swap_chain_create_info.imageArrayLayers = 1;
        swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_chain_create_info.preTransform = surface_caps.currentTransform;
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_chain_create_info.presentMode = picked_present_mode;
        swap_chain_create_info.clipped = true;
        DODO_ASSERT_VK_RESULT(_functions.CreateSwapchainKHR(_device, &swap_chain_create_info, nullptr, &swap_chain->vk_swap_chain));

        uint32_t image_count = 0;
        DODO_ASSERT_VK_RESULT(_functions.GetSwapchainImagesKHR(_device, swap_chain->vk_swap_chain, &image_count, nullptr));
        swap_chain->images.resize(image_count);
        DODO_ASSERT_VK_RESULT(_functions.GetSwapchainImagesKHR(_device, swap_chain->vk_swap_chain, &image_count, swap_chain->images.data()));
        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = swap_chain->format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        swap_chain->image_views.resize(image_count);
        for (size_t i = 0; i < swap_chain->image_views.size(); i++) {
            image_view_create_info.image = swap_chain->images.at(i);
            DODO_ASSERT_VK_RESULT(vkCreateImageView(_device, &image_view_create_info, nullptr, &swap_chain->image_views.at(i)));
        }

        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = swap_chain->render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.width = extent.width;
        framebuffer_create_info.height = extent.height;
        framebuffer_create_info.layers = 1;
        swap_chain->framebuffers.resize(image_count);
        for (size_t i = 0; i < swap_chain->framebuffers.size(); i++) {
            framebuffer_create_info.pAttachments = &swap_chain->image_views.at(i);
            DODO_ASSERT_VK_RESULT(vkCreateFramebuffer(_device, &framebuffer_create_info, nullptr, &swap_chain->framebuffers.at(i)));
        }
    }

    void RenderDeviceVulkan::swap_chain_destroy(SwapChainHandle p_swap_chain) {
        DODO_ASSERT(!p_swap_chain.is_null());
        if (SwapChain* swap_chain = _swap_chain_owner.get_or_null(p_swap_chain)) {
            _swap_chain_release(swap_chain);
        }
    }

    void RenderDeviceVulkan::_swap_chain_release(SwapChain* r_swap_chain) const {
        for (size_t i = 0; i < r_swap_chain->framebuffers.size(); i++) {
            vkDestroyFramebuffer(_device, r_swap_chain->framebuffers.at(i), nullptr);
        }

        r_swap_chain->framebuffers.clear();
        for (size_t i = 0; i < r_swap_chain->image_views.size(); i++) {
            vkDestroyImageView(_device, r_swap_chain->image_views.at(i), nullptr);
        }

        r_swap_chain->image_views.clear();

        if (r_swap_chain->vk_swap_chain) {
            _functions.DestroySwapchainKHR(_device, r_swap_chain->vk_swap_chain, nullptr);
        }
    }

}

#endif
