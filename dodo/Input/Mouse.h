#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // MOUSE CODE //////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    enum class MouseCode
    {
        Left,
        Middle,
        Right,
        AutoCount,
        None
    };

    ////////////////////////////////////////////////////////////////
    // MOUSE ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Mouse
    {
    public:
        Mouse() = default;
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

        void Update()
        {
            // Move current state to previous.
            std::memcpy(&m_PreviousState, &m_State, sizeof(State));
            m_State.Clear();
        }

        void Reset()
        {
            m_State.Clear();
            m_PreviousState.Clear();
        }

    protected:
        void OnWheelScroll(float wheelDelta)
        {
            m_State.WheelDelta = wheelDelta;
        }

        void OnButtonDown(MouseCode mouseCode)
        {
            const auto buttonIndex = static_cast<size_t>(mouseCode);
            m_State.Buttons.at(buttonIndex) = true;
        }
                        
        void OnButtonUp(MouseCode mouseCode)
        {
            const auto buttonIndex = static_cast<size_t>(mouseCode);
            m_State.Buttons.at(buttonIndex) = false;
        }

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