#pragma once

#include "render_command_queue.h"
#include "core/thread_pool.h"

namespace Dodo {

    class RenderThread
    {
    public:
        enum class Policy {
            multi_threaded,
            auto_count,
            none
        };

        RenderThread(Policy p_thread_policy = Policy::none);

        void start();
        void pump();
        void render();
        void wait_on_render_complete();
        template<typename T>
        void enqueue_command(T* instance, void(T::*member)());
        void stop();

    protected:
        enum class State
        {
            idle,
            flush,
            busy,
            auto_count,
            none
        };

        void thread_loop();

        void wait_for_state(State state);
        void transition_to_state(State state);
        void flush_all();
        inline size_t get_render_queue_index() const { return (submission_queue_index + 1) % command_queues.size(); }

        bool is_running = false;
        ThreadPool::TaskId task_id = ThreadPool::invalid_task_id;
        Policy thread_policy = Policy::none;
        std::mutex mutex{};
        State state = State::idle;
        std::condition_variable condition_var{};
        static constexpr size_t max_command_queue_count = 2;
        std::array<RenderCommandQueue, max_command_queue_count> command_queues{};
        size_t submission_queue_index = 0;
    };

    template<typename T>
    inline void RenderThread::enqueue_command(T* instance, void(T::*member)()) {
        command_queues.at(submission_queue_index).enqueue(instance, member);
    }

}