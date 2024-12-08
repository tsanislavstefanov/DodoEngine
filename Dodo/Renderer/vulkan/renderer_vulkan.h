#pragma once

#include "renderer/renderer.h"

namespace Dodo {

    class RenderContextVulkan;

    class RendererVulkan : public Renderer
    {
    public:
        RendererVulkan(Ref<RenderContextVulkan> render_context, RenderContext::Adapter::Type adapter_type);

        SwapchainHandle swapchain_create(SurfaceHandle surface) override;
        void swapchain_begin_frame(SwapchainHandle swapchain) override;
        void swapchain_present(SwapchainHandle swapchain) override;
        void swapchain_on_resize(SwapchainHandle swapchain, uint32_t width, uint32_t height) override;
        void swapchain_destroy(SwapchainHandle swapchain) override;

        BufferHandle buffer_create(BufferUsage buffer_usage, size_t size, void* data = nullptr) override;
        void buffer_upload_data(BufferHandle buffer, void* data, size_t size, size_t offset = 0) override;
        void buffer_destroy(BufferHandle buffer) override;

    private:
        void _initialize_device();
        std::optional<uint32_t> _find_queue_family_index(VkQueueFlags family) const;
        void _request_extension(const std::string& name, bool is_required);

        Ref<RenderContextVulkan> _render_context = nullptr;
        size_t _device_index = 0;
        VkPhysicalDevice _physical_device = nullptr;
        std::optional<uint32_t> _graphics_queue_index{};
        std::map<std::string, bool> _requested_extensions{};
        std::vector<const char*> _enabled_extensions{};
        VkDevice _device = nullptr;
    };

}