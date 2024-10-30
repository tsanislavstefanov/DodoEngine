#pragma once

namespace Dodo {

    using RenderCommand = std::function<void()>;

    class RenderCommandQueue
    {
    public:
        RenderCommandQueue();

        void Enqueue(RenderCommand&& command);
        void Execute();
        void Clear();
        inline size_t GetCommandCount() const { return m_Commands.size(); }

    private:
        std::vector<RenderCommand> m_Commands{};
    };

}