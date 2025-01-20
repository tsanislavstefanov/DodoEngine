#pragma once

namespace Dodo {

    class RenderBackend;

    class Display {
    public:
        using WindowId = int64_t;
        static constexpr WindowId invalid_window = UINT64_MAX;

        struct WindowSpecifications {
            uint32_t width{0};
            uint32_t height{0};
            std::string title{};
        };

        struct Event {
            struct WindowResize {
                uint32_t width{0};
                uint32_t height{0};
            };

            enum class Type {
                window_close,
                window_resize,
                unknown
            };

            WindowId window{invalid_window};
            Type type{Type::unknown};
            union {
                WindowResize resized{};
            };
        };

        static Display& singleton_get();

        Display() = default;
        virtual ~Display() = default;

        virtual WindowId window_create(const WindowSpecifications& window_specs) = 0;
        virtual const void* window_get_platform_data(WindowId window) const = 0;
        virtual void window_set_event_callback(WindowId window, const std::function<void(Event&)>& callback) = 0;
        virtual void window_process_events(WindowId window) = 0;
        virtual void window_destroy(WindowId window) = 0;
        virtual uint32_t render_backend_get_count() const = 0;
        virtual Ref<RenderBackend> render_backend_get(size_t index) const = 0;
    };

}
