// threadpool.cpp

#include "threadpool.h"

ThreadPool::ThreadPool(size_t threads)
    : stop(false)
{
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this]() {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    // Wait until there is a task or until stop == true.
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });
                    // If cancellation is requested and there are no tasks left,
                    // exit immediately.
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                // Execute the task only if we're not cancelled in the middle.
                // (Optionally, tasks themselves should check for cancellation too.)
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    cancel();  // Cancel all pending tasks and signal threads to exit.
    for (std::thread &worker : workers)
        if (worker.joinable())
            worker.join();
}

// NEW: Cancel function that does immediate cancellation.
void ThreadPool::cancel() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
        // Clear all pending tasks from the queue.
        while (!tasks.empty()) {
            tasks.pop();
        }
    }
    // Notify all workers immediately so they can check the stop flag and exit.
    condition.notify_all();
}