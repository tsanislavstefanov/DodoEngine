#include "pch.h"
#include "RenderCommandQueue.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND BUFFER ///////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void RenderCommandQueue::Execute()
    {
        for (auto& command : m_Commands)
        {
            command.Invoke();
        }
    }

}