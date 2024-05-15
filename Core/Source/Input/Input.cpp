#include "pch.h"
#include "Input.h"
#include "Keyboard.h"
#include "Mouse.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // INPUT ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Keyboard* Input::Keyboard = nullptr;
    Mouse*    Input::Mouse    = nullptr;

    bool Input::GetKey(KeyCode keyCode)
    {
        ASSERT(Keyboard, "Keyboard not assigned!");
        return Keyboard->GetKey(keyCode);
    }

    bool Input::GetKeyDown(KeyCode keyCode)
    {
        ASSERT(Keyboard, "Keyboard not assigned!");
        return Keyboard->GetKeyDown(keyCode);
    }

    bool Input::GetKeyUp(KeyCode keyCode)
    {
        ASSERT(Keyboard, "Keyboard not assigned!");
        return Keyboard->GetKeyUp(keyCode);
    }

    float Input::GetMouseWheelDelta()
    {
        ASSERT(Mouse, "Mouse not assigned!");
        return Mouse->GetWheelDelta();
    }

    bool Input::GetMouseButton(MouseCode mouseCode)
    {
        ASSERT(Mouse, "Mouse not assigned!");
        return Mouse->GetButton(mouseCode);
    }

    bool Input::GetMouseButtonDown(MouseCode mouseCode)
    {
        ASSERT(Mouse, "Mouse not assigned!");
        return Mouse->GetButtonDown(mouseCode);
    }

    bool Input::GetMouseButtonUp(MouseCode mouseCode)
    {
        ASSERT(Mouse, "Mouse not assigned!");
        return Mouse->GetButtonUp(mouseCode);
    }

    void Input::Update()
    {
        Keyboard->Update();
        Mouse->Update();
        // TODO: Other inputs (GamePad, etc.)
    }

}