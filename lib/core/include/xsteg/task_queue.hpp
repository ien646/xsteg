#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include <queue>

namespace xsteg
{
    class task_queue
    {
    private:
        typedef std::function<void(void)> task_t;
        std::queue<task_t> _queue;

        int _max_threads = 0;

    public:
        task_queue(int max_threads = std::thread::hardware_concurrency());

        void enqueue(task_t);
        void run(bool run_detached);

    private:
        task_t dequeue_task_lock(std::mutex& queue_lock);
        size_t queue_size_lock(std::mutex& queue_lock);
    };
}