#include "pch.h"
#include <limits>

#ifdef DODO_VULKAN

#include "renderer_vulkan.h"
#include "render_context_vulkan.h"

namespace Dodo {

    namespace Utils {

        static constexpr VkPresentModeKHR convert_to_present_mode(RenderContext::VSyncMode vsync_mode) {
            switch (vsync_mode) {
                case RenderContext::VSyncMode::disabled: return VK_PRESENT_MODE_IMMEDIATE_KHR;
                case RenderContext::VSyncMode::enabled : return VK_PRESENT_MODE_FIFO_KHR;
                case RenderContext::VSyncMode::mailbox : return VK_PRESENT_MODE_MAILBOX_KHR;
                default: break;
            }

            DODO_ASSERT(false);
            return VK_PRESENT_MODE_MAX_ENUM_KHR;
        }

    }

    RendererVulkan::RendererVulkan(Ref<RenderContextVulkan> context) {
        DODO_ASSERT(context);
        _context = context;
    }

    void RendererVulkan::initialize(size_t device_index) {
        _physical_device = _context->physical_device_get(device_index);

        _queue_families.clear();
        const uint32_t queue_family_count = _context->queue_family_get_count(device_index);
        _queue_families.resize(queue_family_count);
        for (size_t i = 0; i < queue_family_count; i++) {
            _queue_families.at(i) = _context->queue_family_get(device_index, i);
        }
       
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {};
        _add_queue_create_infos(queue_create_infos);
        _initialize_device(queue_create_infos);
    }

