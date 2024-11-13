#include "pch.h"
#include "render_command_queue.h"

namespace Dodo {

    static inline constexpr size_t queue_page_size = 16_kb;

    RenderCommandQueue::RenderCommandQueue()
        : queue(queue_page_size, 0) {}

    void RenderCommandQueue::flush() {
        size_t read_cursor = 0;
        for (size_t i = 0; i < command_count; i++) {
            auto* command = reinterpret_cast<Command*>(queue.data() + read_cursor);
            command->execute();
            read_cursor += command_offsets.at(i);
        }

        offset = 0;
        command_count = 0;
    }

    void RenderCommandQueue::ensure_capacity(size_t size) {
        const size_t available = queue.capacity() - queue.size();
        if (available < size) {
            const size_t not_enough = size - available;
            queue.resize(queue.size() + queue_page_size + not_enough, 0);
        }
    }

}