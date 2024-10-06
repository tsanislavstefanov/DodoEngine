#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // KEY CODE ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    enum class KeyCode
    {
        LeftArrow ,
        UpArrow   ,
        RightArrow,
        DownArrow ,
        Alpha0    ,
        Alpha1    ,
        Alpha2    ,
        Alpha3    ,
        Alpha4    ,
        Alpha5    ,
        Alpha6    ,
        Alpha7    ,
        Alpha8    ,
        Alpha9    ,
        Q         ,
        W         ,
        E         ,
        R         ,
        T         ,
        Y         ,
        u         ,
        I         ,
        O         ,
        P         ,
        A         ,
        AutoCount ,
        None
    };

    ////////////////////////////////////////////////////////////////
    // KEYBOARD ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Keyboard
    {
    public:
        Keyboard() = default;
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
        void OnKeyDown(KeyCode keyCode)
        {
            const auto keyIndex = static_cast<size_t>(keyCode);
            m_State.Keys.at(keyIndex) = true;
        }

        void OnKeyUp(KeyCode keyCode)
        {
            const auto keyIndex = static_cast<size_t>(keyCode);
            m_State.Keys.at(keyIndex) = false;
        }

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