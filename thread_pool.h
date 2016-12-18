#ifndef THREAD_POOL_H
#define THREAD_POOL_H


#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include <vector>
#include <functional>
#include <condition_variable>
#include <queue>


class ThreadPool {
private:
    using Job = std::function<void()>;

    // function each thread performs
    static void worker(ThreadPool *pool);

    std::queue<Job> jobs;
    mutable std::mutex jobs_mutex;

    // notification variable for waiting threads
    std::condition_variable jobs_available;

    std::vector<std::thread> threads;
    std::atomic<std::size_t> threads_waiting;

    std::atomic<bool> terminate;
    std::atomic<bool> paused;

public:
    using ids_vector = std::vector<std::thread::id>;

    /**
     * Starts get_threads_count threads, waiting for jobs.
     * May throw a std::system_error if a thread could not be started.
     * @param threads_count number of threads to be used in the pool
     */
    ThreadPool(std::size_t threads_count = std::max(std::thread::hardware_concurrency(), 2u) - 1u);

    // non-copyable,
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // but movable
    ThreadPool(ThreadPool&&) = default;
    ThreadPool& operator=(ThreadPool&&) = default;

    /**
     * Clears job queue.
     * Blocks until all threads are finished executing their current job
     */
    ~ThreadPool();

    /**
     * Add a function to be executed
     * @tparam Func function type
     * @tparam Args function argument types
     * @param func function
     * @param args function arguments
     * @return future of function return value
     */
    template<typename Func, typename... Args>
    auto add(Func&& func, Args&&... args) -> std::future<typename std::result_of<Func(Args...)>::type>;

    /**
     * Returns number of threads in the pool,
     * @return number of threads
     */
    std::size_t get_threads_count() const;

    /**
     * Returns the number of jobs waiting to be executed.
     * @return number of jobs waiting
     */
    std::size_t get_waiting_jobs_count() const;

    /**
     * Returns a vector of ids of the threads used by the ThreadPool.
     * @return vector of ids
     */
    ids_vector get_ids() const;

    /**
     * Clears currently queued jobs.
     * Does not affect currently running jobs.
     */
    void clear();

    /**
     * Pause and resume job execution.
     * Does not affect currently running jobs.
     * @param state
     */
    void pause(bool state);

    /**
     * Blocks calling thread until job queue is empty
     */
    void wait();
};

template<typename Func, typename... Args>
auto ThreadPool::add(Func&& func, Args&&... args) -> std::future<typename std::result_of<Func(Args...)>::type> {
    using PackedTask = std::packaged_task<typename std::result_of<Func(Args...)>::type()>;
    auto task = std::make_shared<PackedTask>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

    auto future = task.get()->get_future();

    {
        std::lock_guard<std::mutex> lock(jobs_mutex);
        jobs.emplace([task]() {
            (*task.get())();
        });
    }

    // let a waiting thread know there is an available job
    jobs_available.notify_one();

    return future;
}


#endif //THREAD_POOL_H
