#include "countdown_latch.h"

namespace Synchronization {
    CountDownLatch::CountDownLatch(size_t count): count(count) {

    }

    void CountDownLatch::count_down() {
        std::unique_lock<std::mutex> lock(mutex);
        if (--count == 0) {
            condition.notify_all();
        }
    }

    void CountDownLatch::await() {
        std::unique_lock<std::mutex> lock(mutex);
        if (count > 0) {
            condition.wait(lock, [&]{ return count == 0; });
        }
    }
}
