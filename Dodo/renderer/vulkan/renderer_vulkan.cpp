#include "pch.h"

#ifdef DODO_VULKAN

#include "renderer_vulkan.h"
#include "render_context_vulkan.h"

namespace Dodo {

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
        _initialize_main_queue();
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

            static constexpr float queue_priority = 1.0f;
            const uint32_t max_queue_count_per_family = 1;

            VkDeviceQueueCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_info.queueFamilyIndex = i;
            create_info.queueCount = std::min(_queue_families.at(i).queueCount, max_queue_count_per_family);
            create_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(std::move(create_info));

            _queues.at(i).resize(queue_create_infos.at(i).queueCount);
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
                vkGetDeviceQueue(_device, i, j, &_queues.at(i).at(j).queue);
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

    void RendererVulkan::_initialize_main_queue() {
        for (uint32_t i = 0; i < _queues.size(); i++) {
            for (uint32_t j = 0; j < _queues.at(i).size(); j++) {
                _main_queue_family_index = i;
                _main_queue_index = j;
                break;
            }
        }
    }

    SwapchainHandle RendererVulkan::swapchain_create(SurfaceHandle surface_handle, uint32_t desired_frame_count) {
        DODO_ASSERT(surface_handle);
        auto surface = reinterpret_cast<RenderContextVulkan::SurfaceVulkan*>(surface_handle.handle);

        uint32_t present_queue_family_index = std::numeric_limits<uint32_t>::max();
        uint32_t present_queue_index = std::numeric_limits<uint32_t>::max();
        if (_context->queue_family_supports_present(_physical_device, _main_queue_family_index, surface_vk->surface)) {
            present_queue_family_index = _main_queue_family_index;
            present_queue_index = _main_queue_index;
        }
        else {
            uint32_t virtual_count = std::numeric_limits<uint32_t>::max();
            for (uint32_t i = 0; i < _queues.size(); i++) {
                if (_queues.at(i).empty()) {
                    continue;
                }

                if (!_context->queue_family_supports_present(_physical_device, i, surface_vk->surface)) {
                    continue;
                }

                for (uint32_t j = 0; j < _queues.at(i).size(); j++) {
                    if (_queues.at(i).at(j).virtual_count < virtual_count) {
                        present_queue_family_index = i;
                        present_queue_index = j;
                        virtual_count = _queues.at(i).at(j).virtual_count;
                    }
                }
            }
        }

        _queues.at(present_queue_family_index).at(present_queue_index).virtual_count++;

        auto swapchain = new Swapchain();
        swapchain->surface_handle = surface_handle;
        swapchain->present_queue_family_index = present_queue_family_index;
        swapchain->present_queue_index = present_queue_index;

        SwapchainHandle swapchain_handle(swapchain);
        _invalidate_swapchain(swapchain_handle, desired_frame_count);
        return swapchain_handle;
    }

    void RendererVulkan::swapchain_begin_frame(SwapchainHandle swapchain_handle)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::swapchain_present(SwapchainHandle swapchain_handle)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::swapchain_on_resize(SwapchainHandle swapchain_handle, uint32_t width, uint32_t height)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::swapchain_destroy(SwapchainHandle swapchain_handle)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::_swapchain_invalidate(SwapchainHandle swapchain_handle, uint32_t desired_frame_count) {
        _swapchain_select_format_and_color_space(swapchain_handle);
    }

    void RendererVulkan::_swapchain_select_format_and_color_space(SwapchainHandle swapchain_handle, VkFormat desired_format = VK_FORMAT_B8G8R8A8_SRGB, VkColorSpaceKHR desired_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        const RenderContextVulkan::Functions& context_functions = _context->functions_get();

        uint32_t format_count = 0;
        context_functions.GetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        context_functions.GetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface, &format_count, formats.data());

        VkFormat picked_format = formats.back().format;
        VkColorSpaceKHR picked_color_space = formats.back().colorSpace;
        for (const auto& format : formats) {
            if (format.format == desired_format && format.colorSpace == desired_color_space) {
                picked_format = format.format;
                picked_color_space = format.colorSpace;
                break;
            }
        }

        auto swapchain = swapchain_handle.cast_to<Swapchain>();
        swapchain->format = picked_format;
        swapchain->color_space = picked_color_space;
    }

    void RendererVulkan::_swapchain_release(SwapchainHandle swapchain_handle) {

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
