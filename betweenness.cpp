#include "betweenness.h"

#include <stack>
#include <queue>
#include <iostream>
#include <mutex>

#include "thread_pool.h"
#include "countdown_latch.h"

namespace {
    class BrandesWorker {
    private:
        Brandes::Graph &graph;
        std::stack<std::shared_ptr<Brandes::Node>> stack;
        std::queue<std::shared_ptr<Brandes::Node>> queue;

        std::vector<std::vector<size_t>> predecessors;
        std::vector<size_t> shortest_paths;
        std::vector<long long> distance;
        std::vector<Brandes::WeightType> delta;

        std::mutex mutex;
        void init(const std::shared_ptr<Brandes::Node> &node);
        void apply(const std::shared_ptr<Brandes::Node> &node);
    public:
        BrandesWorker(Brandes::Graph &graph);
        void compute(const std::shared_ptr<Brandes::Node> &node);
    };

    BrandesWorker::BrandesWorker(Brandes::Graph &graph) : graph(graph) {
        size_t graph_size = graph.get_size();
        predecessors = std::vector<std::vector<size_t>>(graph_size);
        shortest_paths = std::vector<size_t>(graph_size);
        distance = std::vector<long long>(graph_size);
        delta = std::vector<Brandes::WeightType >(graph_size);
    }

    void BrandesWorker::init(const std::shared_ptr<Brandes::Node> &node) {
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

    void BrandesWorker::compute(const std::shared_ptr<Brandes::Node> &node) {
        init(node);

        while (!queue.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = queue.front();
            queue.pop();
            stack.push(current_node);

            Synchronization::CountDownLatch countdown_latch{ current_node->get_neighbours().size() };
            for (const auto &neighbour_weak : current_node->get_neighbours()) {
                std::shared_ptr<Brandes::Node> neighbour = neighbour_weak.lock();
                if (distance[neighbour->get_order()] < 0) {
                    queue.push(neighbour);
                    distance[neighbour->get_order()] = distance[current_node->get_order()] + 1;
                }

                if (distance[neighbour->get_order()] == distance[current_node->get_order()] + 1) {
                    shortest_paths[neighbour->get_order()] += shortest_paths[current_node->get_order()];
                    predecessors[neighbour->get_order()].push_back(current_node->get_order());
                }
            }
        }

        apply(node);
    }

    void BrandesWorker::apply(const std::shared_ptr<Brandes::Node> &node) {
        while (!stack.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = stack.top();
            stack.pop();

            for (const auto &predecessor : predecessors[current_node->get_order()]) {
                size_t short_p = shortest_paths[predecessor];
                size_t short_c = shortest_paths[current_node->get_order()];
                Brandes::WeightType delta_c = delta[current_node->get_order()];
                delta[predecessor] += (short_p / short_c) * (1 + delta_c);
            }

            if (current_node->get_order() != node->get_order()) {
                *current_node += delta[current_node->get_order()];
            }
        }
    }
}

namespace Brandes {
    void betweenness(const size_t &threads_count, Graph &graph) {
        Synchronization::ThreadPool thread_pool{ threads_count };
        graph.clear_weights();

        for (const auto &node : graph.get_nodes()) {
            std::shared_ptr<BrandesWorker> worker = std::make_shared<BrandesWorker>(graph);
            thread_pool.add([worker, &node] {
                return worker->compute(node.second);
            });
        }
        thread_pool.wait();
    }
}
