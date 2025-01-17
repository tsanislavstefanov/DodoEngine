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
        static constexpr WindowId invalid_window = static_cast<WindowId>(-1);

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

            WindowId window = invalid_window;
            Type type = Type::none;

            union {
                WindowResizeEvent resized = {};
            };
        };

        virtual WindowId window_create(const WindowSpecifications& window_specs) = 0;
        virtual void window_show_and_focus(WindowId window) = 0;
        virtual void window_focus(WindowId window) = 0;
        virtual void window_set_event_callback(WindowId window, Func<void(Event&)>&& callback) = 0;
        virtual void window_process_events(WindowId window) = 0;
        virtual const void* window_get_platform_data(WindowId window) const = 0;

        virtual uint32_t context_get_count() const = 0;
        virtual Ref<RenderContext> context_get(size_t index) const = 0;
    };

}