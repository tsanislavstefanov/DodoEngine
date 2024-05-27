#pragma once

#include "Bindings/Event.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Swapchain.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOW SPECS ////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct WindowSpecs
    {
        uint32_t Width = 0, Height = 0;
        std::string Title{};
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
    // WINDOW MINIMIZE EVENT /////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct WindowMinimizeEvent : public Event
    {
        const bool Iconify;

        WindowMinimizeEvent(bool iconify)
            : Iconify(iconify)
        {}

        EVENT_TYPE(WindowMinimize)
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

        uint32_t GetWidth () const { return m_Data.Width ; }
        uint32_t GetHeight() const { return m_Data.Height; }

        Ref<Swapchain> GetSwapchain() const { return m_Swapchain; }

        void SetEventCallback(const EventCallback& eventCallback) { m_Data.EventCallback = eventCallback; }
        bool HasFocus() const { return m_Data.HasFocus; }
        void ProcessEvents();

        virtual void* GetHandle() const = 0;
        virtual void  SetTitle(const std::string& title) = 0;
        virtual void  PollEvents() = 0;
        virtual void  Destroy() = 0;

    protected:
        ////////////////////////////////////////////////////////////
        // WINDOWS DATA ////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct WindowData
        {
            uint32_t Width = 0, Height = 0;
            std::string Title{};
            bool HasFocus = false;
            EventCallback EventCallback{};
        };

        WindowData m_Data{};
        Ref<RenderContext> m_RenderContext = nullptr;
        Ref<Swapchain> m_Swapchain = nullptr;
    };

}