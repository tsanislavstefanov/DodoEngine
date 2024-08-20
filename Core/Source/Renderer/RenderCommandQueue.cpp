#include "pch.h"
#include "RenderCommandQueue.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND QUEUE ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void RenderCommandQueue::Execute()
    {
        for (const RenderCommand& cmd : m_Commands)
        {
            cmd();
        }

        m_Commands.clear();
    }

}