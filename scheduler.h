#ifndef BRANDES_SCHEDULER_H
#define BRANDES_SCHEDULER_H


#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <queue>

template <typename Scope, typename ScopeArgs, typename Task>
class Scheduler {
private:
    ScopeArgs scope_args;

    std::atomic_bool terminating;
    std::mutex mutex;
    std::condition_variable workers_condition;
    std::condition_variable wait_condition;

    std::vector<std::thread> threads;
    std::queue<Task> tasks;

    static void worker(Scheduler<Scope, ScopeArgs, Task>* scheduler);
public:
    Scheduler(std::size_t threads_count, ScopeArgs args);
    ~Scheduler();

    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler &) = delete;
    Scheduler(Scheduler &&) = default;
    Scheduler &operator=(Scheduler &&) = default;

    void schedule(Task task);
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
