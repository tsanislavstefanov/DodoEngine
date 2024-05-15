#pragma once

#include "KeyCode.h"
#include "Bindings/Signal.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // KEY DOWN EVENT //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct KeyDownEvent
    {
        const KeyCode Code;

        KeyDownEvent(KeyCode keyCode)
            : Code(keyCode)
        {}
    };

    ////////////////////////////////////////////////////////////////
    // KEY UP EVENT ////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct KeyUpEvent
    {
        const KeyCode Code;

        KeyUpEvent(KeyCode keyCode)
            : Code(keyCode)
        {}
    };

    ////////////////////////////////////////////////////////////////
    // KEYBOARD ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Keyboard
    {
    public:
        Signal<const KeyDownEvent&> KeyDown{};
        Signal<const KeyUpEvent&>   KeyUp  {};

        virtual ~Keyboard() = default;

        bool GetKey(KeyCode keyCode) const
        {
            const auto keyIndex = static_cast<size_t>(keyCode);
            return m_State.Keys.at(keyIndex);
        }

        bool GetKeyDown(KeyCode keyCode) const
        {
            const auto keyIndex = static_cast<size_t>(keyCode);
            return m_State.Keys.at(keyIndex) && !m_PreviousState.Keys.at(keyIndex);
        }

        bool GetKeyUp(KeyCode keyCode) const
        {
            const auto keyIndex = static_cast<size_t>(keyCode);
            return m_PreviousState.Keys.at(keyIndex) && !m_State.Keys.at(keyIndex);
        }

        void Update();

        void Reset();

    protected:
        bool OnKeyDown(KeyCode keyCode);

        bool OnKeyUp(KeyCode keyCode);

    private:
        ////////////////////////////////////////////////////////////
        // STATE ///////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct State
        {
            static constexpr auto MaxKeyCount = static_cast<size_t>(KeyCode::AutoCount);
            std::array<bool, MaxKeyCount> Keys{};

            void Clear()
            {
                std::memset(this, 0, sizeof(State));
            }
        };

        State m_State{};
        State m_PreviousState{};
    };

}