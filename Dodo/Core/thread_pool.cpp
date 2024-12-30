#include "pch.h"
#include "thread_pool.h"

namespace Dodo {

    ThreadPool::ThreadPool() {
        instance = this;
        const uint32_t max_thread_count = std::thread::hardware_concurrency();
        const auto thread_count = std::max(static_cast<uint32_t>(1), max_thread_count);
        threads.reserve(thread_count);
        for (size_t i = 0; i < thread_count; i++) {
            threads.emplace_back([this, i]() {
                std::shared_ptr<Task> task = nullptr;
                while (true) {
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        condition_var.wait(lock, [this]() -> bool { return !task_queue.empty() || stop; });
                        if (stop) {
                            return;
                        }

                        task = task_queue.front();
                        task_queue.pop();
                        tasks[task->id] = task;
                    }

                    process_task(task);
                }
            });

            thread_handles.push_back(threads.back().native_handle());
        }
    }

    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }

        condition_var.notify_all();
        for (std::thread& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    ThreadPool::TaskId ThreadPool::add_task(Callable&& callable, const std::string& description, void* user_data) {
        auto task = std::make_shared<Task>();
        task->id = current_task_id++;
        task->callable = callable;
        task->description = description;
        task->user_data = user_data;

        std::unique_lock<std::mutex> lock(mutex);
        task_queue.push(task);
        condition_var.notify_one();

        return task->id;
    }

    void ThreadPool::wait_on_task_to_complete(TaskId task_id) {
        std::shared_ptr<Task> task = find_task(task_id);
        if (task) {
            std::unique_lock<std::mutex> lock(task->mutex);
            task->waiting_user_count++;
            task->condition_var.wait(lock, [task]() -> bool { return task->completed; });
            remove_task(task_id);
        }
    }

    void ThreadPool::process_task(std::shared_ptr<Task> task) {
        task->callable(task->user_data);
        // Task done!
        task->completed = true;
        std::unique_lock<std::mutex> lock(task->mutex);
        if (task->waiting_user_count > 0) {
            task->condition_var.notify_all();
        }
    }

    std::shared_ptr<ThreadPool::Task> ThreadPool::find_task(TaskId task_id) {
        std::unique_lock<std::mutex> lock(mutex);
        return tasks.contains(task_id) ? tasks.at(task_id) : nullptr;
    }

    void ThreadPool::remove_task(TaskId task_id) {
        std::unique_lock<std::mutex> lock(mutex);
        if (tasks.contains(task_id)) {
            tasks.erase(task_id);
        }
    }

}