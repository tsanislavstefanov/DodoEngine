#include "pch.h"
#include "vulkan.h"
#include "vulkan_renderer.h"
#include "vulkan_buffer.h"

namespace Dodo {

    VulkanRenderer::VulkanRenderer(RenderThread& render_thread, const Window& target_window, VSyncMode vsync_mode)
        : Renderer(render_thread)
        , instance_(Ref<VulkanInstance>::create(target_window))
        , adapter_(Ref<VulkanAdapter>::create(instance_))
        , device_(Ref<VulkanDevice>::create(instance_, adapter_))
        , swapchain_(Ref<VulkanSwapchain>::create(instance_, device_, target_window, vsync_mode))
    {
        VulkanAllocatorSpecifications allocator_specs{};
        allocator_specs.api_version = instance_->get_api_version();
        allocator_specs.instance = instance_->get_vulkan_instance();
        allocator_specs.physical_device = adapter_->get_vulkan_physical_device();
        allocator_specs.device = device_->get_vulkan_device();
        allocator_ = Ref<VulkanAllocator>::create(std::move(allocator_specs));
    }

    VulkanRenderer::~VulkanRenderer()
    {
        swapchain_ = nullptr;
        allocator_ = nullptr;
        device_ = nullptr;
        instance_ = nullptr;
    }

    BufferID VulkanRenderer::create_buffer(BufferSpecifications&& buffer_specs)
    {
        VulkanBufferInfo* buffer_info = new VulkanBufferInfo{};
        buffer_info->size = buffer_specs.size;

        submit([allocator = allocator_, specs = std::move(buffer_specs), buffer_info]() mutable {
            // We need to stream the data via staging
            // buffer for better performance.
            if (specs.initial_data)
            {
            }
            else
            {
                VkBufferCreateInfo buffer_create_info{};
                buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                buffer_create_info.size  = specs.size;
                buffer_create_info.usage = Utils::convert_to_vulkan_buffer_usage(specs.usage);
                buffer_info->allocation  =
                    allocator->allocate(buffer_create_info, VMA_MEMORY_USAGE_CPU_TO_GPU, &buffer_info->buffer);
            }
        });

        return BufferID(buffer_info);
    }

    void VulkanRenderer::free_buffer(BufferID buffer_id)
    {
        submit([allocator = allocator_, buffer_id]() mutable {
            auto buffer_info = reinterpret_cast<VulkanBufferInfo*>(buffer_id.id);
            allocator->free(buffer_info->buffer, buffer_info->allocation);
            delete buffer_info;
        });
    }

    void VulkanRenderer::begin_frame()
    {
        submit([instance = Ref<VulkanRenderer>(this)]() mutable {
            instance->swapchain_->begin_frame();
        });
    }

    void VulkanRenderer::end_frame()
    {
        submit([instance = Ref<VulkanRenderer>(this)]() mutable {
            instance->swapchain_->end_frame();
        });
    }

    void VulkanRenderer::resize_swapchain(uint32_t width, uint32_t height)
    {
        swapchain_->on_resize(width, height);
    }

}