#include "betweenness.h"

#include <stack>
#include <queue>
#include <iostream>
#include <condition_variable>
#include <thread>
#include <atomic>

#include "scheduler.h"

namespace {
    class BrandesScope {
    private:
        Brandes::Graph &graph;
        std::stack<std::shared_ptr<Brandes::Node>> stack;
        std::queue<std::shared_ptr<Brandes::Node>> queue;

        std::vector<std::vector<size_t>> predecessors;
        std::vector<size_t> shortest_paths;
        std::vector<long long> distance;
        std::vector<Brandes::WeightType> delta;
        mutable std::mutex mutex;

        void init(const std::shared_ptr<Brandes::Node> &node);
        void apply(const std::shared_ptr<Brandes::Node> &node);
    public:
        BrandesScope(const BrandesScope&) = delete;
        BrandesScope(BrandesScope&&) = delete;
        BrandesScope(Brandes::Graph &graph);
        void execute(const std::shared_ptr<Brandes::Node> &node);
    };

    BrandesScope::BrandesScope(Brandes::Graph &graph) : graph(graph) {
        size_t graph_size = graph.get_size();
        predecessors = std::vector<std::vector<size_t>>(graph_size);
        shortest_paths = std::vector<size_t>(graph_size);
        distance = std::vector<long long>(graph_size);
        delta = std::vector<Brandes::WeightType >(graph_size);
    }

    void BrandesScope::init(const std::shared_ptr<Brandes::Node> &node) {
        for (size_t i = 0u; i < graph.get_size(); i++) {
            predecessors[i] = {};
            shortest_paths[i] = 0;
            distance[i] = -1;
            delta[i] = 0;
        }

        shortest_paths[node->get_order()] = 1;
        distance[node->get_order()] = 0;
        queue.push(node);
    }

    void BrandesScope::execute(const std::shared_ptr<Brandes::Node> &node) {
        std::lock_guard<std::mutex> lock(mutex);
        init(node);

        while (!queue.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = queue.front();
            size_t order_c = current_node->get_order();

            queue.pop();
            stack.push(current_node);

            for (const auto &neighbour_weak : current_node->get_neighbours()) {
                auto neighbour = neighbour_weak.lock();
                size_t order_n = neighbour->get_order();

                if (distance[order_n] < 0) {
                    queue.push(neighbour);
                    distance[order_n] = distance[order_c] + 1;
                }

                if (distance[order_n] == distance[order_c] + 1) {
                    shortest_paths[order_n] += shortest_paths[order_c];
                    predecessors[order_n].push_back(order_c);
                }
            }
        }

        apply(node);
    }

    void BrandesScope::apply(const std::shared_ptr<Brandes::Node> &node) {
        size_t order_n = node->get_order();

        while (!stack.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = stack.top();
            size_t order_c = current_node->get_order();

            stack.pop();

            for (const auto &predecessor : predecessors[order_c]) {
                Brandes::WeightType short_p = shortest_paths[predecessor];
                Brandes::WeightType short_c = shortest_paths[order_c];
                Brandes::WeightType delta_c = delta[order_c];
                delta[predecessor] += (short_p / short_c) * (1 + delta_c);
            }

            if (order_c != order_n) {
                current_node->increase_weight(delta[order_c]);
            }
        }
    }
}

namespace Brandes {
    void betweenness(const size_t &threads_count, Graph &graph) {
        graph.clear_weights();
        Scheduler<BrandesScope, Graph, std::shared_ptr<Node>>
                scheduler{ threads_count, std::reference_wrapper<Graph>(graph) };
        for (const auto &node : graph.get_nodes()) {
            scheduler.schedule(node.second);
        }
        scheduler.join();
    }
}
