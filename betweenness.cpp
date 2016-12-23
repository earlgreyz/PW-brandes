#include "betweenness.h"
#include "thread_pool.h"

#include <stack>
#include <queue>

namespace {
    void worker(Brandes::Graph &graph, const std::shared_ptr<Brandes::Node> &node) {
        std::stack<std::shared_ptr<Brandes::Node>> stack;
        std::queue<std::shared_ptr<Brandes::Node>> queue;
        std::vector<std::shared_ptr<Brandes::Node>> predecessors[graph.get_size()];
        size_t shortest_paths[graph.get_size()];
        long long distance[graph.get_size()];
        Brandes::WeightType delta[graph.get_size()];

        for (size_t i = 0; i < graph.get_size(); i++) {
            shortest_paths[i] = 0;
            distance[i] = -1;
            delta[i] = 0;
        }

        shortest_paths[node->get_order()] = 1;
        distance[node->get_order()] = 0;
        queue.emplace(node);

        while (!queue.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = queue.front();
            queue.pop();
            stack.push(current_node);
            // TODO: to też w osobnych wątkach
            for (auto &neighbour : current_node->get_neighbours()) {
                if (distance[neighbour->get_order()] < 0) {
                    queue.push(neighbour);
                    distance[neighbour->get_order()] = distance[current_node->get_order()] + 1;
                }
                if (distance[neighbour->get_order()] == distance[current_node->get_order()] + 1) {
                    shortest_paths[neighbour->get_order()] += shortest_paths[current_node->get_order()];
                    predecessors[neighbour->get_order()].push_back(current_node);
                }
            }
        }

        while (!stack.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = stack.top();
            stack.pop();
            for (auto &neighbour : predecessors[current_node->get_order()]) {
                delta[neighbour->get_order()] += shortest_paths[neighbour->get_order()] / shortest_paths[current_node->get_order()] * (1 + delta[current_node->get_order()]);
            }
            if (current_node->get_order() != node->get_order()) {
                *current_node += delta[current_node->get_order()];
            }
        }
    }
}

namespace Brandes {
    void betweenness(const size_t &threads_count, Graph &graph) {
        ThreadPool thread_pool{ threads_count };
        graph.clear_weights();

        for (const auto &node : graph.get_nodes()) {
            thread_pool.add(worker, graph, node.second);
        }
    }
}
