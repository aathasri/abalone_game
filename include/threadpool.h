#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>
#include <type_traits>
#include <utility>

class ThreadPool {
public:
    // Constructor & Destructor
    ThreadPool(size_t threads);
    ~ThreadPool();

    // Enqueue a task into the thread pool.
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result<F, Args...>::type>;

private:
    // Worker threads.
    std::vector<std::thread> workers;
    // Task queue.
    std::queue<std::function<void()>> tasks;
    
    // Synchronization.
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// Template implementation: defined in the header so it can be instantiated.
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
  -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using return_type = typename std::invoke_result<F, Args...>::type;

    // Create a packaged task that wraps the function call.
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

#endif // THREAD_POOL_H