    void RendererVulkan::_add_queue_create_infos(std::vector<VkDeviceQueueCreateInfo>& queue_create_infos) {
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

    void RendererVulkan::_initialize_device(std::vector<VkDeviceQueueCreateInfo>& queue_create_infos) {
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

        const RenderContextVulkan::Functions& context_functions = _context->functions_get();
        _functions.CreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(context_functions.GetDeviceProcAddr(_device, "vkCreateSwapchainKHR"));
        _functions.GetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(context_functions.GetDeviceProcAddr(_device, "vkGetSwapchainImagesKHR"));
        _functions.AcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(context_functions.GetDeviceProcAddr(_device, "vkAcquireNextImageKHR"));
        _functions.QueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(context_functions.GetDeviceProcAddr(_device, "vkQueuePresentKHR"));
        _functions.DestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(context_functions.GetDeviceProcAddr(_device, "vkDestroySwapchainKHR"));
    }

    void RendererVulkan::_request_extension(const std::string& name, bool is_required) {
        DODO_ASSERT(!_requested_extensions.contains(name));
        _requested_extensions.insert({ name, is_required });
    }

    CommandQueueFamilyHandle RendererVulkan::command_queue_family_get(CommandQueueFamilyType cmd_queue_family_type, SurfaceHandle surface_handle) {
        VkQueueFlags desired_queue_bits = VK_QUEUE_FLAG_BITS_MAX_ENUM;
        if (cmd_queue_family_type == CommandQueueFamilyType::draw) {
            desired_queue_bits = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
        }
        else if (cmd_queue_family_type == CommandQueueFamilyType::compute) {
            desired_queue_bits = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
        }
        else {
            desired_queue_bits = VK_QUEUE_TRANSFER_BIT;
        }

        VkQueueFlags picked_queue_bits = VK_QUEUE_FLAG_BITS_MAX_ENUM;
        uint32_t picked_queue_family_index = std::numeric_limits<uint32_t>::max();
        for (uint32_t i = 0; i < _queues.size(); i++) {
            if (_queues.at(i).empty()) {
                continue;
            }

            if (surface_handle && !_context->queue_family_supports_present(_physical_device, i, surface_handle)) {
                continue;
            }

            const VkQueueFlags queue_family_bits = _queue_families.at(i).queueFlags;
            const bool includes_all_bits = (queue_family_bits & desired_queue_bits) == desired_queue_bits;
            // Queue families that have less bits, like dedicated compute/transfer queue, will perform better.
            const bool has_less_bits = queue_family_bits < picked_queue_bits;
            if (includes_all_bits && has_less_bits) {
                picked_queue_bits = queue_family_bits;
                picked_queue_family_index = i;
            }
        }

        return CommandQueueFamilyHandle(picked_queue_family_index + 1);
    }

    CommandQueueHandle RendererVulkan::command_queue_create(CommandQueueFamilyHandle cmd_queue_family_handle) {
        DODO_ASSERT(cmd_queue_family_handle);
        auto command_queue = new CommandQueue();
        command_queue->queue_family_index = cmd_queue_family_handle.handle - 1;
        // Safely assume that the queue index is 0 as there is only 1 queue per family.
        // command_queue->queue_index = 0;
        return CommandQueueHandle(command_queue);
    }

    void RendererVulkan::command_queue_execute_and_present(CommandQueueHandle cmd_queue_handle, CommandListHandle* cmd_list_handles, SwapChainHandle* swap_chain_handles) {
        DODO_ASSERT(cmd_queue_handle);
    }

    void RendererVulkan::command_queue_destroy(CommandQueueHandle cmd_queue_handle) {
        DODO_ASSERT(cmd_queue_handle);
        auto command_queue = cmd_queue_handle.cast_to<CommandQueue*>();
        delete command_queue;
    }

    CommandListAllocatorHandle RendererVulkan::command_list_allocator_create(CommandQueueFamilyHandle cmd_queue_family_handle, CommandListType cmd_list_type) {
        DODO_ASSERT(cmd_queue_family_handle);
        const uint32_t queue_family_index = cmd_queue_family_handle.handle - 1;

        VkCommandPoolCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        create_info.queueFamilyIndex = queue_family_index;
        VkCommandPool command_pool = nullptr;
        DODO_ASSERT_VK_RESULT(vkCreateCommandPool(_device, &create_info, nullptr, &command_pool));

        auto command_list_allocator = new CommandListAllocator();
        command_list_allocator->command_pool_vk = command_pool;
        command_list_allocator->command_list_type = cmd_list_type;
        return CommandListAllocatorHandle(command_list_allocator);
    }

    void RendererVulkan::command_list_allocator_destroy(CommandListAllocatorHandle cmd_list_allocator_handle) {
        DODO_ASSERT(cmd_list_allocator_handle);
        auto command_list_allocator = cmd_list_allocator_handle.cast_to<CommandListAllocator*>();
        vkDestroyCommandPool(_device, command_list_allocator->command_pool_vk, nullptr);
        delete command_list_allocator;
    }

    CommandListHandle RendererVulkan::command_list_create(CommandListAllocatorHandle cmd_list_allocator_handle) {
        DODO_ASSERT(cmd_list_allocator_handle);
        auto command_list_allocator = cmd_list_allocator_handle.cast_to<CommandListAllocator*>();

        VkCommandBufferLevel command_buffer_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        if (command_list_allocator->command_list_type == CommandListType::secondary) {
            command_buffer_level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        }

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = command_list_allocator->command_pool_vk;
        alloc_info.level = command_buffer_level;
        alloc_info.commandBufferCount = 1;
        VkCommandBuffer command_buffer = nullptr;
        DODO_ASSERT_VK_RESULT(vkAllocateCommandBuffers(_device, &alloc_info, &command_buffer));
        return CommandListHandle(command_buffer);
    }

    void RendererVulkan::command_list_begin(CommandListHandle cmd_list_handle) {
        DODO_ASSERT(cmd_list_handle);
        auto command_buffer = cmd_list_handle.cast_to<VkCommandBuffer>();

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        DODO_ASSERT_VK_RESULT(vkBeginCommandBuffer(command_buffer, &begin_info));
    }

    void RendererVulkan::command_list_end(CommandListHandle cmd_list_handle) {
        DODO_ASSERT(cmd_list_handle);
        auto command_buffer = cmd_list_handle.cast_to<VkCommandBuffer>();

        DODO_ASSERT_VK_RESULT(vkEndCommandBuffer(command_buffer));
    }

    SwapChainHandle RendererVulkan::swap_chain_create(SurfaceHandle surface_handle) {
        DODO_ASSERT(surface_handle);

        const RenderContextVulkan::Functions& context_functions = _context->functions_get();
        auto surface = surface_handle.cast_to<RenderContextVulkan::Surface*>();

        uint32_t format_count = 0;
        DODO_ASSERT_VK_RESULT(context_functions.GetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface->surface_vk, &format_count, nullptr));
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        DODO_ASSERT_VK_RESULT(context_functions.GetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface->surface_vk, &format_count, formats.data()));

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

        auto swap_chain = new SwapChain();
        swap_chain->surface_handle = surface_handle;
        swap_chain->format = picked_format;
        swap_chain->color_space = picked_color_space;

        VkAttachmentDescription color_attachment = {};
        color_attachment.format = swap_chain->format;
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
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        VkRenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount = 1;
        render_pass_create_info.pAttachments = &color_attachment;
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        VkRenderPass render_pass = nullptr;
        DODO_ASSERT_VK_RESULT(vkCreateRenderPass(_device, &render_pass_create_info, nullptr, &swap_chain->render_pass));

