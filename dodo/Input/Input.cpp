#include "pch.h"
#include "Input.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // INPUT ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Keyboard* Input::Keyboard = nullptr;
    Mouse* Input::Mouse = nullptr;

    void Input::Update()
    {
        Keyboard->Update();
        Mouse->Update();
    }

}