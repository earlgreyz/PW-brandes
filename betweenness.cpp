#include "betweenness.h"
#include "thread_pool.h"

namespace Brandes {
    void betweenness(const size_t &threads_count, Graph &graph) {
        ThreadPool thread_pool{ threads_count };
        graph.clear_weights();

    }
}
