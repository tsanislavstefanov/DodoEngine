#pragma once

namespace Dodo {

    using RenderCommand = std::function<void()>;

    class RenderCommandQueue
    {
    public:
        RenderCommandQueue();

        void enqueue(RenderCommand&& command, bool execute_immediately);
        void execute_all();
        inline uint32_t command_count() const { return (uint32_t)commands_.size(); }

    private:
        std::vector<RenderCommand> commands_{};
    };

}