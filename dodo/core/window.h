#pragma once

#include "bindings/func.h"
#ifdef DODO_USE_VULKAN
#   include "graphics/drivers/vulkan/vulkan.h"
#endif

namespace Dodo {

    struct WindowResizeEvent
    {
        const uint32_t width = 0, height = 0;
    };

    struct WindowSpecifications
    {
        uint32_t width = 1280, height = 720;
        std::string title = "Dodo Engine";
        bool maximized = false;
    };

    class Window : public RefCounted
    {
    public:
        static Ref<Window> create(WindowSpecifications&& specs);

        Func<void(WindowResizeEvent&)> on_resized{};
        Func<void()> on_close{};

        Window(WindowSpecifications&& specs)
            : specifications_(std::move(specs))
        {}
        virtual ~Window() = default;

        virtual void process_events() = 0;
        virtual void* native_handle() const = 0;

        inline uint32_t width() const
        {
            return specifications_.width;
        }

        inline uint32_t height() const
        {
            return specifications_.height;
        }

        inline bool has_focus() const
        {
            return has_focus_;
        }

#ifdef DODO_USE_VULKAN
        virtual std::vector<std::string> get_required_vulkan_instance_extensions() const = 0;
        virtual VkSurfaceKHR create_vulkan_surface(VkInstance instance) const = 0;
#endif

    protected:
        WindowSpecifications specifications_{};
        bool has_focus_ = true;
    };

}