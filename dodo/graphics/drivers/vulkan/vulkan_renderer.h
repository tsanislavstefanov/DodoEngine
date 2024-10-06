#pragma once

#include "vulkan_adapter.h"
#include "Vulkan_allocator.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_swapchain.h"
#include "graphics/renderer.h"

namespace Dodo {

    class VulkanRenderer : public Renderer
    {
    public:
        VulkanRenderer(RenderThread& render_thread, const Window& target_window, VSyncMode vsync_mode);
        ~VulkanRenderer();

        BufferID create_buffer(BufferSpecifications&& buffer_specs) override;
        void free_buffer(BufferID buffer_id) override;

        void begin_frame() override;
        void end_frame() override;
        void resize_swapchain(uint32_t width, uint32_t height) override;
        RenderThreadPolicy get_thread_policy() const { return RenderThreadPolicy::multi_threaded; }
        RendererType get_type() const { return RendererType::vulkan; }

    private:
        Ref<VulkanInstance> instance_ = nullptr;
        Ref<VulkanAdapter> adapter_ = nullptr;
        Ref<VulkanDevice> device_ = nullptr;
        Ref<VulkanSwapchain> swapchain_ = nullptr;
        Ref<VulkanAllocator> allocator_ = nullptr;
    };

}