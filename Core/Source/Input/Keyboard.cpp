#include "pch.h"
#include "Keyboard.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // KEYBOARD ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void Keyboard::Update()
    {
        // Move current state to previous.
        std::memcpy(&m_PreviousState, &m_State, sizeof(State));
        m_State.Clear();
    }

    void Keyboard::Reset()
    {
        m_State.Clear();
        m_PreviousState.Clear();
    }

    bool Keyboard::OnKeyDown(KeyCode keyCode)
    {
        const auto keyIndex = static_cast<size_t>(keyCode);
        m_State.Keys.at(keyIndex) = true;
        return true;
    }

    bool Keyboard::OnKeyUp(KeyCode keyCode)
    {
        const auto keyIndex = static_cast<size_t>(keyCode);
        m_State.Keys.at(keyIndex) = false;
        return true;
    }

}