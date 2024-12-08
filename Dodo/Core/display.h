#pragma once

#include "core/func.h"
#include "renderer/render_context.h"

namespace Dodo {

    class Display : public RefCounted {
    public:
        using WindowId = int64_t;
        static constexpr WindowId invalid_window_id = static_cast<WindowId>(-1);

        struct WindowSpecifications {
            uint32_t width = 0;
            uint32_t height = 0;
            std::string title{};
        };

        struct Event {
            struct WindowResizeEvent {
                uint32_t width  = 0;
                uint32_t height = 0;
            };

            enum class Type {
                window_close,
                window_resize,
                none
            };

            Type type = Type::none;
            union {
                WindowResizeEvent resized{};
            };
        };

        using EventCallback = Func<void(WindowId, Event&)>;

        static Ref<Display> create(RenderContext::Type context_type);

        virtual ~Display() = default;

        virtual WindowId window_create(const WindowSpecifications& window_specs) = 0;
        virtual void window_set_event_callback(WindowId window_id, EventCallback&& callback) = 0;
        virtual void window_process_events(WindowId window_id) = 0;
        virtual const void* window_get_platform_data(WindowId window_id) const = 0;
        Ref<RenderContext> get_render_context() const;

    protected:
        WindowId _window_id_counter = 0;
        Ref<RenderContext> _render_context = nullptr;
    };

}