#pragma once

#include "KeyCode.h"
#include "MouseCode.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // FORWARD DECLARATIONS ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Keyboard;
    class Mouse;

    ////////////////////////////////////////////////////////////////
    // INPUT ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Input
    {
    public:
        static Keyboard* Keyboard;
        static Mouse*    Mouse;

        static bool  GetKey(KeyCode keyCode);
                    
        static bool  GetKeyDown(KeyCode keyCode);
                    
        static bool  GetKeyUp(KeyCode keyCode);
                    
        static float GetMouseWheelDelta();
                    
        static bool  GetMouseButton(MouseCode mouseCode);
                    
        static bool  GetMouseButtonDown(MouseCode mouseCode);
                    
        static bool  GetMouseButtonUp(MouseCode mouseCode);

        static void  Update();

        virtual ~Input() = default;
    };

}