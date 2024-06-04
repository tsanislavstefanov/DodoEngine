#pragma once

#include "Bindings/Func.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER FUNC /////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    using RenderFunc = Func<void(), sizeof(void*) + sizeof(uint64_t)>;

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND QUEUE ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderCommandQueue
    {
    public:
        RenderCommandQueue() = default;

        uint32_t GetCommandCount() const
        {
            return m_Commands.size();
        }

        template<typename Command>
        void Submit(Command&& command)
        {
            m_Commands.push_back(std::move(command));
        }

        void Execute();
    private:
        std::deque<RenderFunc> m_Commands{};
    };

}