#include "pch.h"
#include "render_thread.h"

namespace Dodo {

    RenderThread::RenderThread(Policy thread_policy)
        : thread_policy{thread_policy} {}

    void RenderThread::start() {
        is_running = true;
        if (thread_policy == Policy::multi_threaded) {
            task_id = ThreadPool::get_singleton()->add_task([this](auto) { thread_loop(); }, "Render Thread");
        }
    }

    void RenderThread::pump() {
        render();
        wait_on_render_complete();
    }

    void RenderThread::render() {
        submission_queue_index = (submission_queue_index + 1) % command_queues.size();
        if (thread_policy == Policy::multi_threaded) {
            transition_to_state(State::flush);
        }
        else {
            flush_all();
        }
    }

    void RenderThread::wait_on_render_complete() {
        if (thread_policy == Policy::multi_threaded) {
            wait_for_state(State::idle);
        }
    }

    void RenderThread::stop() {
        is_running = false;
        pump();
    }

    void RenderThread::thread_loop() {
        while (is_running) {
            flush_all();
        }
    }

    void RenderThread::wait_for_state(State state) {
        std::unique_lock<std::mutex> lock(mutex);
        condition_var.wait(lock, [this, state]() -> bool { return state == state; });
    }

    void RenderThread::transition_to_state(State state) {
        std::unique_lock<std::mutex> lock(mutex);
        state = state;
        condition_var.notify_all();
    }

    void RenderThread::flush_all() {
        wait_for_state(State::flush);
        transition_to_state(State::busy);
        RenderCommandQueue& queue = command_queues.at(get_render_queue_index());
        queue.flush();
        // Render done!
        transition_to_state(State::idle);
    }

}