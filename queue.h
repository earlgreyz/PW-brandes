#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace ThreadSafe {
    /**
     * Thread safe queue implementation
     * @tparam T stored type
     */
    template <typename T>
    class Queue {
    private:
        mutable std::mutex mutex;
        std::queue<T> queue;
        std::condition_variable condition;

    public:
        /**
         * Get the first value of the queue.
         * Blocks the thread until a value is available.
         * @return front of the queue
         */
        T pop() {
            std::unique_lock<std::mutex> lock{ mutex };
            condition.wait(lock, [&] {
                return !queue.empty();
            });
            T & value = std::move(queue.front());
            queue.pop();
            return value;
        }

        /**
         * Push a new value onto the queue.
         * Notifies a thread waiting on pop()
         * @param value
         */
        void push(T value) {
            std::lock_guard<std::mutex> lock{ mutex };
            queue.push(std::move(value));
            condition.notify_one();
        }

        /**
         * Checks whether or not the queue is empty.
         * @return
         */
        bool empty() const {
            std::lock_guard<std::mutex> lock{ mutex };
            return queue.empty();
        }
    };
}

#endif //THREADSAFE_QUEUE_H
