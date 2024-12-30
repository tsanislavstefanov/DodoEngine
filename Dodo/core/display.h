#pragma once

#include "core/func.h"

namespace Dodo {

    class RenderContext;

    class Display : public RefCounted {
    public:
        static Ref<Display> create();

        Display() = default;
        virtual ~Display() = default;

        using WindowId = int64_t;
        static constexpr WindowId invalid_window_id = static_cast<WindowId>(-1);

        struct WindowSpecifications {
            uint32_t width = 0;
            uint32_t height = 0;
            std::string title = {};
        };

        struct Event {
            struct WindowResizeEvent {
                uint32_t width = 0;
                uint32_t height = 0;
            };

            enum class Type {
                window_close,
                window_resize,
                none
            };

            WindowId window_id = invalid_window_id;
            Type type = Type::none;

            union {
                WindowResizeEvent resized = {};
            };
        };

        using EventCallback = Func<void(Event&)>;

        virtual WindowId window_create(const WindowSpecifications& window_specs) = 0;
        virtual void window_show_and_focus(WindowId window_id) = 0;
        virtual void window_focus(WindowId window_id) = 0;
        virtual void window_set_event_callback(WindowId window_id, Func<void(Event&)>&& callback) = 0;
        virtual void window_process_events(WindowId window_id) = 0;
        virtual const void* window_get_platform_data(WindowId window_id) const = 0;

        virtual uint32_t context_get_count() const = 0;
        virtual Ref<RenderContext> context_get(size_t index) const = 0;
    };

}
