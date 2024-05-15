#pragma once

#include "Bindings/Signal.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // WINDOW SPECS ////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct WindowSpecs
    {
        uint32_t    Width  = 0;
        uint32_t    Height = 0;
        std::string Title{};
    };

    ////////////////////////////////////////////////////////////////
    // WINDOW RESIZE EVENT /////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct WindowResizeEvent
    {
        const uint32_t Width, Height;

        WindowResizeEvent(uint32_t width, uint32_t height)
            : Width (width )
            , Height(height)
        {}
    };

    ////////////////////////////////////////////////////////////////
    // WINDOW //////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Window : public RefCounted
    {
    public:
        static Ref<Window> Create(const WindowSpecs& specs);

        Signal<const WindowResizeEvent&> Resize{};
        Signal<> Close{};

        [[nodiscard]] virtual void* GetHandle() const = 0;

        [[nodiscard]] uint32_t GetWidth() const
        {
            return m_Data.Width;
        }

        [[nodiscard]] uint32_t GetHeight() const
        {
            return m_Data.Height;
        }

        [[nodiscard]] bool HasFocus() const
        {
            return m_Data.HasFocus;
        }

        virtual void SetTitle(const std::string& title) = 0;

        virtual void PollEvents() = 0;

        virtual void Dispose() = 0;

    protected:
        ////////////////////////////////////////////////////////////
        // WINDOWS DATA ////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct WindowData
        {
            uint32_t    Width    = 0;
            uint32_t    Height   = 0;
            std::string Title{};
            bool        HasFocus = false;
        };

        WindowData m_Data{};
    };

}