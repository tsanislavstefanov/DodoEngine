#pragma once

#include "Func.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // EVENT TYPE //////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    enum class EventType
    {
        WindowResize   ,
        WindowClose    ,
        KeyDown        ,
        KeyUp          ,
        MouseScroll    ,
        MouseButtonDown,
        MouseButtonUp  ,
        AutoCount      ,
        None
    };

    ////////////////////////////////////////////////////////////////
    // EVENT TYPE MACRO(S) /////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

#ifndef EVENT_TYPE
    #define EVENT_TYPE(TYPE)                                                        \
        static EventType GetStaticType()                { return EventType::TYPE; } \
               EventType GetType      () const override { return GetStaticType(); }
#endif

    ////////////////////////////////////////////////////////////////
    // EVENT ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Event
    {
    public:
        virtual ~Event() = default;

        virtual EventType GetType() const = 0;

        bool IsHandled() const
        {
            return m_Handled;
        }

    private:
        bool m_Handled = false;

        friend class EventDispatcher;
    };

    ////////////////////////////////////////////////////////////////
    // EVENT ACTION ////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    template<typename T>
    using EventAction = Func<bool(T&)>;

    ////////////////////////////////////////////////////////////////
    // EVENT DISPATCHER ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class EventDispatcher
    {
    public:
        EventDispatcher(Event& event)
            : m_Event(event)
        {}

        template<typename T>
        void Dispatch(const EventAction<T>& eventAction)
        {
            if (m_Event.IsHandled()) // Don't dispatch anymore.
            {
                return;
            }
            
            if (m_Event.GetType() == T::GetStaticType())
            {
                m_Event.m_Handled |= eventAction(static_cast<T&>(m_Event));
            }
        }

    private:
        Event& m_Event;
    };

    ////////////////////////////////////////////////////////////////
    // EVENT CALLBACK //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    using EventCallback = Func<void(Event&)>;

}