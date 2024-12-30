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

        const uint32_t queue_family_count = _context->queue_family_get_count(device_index);
        _queue_families.resize(queue_family_count);
        for (size_t i = 0; i < queue_family_count; i++) {
            _queue_families.at(i) = _context->queue_family_get(device_index, i);
        }

        _add_queue_create_infos();
        _initialize_device();
    }

    SwapchainHandle RendererVulkan::swapchain_create(SurfaceHandle surface_handle, uint32_t desired_framebuffer_count) {
        DODO_ASSERT(surface_handle);
        auto surface_vk = reinterpret_cast<RenderContextVulkan::SurfaceVulkan*>(surface_handle.handle);

        uint32_t queue_family_index = std::numeric_limits<uint32_t>::max();
        uint32_t queue_index = std::numeric_limits<uint32_t>::max();
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
                    queue_family_index = i;
                    queue_index = j;
                    virtual_count = _queues.at(i).at(j).virtual_count;
                }
            }
        }

        _queues.at(queue_family_index).at(queue_index).virtual_count++;

        auto swapchain_vk = new SwapchainVulkan();
        swapchain_vk->surface_handle = surface_handle;
        swapchain_vk->framebuffer_count = desired_framebuffer_count;
        swapchain_vk->queue_family_index = queue_family_index;
        swapchain_vk->queue_index = queue_index;

        SwapchainHandle swapchain_handle(swapchain_vk);
        // Cache main swapchain.
        if (surface_vk->is_main_surface) {
            _main_swapchain_handle = swapchain_handle;
        }

        _create_swapchain();

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

    void RendererVulkan::_add_queue_create_infos() {
        _queues.clear();

        const size_t queue_family_count = _queue_families.size();
        _queues.resize(queue_family_count);

        const VkQueueFlags desired_queue_families = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
        for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_count); i++) {
            if ((_queue_families.at(i).queueFlags & desired_queue_families) == 0) {
                continue;
            }

            constexpr float queue_priority = 1.0f;
            constexpr uint32_t max_queue_count_per_family = 1;
            VkDeviceQueueCreateInfo queue_create_info = {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = i;
            queue_create_info.queueCount = std::min(_queue_families.at(i).queueCount, max_queue_count_per_family);
            queue_create_info.pQueuePriorities = &queue_priority;
            _queue_create_infos.push_back(std::move(queue_create_info));

            _queues.at(i).resize(_queue_create_infos.at(i).queueCount);
        }
    }

    void RendererVulkan::_initialize_device() {
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
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(_queue_create_infos.size());
        device_create_info.pQueueCreateInfos = _queue_create_infos.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(_enabled_extensions.size());
        device_create_info.ppEnabledExtensionNames = _enabled_extensions.data();
        DODO_ASSERT_VK_RESULT(vkCreateDevice(_physical_device, &device_create_info, nullptr, &_device));

        for (uint32_t i = 0; i < static_cast<uint32_t>(_queues.size()); i++) {
            for (uint32_t j = 0; j < static_cast<uint32_t>(_queues.at(i).size()); j++) {
                if (!_queues.at(i).empty()) {
                    vkGetDeviceQueue(_device, i, j, &_queues.at(i).at(j).queue);
                }
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

    void RendererVulkan::_create_swapchain() {

    }

    VkSurfaceFormatKHR RendererVulkan::_query_surface_format(VkSurfaceKHR surface) {
        const RenderContextVulkan::Functions& context_functions = _context->functions_get();

        uint32_t format_count = 0;
        context_functions.GetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface, &format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(format_count);
        context_functions.GetPhysicalDeviceSurfaceFormatsKHR(_physical_device, surface, &format_count, formats.data());

        VkSurfaceFormatKHR preferred_format = {};
        preferred_format.format = VK_FORMAT_B8G8R8A8_SRGB;
        preferred_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        for (const VkSurfaceFormatKHR& format : formats) {
            if (format.format == preferred_format.format && format.colorSpace == preferred_format.colorSpace) {
                return format;
            }
        }

        return formats.back();
    }

}

#endif