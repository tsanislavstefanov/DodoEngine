#include "pch.h"
#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"

////////////////////////////////////////////////////////////////
// INPUT ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

Keyboard* Input::s_Keyboard = nullptr;
Mouse* Input::s_Mouse = nullptr;

bool Input::GetKey(KeyCode keyCode)
{
    ASSERT(s_Keyboard, "Keyboard not assigned!");
    return s_Keyboard->GetKey(keyCode);
}

bool Input::GetKeyDown(KeyCode keyCode)
{
    ASSERT(s_Keyboard, "Keyboard not assigned!");
    return s_Keyboard->GetKeyDown(keyCode);
}

bool Input::GetKeyUp(KeyCode keyCode)
{
    ASSERT(s_Keyboard, "Keyboard not assigned!");
    return s_Keyboard->GetKeyUp(keyCode);
}

float Input::GetMouseWheelDelta()
{
    ASSERT(s_Mouse, "Mouse not assigned!");
    return s_Mouse->GetWheelDelta();
}

bool Input::GetMouseButton(MouseCode mouseCode)
{
    ASSERT(s_Mouse, "Mouse not assigned!");
    return s_Mouse->GetButton(mouseCode);
}

bool Input::GetMouseButtonDown(MouseCode mouseCode)
{
    ASSERT(s_Mouse, "Mouse not assigned!");
    return s_Mouse->GetButtonDown(mouseCode);
}

bool Input::GetMouseButtonUp(MouseCode mouseCode)
{
    ASSERT(s_Mouse, "Mouse not assigned!");
    return s_Mouse->GetButtonUp(mouseCode);
}

void Input::Update()
{
    s_Keyboard->Update();
    s_Mouse->Update();
    // TODO: Other inputs (GamePad, etc.)
}