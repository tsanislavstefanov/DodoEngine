#include "pch.h"
#include "Mouse.h"

////////////////////////////////////////////////////////////////
// MOUSE ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void Mouse::Update()
{
    std::memcpy(&m_PreviousState, &m_State, sizeof(State));
    m_State.Clear();
}

void Mouse::Reset()
{
    m_State.Clear();
    m_PreviousState.Clear();
}

bool Mouse::OnWheelScroll(float wheelDelta)
{
    WheelScroll.Emit(MouseScrollEvent{ wheelDelta });
    m_State.WheelDelta = wheelDelta;
    return true;
}

bool Mouse::OnButtonDown(MouseCode mouseCode)
{
    ButtonDown.Emit(MouseButtonDownEvent{ mouseCode });
    const auto buttonIndex = static_cast<size_t>(mouseCode);
    m_State.Buttons.at(buttonIndex) = true;
    return true;
}

bool Mouse::OnButtonUp(MouseCode mouseCode)
{
    ButtonUp.Emit(MouseButtonUpEvent{ mouseCode });
    const auto buttonIndex = static_cast<size_t>(mouseCode);
    m_State.Buttons.at(buttonIndex) = false;
    return true;
}