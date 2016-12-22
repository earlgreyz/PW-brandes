#ifndef BRANDES_BETWEENNESS_H
#define BRANDES_BETWEENNESS_H

#include <cstddef>
#include "graph.h"

namespace Brandes {
    void betweenness(const size_t &threads_count, const Graph &graph);
}


#endif //BRANDES_BETWEENNESS_H
