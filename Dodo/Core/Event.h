#pragma once

#include "func.h"

namespace Dodo {

    class Event {
    public:
        bool handled = false;

        Event() = default;
        virtual ~Event() = default;

        virtual uint64_t get_type_id() const = 0;

    protected:
        static uint64_t generate_new_type_id();
    };

    template<typename T>
    class RegisterEventOfType : public Event {
    public:
        static uint64_t get_static_type_id();

        RegisterEventOfType() = default;

        uint64_t get_type_id() const override;
    };

    template<typename E>
    inline uint64_t RegisterEventOfType<E>::get_static_type_id() {
        static const uint64_t type_id = generate_new_type_id();
        return type_id;
    }

    template<typename E>
    inline uint64_t RegisterEventOfType<E>::get_type_id() const {
        return get_static_type_id();
    }

    template<typename T>
    using EventAction = Func<bool(T&)>;

    class EventDispatcher {
    public:
        EventDispatcher(Event& event)
            : event{event} {}

        template<typename T>
        void dispatch(EventAction<T>&& action) {
            // Don't dispatch further.
            if (event.handled)
                return;

            if (event.get_type_id() == T::get_static_type_id())
                event.handled = event.handled || action(static_cast<T&>(event));
        }
    
    private:
        Event& event;
    };

    using EventCallback = Func<void(Event&)>;

}