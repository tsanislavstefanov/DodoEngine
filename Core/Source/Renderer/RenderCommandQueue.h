#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND QUEUE ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderCommandQueue
    {
    public:
        RenderCommandQueue() = default;

        template<typename RenderCommand>
        void Submit(RenderCommand&& cmd)
        {
            m_Commands.push_back(std::forward<RenderCommand>(cmd));
        }

        void Execute();

        uint32_t GetCommandCount() const
        {
            return (uint32_t)m_Commands.size();
        }

    private:
        ////////////////////////////////////////////////////////////
        // COMMAND /////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////
        
        using Command = std::function<void()>;
        std::deque<Command> m_Commands{};
    };

}