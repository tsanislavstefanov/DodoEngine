#include "pch.h"
#include "render_command_queue.h"

namespace Dodo {

    RenderCommandQueue::RenderCommandQueue()
    {
        // Improve performance and avoid some allocations and data
        // copying.
        static constexpr auto default_capacity = 1024;
        commands_.reserve(default_capacity);
    }

    void RenderCommandQueue::enqueue(RenderCommand&& command, bool execute_immediately)
    {
        if (execute_immediately)
        {
            command();
        }
        else
        {
            commands_.emplace_back(std::forward<RenderCommand>(command));
        }
    }

    void RenderCommandQueue::execute_all()
    {
        for (const auto& command : commands_)
        {
            command();
        }

        commands_.clear();
    }

}