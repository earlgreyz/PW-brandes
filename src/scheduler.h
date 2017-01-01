#ifndef BRANDES_SCHEDULER_H
#define BRANDES_SCHEDULER_H


#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>

namespace Synchronization {
    /**
     * Creates Scopes instances and performs execute function for every
     * scheduledTask.
     * @tparam Scope class of the Scope
     * @tparam ScopeArgs argument of the Scope constructor
     */
    template <typename Scope, typename ...ScopeArgs>
    class Scheduler {
    private:
        /**
         * Marks if scheduler is about to finish.
         */
        std::atomic_bool terminating;
        /**
         * Mutex to guarantee thread safety.
         */
        std::mutex mutex;
        /**
         * Condition for workers to wait for tasks.
         */
        std::condition_variable workers_condition;
        /**
         * Condition for threads to wait for all workers to finish.
         */
        std::condition_variable wait_condition;

        /**
         * Vector of worker threads.
         */
        std::vector<std::thread> threads;
        /**
         * Queue of scheduled tasks.
         */
        std::queue<typename Scope::ScopeTask> tasks;

        /**
         * Function performed by each thread.
         * @param scheduler - pointer to `this`
         * @param args - arguments to be passed to Scope constructor
         */
        static void worker(Scheduler<Scope, ScopeArgs...>* scheduler,
                           ScopeArgs... args);
    public:
        /**
         * Creates new scheduler.
         * @param threads_count number of threads to be used by scheduler.
         * @param args arguments to be passed to Scope constructor
         */
        Scheduler(std::size_t threads_count, ScopeArgs ...args);
        ~Scheduler();

        // Non copyable
        Scheduler(const Scheduler &) = delete;
        Scheduler &operator=(const Scheduler &) = delete;
        // But movable
        Scheduler(Scheduler &&) = default;
        Scheduler &operator=(Scheduler &&) = default;

        /**
         * Schedule new task.
         * Scheduled task will be performed in FIFO order.
         * @param task task to be scheduled.
         */
        void schedule(typename Scope::ScopeTask task);
        /**
         * Stops execution of thread that performed join until all scheduled
         * tasks are finished.
         */
        void join();
    };

    template <typename Scope, typename ...ScopeArgs>
    void Scheduler<Scope, ScopeArgs...>
    ::worker(Scheduler<Scope, ScopeArgs...> *scheduler, ScopeArgs... args) {
        Scope scope{ args... };
        while (true) {
            std::unique_lock<std::mutex> lock{ scheduler->mutex };

            if (scheduler->tasks.empty()) {
                scheduler->wait_condition.notify_all();
                scheduler->workers_condition.wait(lock, [&] {
                    return !scheduler->tasks.empty() || scheduler->terminating;
                });
            }

            if (scheduler->terminating) {
                break;
            }

            auto task = std::move(scheduler->tasks.front());
            scheduler->tasks.pop();

            lock.unlock();
            scope.execute(task);
        }
    }

    template <typename Scope, typename ...ScopeArgs>
    Scheduler<Scope, ScopeArgs...>
    ::Scheduler(std::size_t threads_count, ScopeArgs ...args)
            : terminating(false) {
        threads.reserve(threads_count);
        for (std::size_t i = 0u; i < threads_count; i++) {
            threads.emplace_back(worker, this, args...);
        }
    }

    template <typename Scope, typename ...ScopeArgs>
    Scheduler<Scope, ScopeArgs...>::~Scheduler() {
        terminating = true;
        workers_condition.notify_all();

        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        wait_condition.notify_all();
    }

    template <typename Scope, typename ...ScopeArgs>
    void Scheduler<Scope, ScopeArgs...>
    ::schedule(typename Scope::ScopeTask task) {
        std::lock_guard<std::mutex> lock{ mutex };
        tasks.push(task);
        workers_condition.notify_one();
    }

    template <typename Scope, typename ...ScopeArgs>
    void Scheduler<Scope, ScopeArgs...>::join() {
        std::unique_lock<std::mutex> lock{ mutex };
        if (!tasks.empty()) {
            wait_condition.wait(lock, [&] {
                return tasks.empty();
            });
        }
    }
}


#endif //BRANDES_SCHEDULER_H
