#include <xsteg/task_queue.hpp>

#include <unordered_map>

namespace xsteg
{
    task_queue::task_queue(int max_threads)
    {
        _max_threads = max_threads;
    }

    void task_queue::enqueue(task_t task)
    {
        _queue.push(task);
    }

    void task_queue::run(bool run_detached)
    {
        std::mutex queue_lock;

        std::mutex task_counter_lock;
        int task_counter = 0;

        std::function<int(void)> currently_running_tasks = [&]()
        {
            std::lock_guard lock(task_counter_lock);
            return task_counter;
        };

        std::function<void(void)> inc_currently_running_tasks = [&]()
        {
            std::lock_guard lock(task_counter_lock);
            ++task_counter;
        };

        std::function<void(void)> dec_currently_running_tasks = [&]()
        {
            std::lock_guard lock(task_counter_lock);
            --task_counter;
        };

        std::thread th([&]()
        {
            int task_idx = 0;
            std::unordered_map<int, std::thread> task_threads;
            while(queue_size_lock(queue_lock) > 0)
            {
                while(currently_running_tasks() < _max_threads)
                {
                    task_threads.emplace(task_idx++, std::thread([&]()
                    {
                        task_t tsk = dequeue_task_lock(queue_lock);
                        inc_currently_running_tasks();
                        tsk();
                        dec_currently_running_tasks();
                    }));
                }
                std::this_thread::yield();
            }
            /* Await remaining tasks */
            for(auto& tsk : task_threads)
            {
                tsk.second.join();
            }
        });
        
        if(!run_detached)
        {
            th.join();
        }
    }

    task_queue::task_t task_queue::dequeue_task_lock(std::mutex& queue_lock)
    {
        queue_lock.lock();
        task_t tsk = _queue.front();
        _queue.pop();
        queue_lock.unlock();
        return tsk;
    }

    size_t task_queue::queue_size_lock(std::mutex& queue_lock)
    {
        std::lock_guard lock(queue_lock);
        return _queue.size();
    }
}