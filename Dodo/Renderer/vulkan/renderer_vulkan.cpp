#include "renderer_vulkan.h"
#include "pch.h"
#include "vulkan_utils.h"
#include "renderer_vulkan.h"
#include "render_context_vulkan.h"

namespace Dodo {

    RendererVulkan::RendererVulkan(Ref<RenderContextVulkan> render_context, RenderContext::Adapter::Type adapter_type)
        : _render_context{render_context} {
        // Find physical device of requested adapter type.
        const uint32_t adapter_count = _render_context->get_adapter_count();
        for (size_t i = 0; i < adapter_count; i++) {
            if (_render_context->get_adapter(i).type == adapter_type) {
                _device_index = i;
                _physical_device = _render_context->get_physical_device(_device_index);
            }
        }

        // Get any physical device when requested adapter type
        // has not been found!
        _physical_device = (!_physical_device) ? _render_context->get_physical_device(0) : _physical_device;
        DODO_ASSERT(_physical_device, "Vulkan failed to select a suitable GPU!");

        _initialize_device();
    }

    Dodo::SwapchainHandle RendererVulkan::swapchain_create(SurfaceHandle surface)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::swapchain_begin_frame(SwapchainHandle swapchain)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::swapchain_present(SwapchainHandle swapchain)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::swapchain_on_resize(SwapchainHandle swapchain, uint32_t width, uint32_t height)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::swapchain_destroy(SwapchainHandle swapchain)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    Dodo::BufferHandle RendererVulkan::buffer_create(BufferUsage buffer_usage, size_t size, void* data /*= nullptr*/)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::buffer_upload_data(BufferHandle buffer, void* data, size_t size, size_t offset /*= 0*/)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::buffer_destroy(BufferHandle buffer)
    {
        throw std::logic_error("The method or operation is not implemented.");
    }

    void RendererVulkan::_initialize_device() {
        _graphics_queue_index = _find_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
        DODO_ASSERT(_graphics_queue_index.has_value(), "Vulkan graphics queue not found!");

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
        static constexpr float default_queue_priority = 1.0f;
        VkDeviceQueueCreateInfo& queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = _graphics_queue_index.value();
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &default_queue_priority;
        queue_create_infos.push_back(std::move(queue_create_info));

        std::set<std::string> supported_extensions{};
        uint32_t extension_count = 0;
        DODO_VERIFY_VK_RESULT(vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &extension_count, nullptr));
        std::vector<VkExtensionProperties> extensions(extension_count);
        DODO_VERIFY_VK_RESULT(vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &extension_count, extensions.data()));
        for (const auto& extension : extensions)
            m_SupportedExtensions.insert(extension.extensionName);

        _request_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);

        // Validate requested extensions.
        for (const auto& [name, is_required] : _requested_extensions) {
            if (!supported_extensions.contains(name)) {
                if (is_required) {
                    DODO_ASSERT(false, "Vulkan required device extension[{0}] not supported!", name);
                }
                else {
                    DODO_LOG_WARNING_TAG("Renderer", "Vulkan device extension [{0}] not supported!", name);
                    continue;
                }
            }

            _enabled_extensions.push_back(name.c_str());
        }

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(_enabled_extensions.size());
        device_create_info.ppEnabledExtensionNames = _enabled_extensions.data();
        DODO_VERIFY_VK_RESULT(vkCreateDevice(_physical_device, &device_create_info, nullptr, &_device));

        vkGetDeviceQueue(m_Device, m_Adapter->GetGraphicsQueueIndex(), 0, &m_GraphicsQueue);

    }

    std::optional<uint32_t> RendererVulkan::_find_queue_family_index(VkQueueFlags family) const {
        const uint32_t queue_count = _render_context->get_queue_family_count(_device_index);
        for (size_t i = 0; i < queue_count; i++) {
            if (_render_context->get_queue_family(_device_index, i).queueFlags & family) {
                return i;
            }
        }

        return std::nullopt;
    }

    void RendererVulkan::_request_extension(const std::string& name, bool is_required) {
        DODO_ASSERT(!_requested_extensions.contains(name), "Vulkan device extension [{0}] already requested!", name);
        _requested_extensions.insert({ name, is_required });
    }

}