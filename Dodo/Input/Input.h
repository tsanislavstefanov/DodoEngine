#pragma once

#include "Keyboard.h"
#include "Mouse.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // INPUT ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Input
    {
    public:
        static bool GetKey(KeyCode keyCode)
        {
            return Keyboard->GetKey(keyCode);
        }
                    
        static bool GetKeyDown(KeyCode keyCode)
        {
            return Keyboard->GetKeyDown(keyCode);
        }
                    
        static bool GetKeyUp(KeyCode keyCode)
        {
            return Keyboard->GetKeyUp(keyCode);
        }
                    
        static float GetMouseWheelDelta()
        {
            return Mouse->GetWheelDelta();
        }
                    
        static bool GetMouseButton(MouseCode mouseCode)
        {
            return Mouse->GetButton(mouseCode);
        }
                    
        static bool GetMouseButtonDown(MouseCode mouseCode)
        {
            return Mouse->GetButtonDown(mouseCode);
        }
                    
        static bool GetMouseButtonUp(MouseCode mouseCode)
        {
            return Mouse->GetButtonUp(mouseCode);
        }

        static void Update();

        virtual ~Input() = default;

    protected:
        Input() = default;

        static Keyboard* Keyboard;
        static Mouse* Mouse;
    };

}