#pragma once

#include "MouseCode.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // MOUSE ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Mouse
    {
    public:
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