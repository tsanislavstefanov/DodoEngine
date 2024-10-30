#pragma once

#include "Bindings/Func.h"

namespace Dodo {

    class Event
    {
    public:
        bool Handled = false;

        Event() = default;
        virtual ~Event() = default;

        virtual uint64_t GetTypeId() const = 0;

    protected:
        uint64_t GenerateNewTypeId() const;
    };

    template<typename E>
    class RegisterEventOfType : public Event
    {
    public:
        static uint64_t GetStaticTypeId();

        RegisterEventOfType() = default;

        uint64_t GetTypeId() const override;
    };

    template<typename E>
    inline uint64_t RegisterEventOfType<E>::GetStaticTypeId()
    {
        static const uint64_t typeId = GenerateNewTypeId();
        return typeId;
    }

    template<typename E>
    inline uint64_t RegisterEventOfType<E>::GetTypeId() const
    {
        return GetStaticTypeId();
    }

    template<typename E>
    using EventAction = Func<bool(E&)>;

    class EventDispatcher
    {
    public:
        EventDispatcher(Event& event)
            : m_Event{event}
        {
        }

        template<typename E>
        void Dispatch(EventAction<E>&& action)
        {
            // Don't dispatch further.
            if (m_Event.Handled)
                return;

            if (m_Event.GetTypeId() == E::GetStaticTypeId())
                m_Event.Handled = m_Event.Handled || action(static_cast<E&>(m_Event));
        }
    
    private:
        Event& m_Event;
    };

    using EventCallback = Func<void(Event&)>;

}