#ifndef BRANDES_BETWEENNESS_H
#define BRANDES_BETWEENNESS_H


#include <cstddef>
#include "graph.h"

namespace Brandes {
    void calculate_weights(const std::size_t &threads_count, Graph &graph);
}


#endif //BRANDES_BETWEENNESS_H
