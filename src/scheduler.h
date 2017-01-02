#ifndef BRANDES_SCHEDULER_H
#define BRANDES_SCHEDULER_H


#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>
#include <type_traits>

namespace Synchronization {
    /**
     * Scheduler Scope interface.
     * @tparam T type of tasks to execute
     */
    template <typename T>
    class Scope {
    public:
        using Task = T;
        /**
         * Executes the given task.
         * @param task task to execute
         */
        virtual void execute(Task task) = 0;
    };

    /**
     * Creates Scopes instances and performs execute function for every
     * scheduledTask.
     * @tparam S subclass of the Scope
     * @tparam Args arguments for S constructor
     */
    template <typename S, typename ...Args>
    class Scheduler {
        static_assert(std::is_base_of<Scope<typename S::Task>, S>::value,
                      "Given Scope class must inherit from Scope base class!");
    private:
        using Task = typename S::Task;
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
        std::queue<Task> tasks;

        /**
         * Function performed by each thread.
         * @param scheduler pointer to `this`
         * @param args arguments to be passed to Scope constructor
         */
        static void worker(Scheduler<S, Args...> *scheduler, Args ...args);
    public:
        /**
         * Creates new scheduler.
         * @param threads_count number of threads to be used by scheduler.
         * @param args arguments to be passed to Scope constructor
         */
        Scheduler(std::size_t threads_count, Args ...args);
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
        void schedule(Task task);
        /**
         * Stops execution of thread that performed join until all scheduled
         * tasks are finished.
         */
        void join();
    };

    template <typename S, typename ...Args>
    void Scheduler<S, Args...>::worker(Scheduler<S, Args...> *scheduler, Args ...args) {
        S scope{ args... };
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

    template <typename S, typename ...Args>
    Scheduler<S, Args...>::Scheduler(std::size_t threads_count, Args ...args)
            : terminating(false) {
        threads.reserve(threads_count);
        for (std::size_t i = 0u; i < threads_count; i++) {
            threads.emplace_back(worker, this, args...);
        }
    }

    template <typename S, typename ...Args>
    Scheduler<S, Args...>::~Scheduler() {
        terminating = true;
        workers_condition.notify_all();

        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        wait_condition.notify_all();
    }

    template <typename S, typename ...Args>
    void Scheduler<S, Args...>::schedule(Task task) {
        std::lock_guard<std::mutex> lock{ mutex };
        tasks.push(task);
        workers_condition.notify_one();
    }

    template <typename S, typename ...Args>
    void Scheduler<S, Args...>::join() {
        std::unique_lock<std::mutex> lock{ mutex };
        if (!tasks.empty()) {
            wait_condition.wait(lock, [&] {
                return tasks.empty();
            });
        }
    }
}


#endif //BRANDES_SCHEDULER_H
