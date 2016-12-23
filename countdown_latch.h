#ifndef SYNCHRONIZATION_COUNTDOWN_LATCH_H
#define SYNCHRONIZATION_COUNTDOWN_LATCH_H


#include <cstddef>
#include <condition_variable>
#include <mutex>

namespace Synchronization {
    class CountDownLatch {
    private:
        size_t count;
        std::mutex mutex;
        std::condition_variable condition;
    public:
        CountDownLatch(size_t count);
        void count_down();
        void await();
    };
}


#endif //BRANDES_COUNTDOWN_LATCH_H
