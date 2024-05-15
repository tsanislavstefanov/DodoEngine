#pragma once

#include "MouseCode.h"
#include "Bindings/Signal.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // MOUSE SCROLL EVENT //////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct MouseScrollEvent
    {
        const float WheelDelta;

        MouseScrollEvent(float wheelDelta)
            : WheelDelta(wheelDelta)
        {}
    };

    ////////////////////////////////////////////////////////////////
    // MOUSE BUTTON DOWN EVENT /////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct MouseButtonDownEvent
    {
        const MouseCode Code;

        MouseButtonDownEvent(MouseCode mouseCode)
            : Code(mouseCode)
        {}
    };

    ////////////////////////////////////////////////////////////////
    // MOUSE BUTTON UP EVENT ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct MouseButtonUpEvent
    {
        const MouseCode Code;

        MouseButtonUpEvent(MouseCode mouseCode)
            : Code(mouseCode)
        {}
    };

    ////////////////////////////////////////////////////////////////
    // MOUSE ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Mouse
    {
    public:
        Signal<const MouseScrollEvent&>     WheelScroll{};
        Signal<const MouseButtonDownEvent&> ButtonDown {};
        Signal<const MouseButtonUpEvent&>   ButtonUp   {};

        virtual ~Mouse() = default;

        float GetWheelDelta() const
        {
            return m_State.WheelDelta;
        }

        bool GetButton(MouseCode mouseCode) const
        {
            const auto buttonIndex = static_cast<size_t>(mouseCode);
            return m_State.Buttons.at(buttonIndex);
        }

        bool GetButtonDown(MouseCode mouseCode) const
        {
            const auto buttonIndex = static_cast<size_t>(mouseCode);
            return m_State.Buttons.at(buttonIndex) && !m_PreviousState.Buttons.at(buttonIndex);
        }

        bool GetButtonUp(MouseCode mouseCode) const
        {
            const auto buttonIndex = static_cast<size_t>(mouseCode);
            return m_PreviousState.Buttons.at(buttonIndex) && !m_State.Buttons.at(buttonIndex);
        }

        void Update();

        void Reset();

    protected:
        bool OnWheelScroll(float wheelDelta);

        bool OnButtonDown(MouseCode mouseCode);
                        
        bool OnButtonUp(MouseCode mouseCode);

    private:
        ////////////////////////////////////////////////////////////
        // STATE ///////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct State
        {
            float WheelDelta = 0.0f;
            static constexpr auto MaxButtonCount = static_cast<size_t>(MouseCode::AutoCount);
            std::array<bool, MaxButtonCount> Buttons{};

            void Clear()
            {
                std::memset(this, 0, sizeof(State));
            }
        };

        State m_State{};
        State m_PreviousState{};
    };

}