#include "pch.h"
#include "RenderCommandQueue.h"

namespace Dodo {

    RenderCommandQueue::RenderCommandQueue()
    {
        // Improve performance and avoid some allocations and data
        // copying.
        static constexpr size_t defaultCapacity = 1024;
        m_Commands.reserve(defaultCapacity);
    }

    void RenderCommandQueue::Enqueue(RenderCommand&& command)
    {
        m_Commands.emplace_back(std::forward<RenderCommand>(command));
    }

    void RenderCommandQueue::Execute()
    {
        for (const RenderCommand& command : m_Commands)
            command();
    }

    void RenderCommandQueue::Clear()
    {
        m_Commands.clear();
    }

}