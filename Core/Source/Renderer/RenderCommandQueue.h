#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    
    using RenderCommand = std::function<void()>;

    ////////////////////////////////////////////////////////////////
    // RENDER COMMAND QUEUE ////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderCommandQueue
    {
    public:
        RenderCommandQueue() = default;

        void Submit(const RenderCommand& cmd)
        {
            m_Commands.push_back(cmd);
        }

        void Execute();

        uint32_t GetCommandCount() const
        {
            return static_cast<uint32_t>(m_Commands.size());
        }

    private:
        std::deque<RenderCommand> m_Commands{};
    };

}