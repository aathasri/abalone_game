#include "threadpool.h"

ThreadPool::ThreadPool(size_t threads)
    : stop(false), canceled(false)
{
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this]() {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    // Wait until there is a task, or stop is true, or cancellation is requested.
                    this->condition.wait(lock, [this] {
                        return this->stop || this->canceled.load() || !this->tasks.empty();
                    });
                    // If cancellation is requested or the pool is stopping and no tasks remain, exit.
                    if ((this->stop && this->tasks.empty()) || this->canceled.load())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
        if (worker.joinable())
            worker.join();
}

void ThreadPool::cancelTasks() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    // Set cancellation flag.
    canceled.store(true);
    // Clear all pending tasks.
    while (!tasks.empty()) {
        tasks.pop();
    }
    condition.notify_all();
}