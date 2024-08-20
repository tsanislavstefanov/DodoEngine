#pragma once

#include "Bindings/Event.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOW SPECS ////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct WindowSpecs
    {
        uint32_t Width = 0, Height = 0;
        std::string Title = "Unnamed";
        bool Maximized = false;
    };

    ////////////////////////////////////////////////////////////////
    // WINDOW RESIZE EVENT /////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct WindowResizeEvent : public Event
    {
        const uint32_t Width, Height;

        WindowResizeEvent(uint32_t width, uint32_t height)
            : Width (width )
            , Height(height)
        {}

        EVENT_TYPE(WindowResize)
    };

    ////////////////////////////////////////////////////////////////
    // WINDOW CLOSE EVENT //////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct WindowCloseEvent : public Event
    {
        WindowCloseEvent() = default;

        EVENT_TYPE(WindowClose)
    };

    ////////////////////////////////////////////////////////////////
    // WINDOW //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Window : public RefCounted
    {
    public:
        static Ref<Window> Create(const WindowSpecs& specs);

        virtual ~Window() = default;

        virtual void Init() {}

        void ProcessEvents();

        virtual void Destroy() {}
        
        void* GetHandle() const
        {
            return m_Handle;
        }

        uint32_t GetWidth() const
        {
            return m_Data.Width;
        }

        uint32_t GetHeight() const
        {
            return m_Data.Height;
        }

        void SetEventCallback(const EventCallback& eventCallback)
        {
            m_Data.EventCallback = eventCallback;
        }

    protected:
        virtual void PollEvents() = 0;

        ////////////////////////////////////////////////////////////
        // WINDOW DATA /////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct WindowData
        {
            uint32_t Width = 0, Height = 0;
            EventCallback EventCallback{};
        };

        void* m_Handle = nullptr;
        WindowData m_Data{};
    };

}