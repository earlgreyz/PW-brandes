#include "thread_pool.h"
#include <algorithm>

namespace Synchronization {
    ThreadPool::ThreadPool(std::size_t threads_count)
            : threads_waiting(0u), terminate(false), paused(false) {

        threads.reserve(threads_count);
        for (auto i = 0u; i < threads_count; i++) {
            threads.emplace_back(worker, this);
        }
    }

    ThreadPool::~ThreadPool() {
        clear();

        // tell threads to stop when they can
        terminate = true;
        jobs_available.notify_all();

        // wait for all threads to finish
        for (auto &t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    std::size_t ThreadPool::get_threads_count() const {
        return threads.size();
    }

    std::size_t ThreadPool::get_waiting_jobs_count() const {
        std::lock_guard<std::mutex> lock(jobs_mutex);
        return jobs.size();
    }

    ThreadPool::ids_vector ThreadPool::get_ids() const {
        ids_vector vector(threads.size());
        std::transform(threads.begin(), threads.end(), vector.begin(), [](auto &thread) {
            return thread.get_id();
        });

        return vector;
    }

    void ThreadPool::clear() {
        std::lock_guard<std::mutex> lock(jobs_mutex);

        while (!jobs.empty()) {
            jobs.pop();
        }
    }

    void ThreadPool::pause(bool state) {
        paused = state;

        if (!state) {
            jobs_available.notify_all();
            // block until at least 1 thread has started
            while (threads_waiting == threads.size());
        }
    }

    void ThreadPool::wait() {
        // we're done waiting once all threads are waiting
        while (threads_waiting != threads.size());
    }

    void ThreadPool::worker(ThreadPool *pool) {
        while (true) {
            if (pool->terminate) {
                break;
            }

            std::unique_lock<std::mutex> lock(pool->jobs_mutex);

            // if there are no more jobs, or we're paused, go into waiting mode
            if (pool->jobs.empty() || pool->paused) {
                ++pool->threads_waiting;
                pool->jobs_available.wait(lock, [&]() {
                    return pool->terminate || !(pool->jobs.empty() || pool->paused);
                });
                --pool->threads_waiting;
            }

            // last check before grabbing a job
            if (pool->terminate) {
                break;
            }

            // take next job
            auto job = std::move(pool->jobs.front());
            pool->jobs.pop();

            lock.unlock();

            job();
        }
    }
}