        return SwapChainHandle(swap_chain);
    }

    void RendererVulkan::swap_chain_begin_frame(SwapChainHandle swap_chain_handle, bool& needs_resize) {
        DODO_ASSERT(swap_chain_handle);
        auto swap_chain = swap_chain_handle.cast_to<SwapChain*>();
        if ((swap_chain->swap_chain_vk != nullptr) && _context->surface_get_needs_resize(swap_chain->surface_handle)) {
            needs_resize = true;
            return;
        }

        static const auto default_timeout = std::numeric_limits<uint64_t>::max();
        const VkResult result = _functions.AcquireNextImageKHR(_device, swap_chain->swap_chain_vk, default_timeout, nullptr /* semaphore! */, nullptr, &swap_chain->image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            needs_resize = true;
            return;
        }
        
        DODO_ASSERT((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR));
    }

    void RendererVulkan::swap_chain_resize(CommandQueueHandle cmd_queue_handle, SwapChainHandle swap_chain_handle, uint32_t desired_framebuffer_count) {
        DODO_ASSERT(cmd_queue_handle && swap_chain_handle);

        auto command_queue = cmd_queue_handle.cast_to<CommandQueue*>();
        auto swap_chain = swap_chain_handle.cast_to<SwapChain*>();

        _swap_chain_release(swap_chain);

        if (!_context->queue_family_supports_present(_physical_device, command_queue->queue_family_index, swap_chain->surface_handle)) {
            DODO_ASSERT(false);
            DODO_LOG_ERROR_TAG("Renderer", "Surface not supported by device!");
        }

        const RenderContextVulkan::Functions& context_functions = _context->functions_get();
        auto surface = swap_chain->surface_handle.cast_to<RenderContextVulkan::Surface*>();
        VkSurfaceCapabilitiesKHR surface_caps = {};
        DODO_ASSERT_VK_RESULT(context_functions.GetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, surface->surface_vk, &surface_caps));

        uint32_t picked_image_count = std::max(desired_framebuffer_count, surface_caps.minImageCount + 1);
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
        DODO_ASSERT_VK_RESULT(_functions.CreateSwapchainKHR(_device, &swap_chain_create_info, nullptr, &swap_chain->swap_chain_vk));

        uint32_t image_count = 0;
        DODO_ASSERT_VK_RESULT(_functions.GetSwapchainImagesKHR(_device, swap_chain->swap_chain_vk, &image_count, nullptr));
        swap_chain->images.resize(image_count);
        DODO_ASSERT_VK_RESULT(_functions.GetSwapchainImagesKHR(_device, swap_chain->swap_chain_vk, &image_count, swap_chain->images.data()));

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
            DODO_ASSERT_VK_RESULT(VkCreateFramebuffer(_device, &framebuffer_create_info, nullptr, &swap_chain->framebuffers.at(i)));
        }
    }
    
    FramebufferHandle RendererVulkan::swap_chain_get_framebuffer(SwapChainHandle swap_chain_handle, bool& needs_resize) {
        DODO_ASSERT(swap_chain_handle);
        auto swap_chain = swap_chain_handle.cast_to<SwapChain*>();
        if ((swap_chain->swap_chain_vk == nullptr) || _context->surface_get_needs_resize(swap_chain->surface_handle)) {
            needs_resize = true;
            return FramebufferHandle(); 
        }

        return FramebufferHandle(swap_chain->framebuffers.at(swap_chain->image_index));
    }

    void RendererVulkan::swap_chain_destroy(SwapChainHandle swap_chain_handle) {
        DODO_ASSERT(swap_chain_handle);
        auto swap_chain = swap_chain_handle.cast_to<SwapChain*>();
        _swap_chain_release(swap_chain);
        delete swap_chain;
    }

    void RendererVulkan::_swap_chain_release(SwapChain* swap_chain) {
        for (size_t i = 0; i < swap_chain->framebuffers.size(); i++) {
            vkDestroyFramebuffer(_device, swap_chain->framebuffers.at(i), nullptr);
        }

        swap_chain->framebuffers.clear();
        for (size_t i = 0; i < swap_chain->image_views.size(); i++) {
            vkDestroyImageView(_device, swap_chain->image_views.at(i), nullptr);
        }

        swap_chain->image_views.clear();

        if (swap_chain->swap_chain_vk) {
            _functions.DestroySwapchainKHR(_device, swap_chain->swap_chain_vk, nullptr);
        }
    }

    Dodo::BufferHandle RendererVulkan::buffer_create(BufferUsage buffer_usage, size_t size, void* data /*= nullptr*/)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::buffer_upload_data(BufferHandle buffer_handle, void* data, size_t size, size_t offset /*= 0*/)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::buffer_destroy(BufferHandle buffer_handle)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

}

#endif
