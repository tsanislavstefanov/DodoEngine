#pragma once

#include "render_command_queue.h"
#include "concurrency/thread.h"

namespace Dodo {

    enum class RenderThreadPolicy
    {
        single_threaded,
        multi_threaded,
        auto_count,
        none
    };

    class RenderThread
    {
    public:
        struct PerformanceStats
        {
            std::atomic<double> wait_time = 0.0;
            std::atomic<double> work_time = 0.0;
            std::atomic<uint32_t> executed_command_count = 0;
        };

        RenderThread() = default;

        void start(RenderThreadPolicy policy = RenderThreadPolicy::single_threaded);
        void wait_on_render_complete();
        void submit(RenderCommand&& command, bool execute_immediately);
        void flush_all();
        void stop();

        inline const PerformanceStats& performance_stats() const
        {
            return performance_stats_;
        }

    private:
        enum class State
        {
            idle,
            kick,
            busy,
            auto_count,
            none
        };

        static void main(RenderThread* render_thread);

        void pump();
        void do_render();
        void render();
        void wait_for(State state);
        void transition_to(State state);
        size_t render_queue_index() const;

        RenderThreadPolicy policy_ = RenderThreadPolicy::none;
        bool is_running_ = false;
        Ref<Thread> thread_ = nullptr;
        std::mutex mutex_{};
        State state_ = State::idle;
        std::condition_variable state_changed_{};
        static constexpr auto max_command_queue_count = 2;
        std::array<RenderCommandQueue, max_command_queue_count> command_queues_{};
        size_t submission_queue_index_ = 0;
        PerformanceStats performance_stats_{};
    };

}