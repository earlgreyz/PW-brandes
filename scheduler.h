#ifndef BRANDES_SCHEDULER_H
#define BRANDES_SCHEDULER_H


#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>

/**
 * Creates Scopes instances and performs execute function for every scheduled
 * Task.
 * @tparam Scope class to
 * @tparam ScopeArgs argument of the constructor - has to be given explicitly
 * (up until to c++17 it cannot be deduced from context)
 * @tparam Task type to the schedule (argument of Scope::execute function) -
 * has to be given explicitly
 */
template <typename Scope, typename ScopeArgs, typename Task>
class Scheduler {
private:
    /**
     * Argument to be passed Scope instances constructor
     */
    ScopeArgs scope_args;

    /**
     * Marks if scheduler is about to finish.
     */
    std::atomic_bool terminating;
    /**
     * Mutex to guarantee thread safety
     */
    std::mutex mutex;
    /**
     * Condition for workers to wait for tasks
     */
    std::condition_variable workers_condition;
    /**
     * Condition for threads to wait for all workers to finish
     */
    std::condition_variable wait_condition;

    /**
     * Vector of worker threads
     */
    std::vector<std::thread> threads;
    /**
     * Queue of scheduled tasks
     */
    std::queue<Task> tasks;

    /**
     * Function performed by each thread.
     * @param scheduler - pointer to `this`
     */
    static void worker(Scheduler<Scope, ScopeArgs, Task>* scheduler);
public:
    /**
     * Creates new scheduler.
     * @param threads_count - number of threads to be used by scheduler.
     * @param args - argument to be passed to Scope instances
     */
    Scheduler(std::size_t threads_count, ScopeArgs args);
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
     * @param task - task to be scheduled.
     */
    void schedule(Task task);
    /**
     * Stops execution of thread that performed join until all scheduled tasks
     * are finished.
     */
    void join();
};

template <typename Scope, typename ScopeArgs, typename Task>
void Scheduler<Scope, ScopeArgs, Task>::worker(Scheduler<Scope, ScopeArgs, Task> *scheduler) {
    Scope scope{ scheduler->scope_args };
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

template <typename Scope, typename ScopeArgs, typename Task>
Scheduler<Scope, ScopeArgs, Task>::Scheduler(std::size_t threads_count, ScopeArgs args)
        : scope_args(args) {
    threads.reserve(threads_count);
    for (std::size_t i = 0u; i < threads_count; i++) {
        threads.emplace_back(worker, this);
    }
}

template <typename Scope, typename ScopeArgs, typename Task>
Scheduler<Scope, ScopeArgs, Task>::~Scheduler() {
    terminating = true;
    workers_condition.notify_all();

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    wait_condition.notify_all();
}

template <typename Scope, typename ScopeArgs, typename Task>
void Scheduler<Scope, ScopeArgs, Task>::schedule(Task task) {
    std::lock_guard<std::mutex> lock{ mutex };
    tasks.push(task);
    workers_condition.notify_one();
}

template <typename Scope, typename ScopeArgs, typename Task>
void Scheduler<Scope, ScopeArgs, Task>::join() {
    std::unique_lock<std::mutex> lock{ mutex };
    if (!tasks.empty()) {
        wait_condition.wait(lock, [&] {
            return tasks.empty();
        });
    }
}


#endif //BRANDES_SCHEDULER_H
