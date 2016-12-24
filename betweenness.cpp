#include "betweenness.h"

#include <stack>
#include <queue>
#include <iostream>
#include <mutex>
#include <functional>

#include "thread_pool.h"
#include "countdown_latch.h"

namespace {

    struct BrandesData {
        std::stack<std::shared_ptr<Brandes::Node>> stack;
        std::queue<std::shared_ptr<Brandes::Node>> queue;

        std::vector<std::vector<size_t>> predecessors;
        std::vector<size_t> shortest_paths;
        std::vector<long long> distance;
        std::vector<Brandes::WeightType> delta;

        std::mutex mutex;

        BrandesData(size_t graph_size) {
            predecessors = std::vector<std::vector<size_t>>(graph_size);
            shortest_paths = std::vector<size_t>(graph_size);
            distance = std::vector<long long>(graph_size);
            delta = std::vector<Brandes::WeightType >(graph_size);

            for (size_t i = 0; i < graph_size; i++) {
                shortest_paths[i] = 0;
                distance[i] = -1;
                delta[i] = 0;
            }
        }
    };

    void neighbour_worker(Synchronization::CountDownLatch &countdown_latch,
                          BrandesData &_,
                          const std::shared_ptr<Brandes::Node> &current_node,
                          const std::shared_ptr<Brandes::Node> &neighbour) {

        if (_.distance[neighbour->get_order()] < 0) {
            {
                std::unique_lock<std::mutex> lock(_.mutex);
                _.queue.push(neighbour);
            }
            _.distance[neighbour->get_order()] = _.distance[current_node->get_order()] + 1;
        }

        if (_.distance[neighbour->get_order()] == _.distance[current_node->get_order()] + 1) {
            _.shortest_paths[neighbour->get_order()] += _.shortest_paths[current_node->get_order()];
            _.predecessors[neighbour->get_order()].push_back(current_node->get_order());
        }

        countdown_latch.count_down();
    }

    void worker(Synchronization::ThreadPool &thread_pool,
                Brandes::Graph &graph,
                const std::shared_ptr<Brandes::Node> &node) {

        BrandesData _(graph.get_size());
        _.shortest_paths[node->get_order()] = 1;
        _.distance[node->get_order()] = 0;
        _.queue.push(node);

        while (!_.queue.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = _.queue.front();
            _.queue.pop();
            _.stack.push(current_node);
            Synchronization::CountDownLatch countdown_latch{ current_node->get_neighbours().size() };
            for (const auto &neighbour_weak : current_node->get_neighbours()) {
                std::shared_ptr<Brandes::Node> neighbour = neighbour_weak.lock();
                thread_pool.add(
                        neighbour_worker,
                        std::reference_wrapper<Synchronization::CountDownLatch>(countdown_latch),
                        std::reference_wrapper<BrandesData>(_),
                        std::reference_wrapper<const std::shared_ptr<Brandes::Node>>(current_node),
                        std::reference_wrapper<const std::shared_ptr<Brandes::Node>>(neighbour)
                );
            }
            countdown_latch.await();
        }

        while (!_.stack.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = _.stack.top();
            _.stack.pop();
            for (const auto &predecessor : _.predecessors[current_node->get_order()]) {
                _.delta[predecessor] += _.shortest_paths[predecessor] / _.shortest_paths[current_node->get_order()] * (1 + _.delta[current_node->get_order()]);
            }
            if (current_node->get_order() != node->get_order()) {
                *current_node += _.delta[current_node->get_order()];
            }
        }
    }
}

namespace Brandes {
    void betweenness(const size_t &threads_count, Graph &graph) {
        Synchronization::ThreadPool thread_pool{ threads_count };
        graph.clear_weights();

        for (const auto &node : graph.get_nodes()) {
            thread_pool.add(
                    worker,
                    std::reference_wrapper<Synchronization::ThreadPool>(thread_pool),
                    std::reference_wrapper<Graph>(graph),
                    std::reference_wrapper<const std::shared_ptr<Node>>(node.second)
            );
        }
        thread_pool.wait();
    }
}
