#pragma once

#include "KeyCode.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // KEYBOARD ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Keyboard
    {
    public:
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