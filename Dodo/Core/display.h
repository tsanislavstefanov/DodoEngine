#pragma once

#include "renderer/render_context.h"

namespace Dodo {

    class Display : public RefCounted {
    public:
        static Ref<Display> create(RenderContext::Type context_type);

        using WindowId = size_t;

        struct WindowSpecifications {
            uint32_t width  = 0;
            uint32_t height = 0;
            std::string title{};
        };

        virtual WindowId window_create(const WindowSpecifications& window_specs) = 0;

        struct Event {
            struct WindowResizeEvent {
                uint32_t width  = 0;
                uint32_t height = 0;
            };

            enum class Type {
                window_close ,
                window_resize,
                none
            };

            Type type = Type::none;

            union {
                WindowResizeEvent resize{};
            };
        };

        using EventCallback = std::function<void(WindowId, Event&)>;

        virtual void window_set_event_callback(WindowId window, EventCallback&& callback) = 0;
        virtual bool window_should_close(WindowId window) const = 0;
        virtual void window_process_events(WindowId window) = 0;
        virtual Ref<RenderContext> render_context_create(Renderer::Type type) = 0;

    protected:
        WindowId _current_window_id = 0;
    };

}