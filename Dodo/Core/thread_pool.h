#pragma once

#include "func.h"

namespace Dodo {

    class ThreadPool {
    public:
        using TaskId = int64_t;
        static constexpr TaskId invalid_task_id = static_cast<TaskId>(-1);
        using Callable = std::function<void(void*)>;

        static ThreadPool* get_singleton() { return instance; }

        ThreadPool();
        ~ThreadPool();

        TaskId add_task(Callable&& callable, const std::string& description = "", void* user_data = nullptr);
        void wait_on_task_to_complete(TaskId task_id);

    private:
        struct Task {
            TaskId id = 0;
            Callable callable{};
            std::string description{};
            void* user_data = nullptr;
            bool completed = false;
            std::mutex mutex{};
            std::condition_variable condition_var{};
            uint32_t waiting_user_count = 0;
        };

        void process_task(std::shared_ptr<Task> task);
        std::shared_ptr<Task> find_task(TaskId task_id);
        void remove_task(TaskId task_id);

        static inline ThreadPool* instance = nullptr;
        std::vector<std::thread> threads{};
        std::vector<std::thread::native_handle_type> thread_handles{};
        std::mutex mutex{};
        std::condition_variable condition_var{};
        bool stop = false;
        std::queue<std::shared_ptr<Task>> task_queue{};
        TaskId current_task_id = 0;
        std::unordered_map<TaskId, std::shared_ptr<Task>> tasks{};
    };

}