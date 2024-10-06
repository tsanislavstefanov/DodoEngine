#include "pch.h"
#include "render_thread.h"

namespace Dodo {

    void RenderThread::start(RenderThreadPolicy policy /* = RenderThreadPolicy::single_threaded */)
    {
        policy_ = policy;
        is_running_ = true;
        if (policy_ == RenderThreadPolicy::multi_threaded)
        {
            thread_ = Thread::create("Render Thread", ThreadAffinity::render_thread);
            thread_->dispatch(&RenderThread::main, this);
        }
    }

    void RenderThread::wait_on_render_complete()
    {
        if (policy_ == RenderThreadPolicy::multi_threaded)
        {
            wait_for(State::idle);
        }
    }

    void RenderThread::submit(RenderCommand&& command, bool execute_immediately)
    {
        RenderCommandQueue& submission_queue = command_queues_.at(submission_queue_index_);
        submission_queue.enqueue(std::forward<RenderCommand>(command), execute_immediately);
    }

    void RenderThread::flush_all()
    {
        submission_queue_index_ = (submission_queue_index_ + 1) % command_queues_.size();
        do_render();
    }

    void RenderThread::stop()
    {
        is_running_ = false;
        pump();
        if (policy_ == RenderThreadPolicy::multi_threaded)
        {
            thread_->join();
        }
    }

    void RenderThread::main(RenderThread* render_thread)
    {
        while (render_thread->is_running_)
        {
            render_thread->render();
        }
    }

    void RenderThread::pump()
    {
        flush_all();
        wait_on_render_complete();
    }

    void RenderThread::do_render()
    {
        if (policy_ == RenderThreadPolicy::multi_threaded)
        {
            transition_to(State::kick);
        }
        else
        {
            render();
        }
    }

    void RenderThread::render()
    {
        Stopwatch wait_stopwatch{};
        wait_for(State::kick);
        performance_stats_.wait_time = wait_stopwatch.milliseconds();
        transition_to(State::busy);

        RenderCommandQueue& render_queue = command_queues_.at(render_queue_index());
        const uint32_t command_count = render_queue.command_count();
        Stopwatch work_stopwatch{};
        render_queue.execute_all();
        performance_stats_.work_time = work_stopwatch.milliseconds();
        performance_stats_.executed_command_count = command_count;
        transition_to(State::idle);
    }

    void RenderThread::wait_for(State state)
    {
        if (policy_ == RenderThreadPolicy::multi_threaded)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            state_changed_.wait(lock, [this, awaited_state = state]() -> bool {
                return state_ == awaited_state;
            });
        }
    }

    void RenderThread::transition_to(State state)
    {
        if (policy_ == RenderThreadPolicy::multi_threaded)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            state_ = state;
            state_changed_.notify_all();
        }
    }

    size_t RenderThread::render_queue_index() const
    {
        return (submission_queue_index_ + 1) % command_queues_.size();
    }
}