#include "betweenness.h"

#include <stack>
#include <queue>
#include <iostream>
#include <condition_variable>
#include <thread>
#include <atomic>

#include "scheduler.h"

namespace {
    /**
     * Scope used to calculate weights.
     * @see Scheduler
     */
    class BrandesScope :
            public Synchronization::Scope<std::shared_ptr<Brandes::Node>> {
    private:
        /**
         * Reference to the graph
         */
        Brandes::Graph &graph;
        /**
         * Stack of Nodes
         */
        std::stack<std::shared_ptr<Brandes::Node>> stack;
        /**
         * Queue of nodes
         */
        std::queue<std::shared_ptr<Brandes::Node>> queue;

        /**
         * Predecessors of the node on all shortest paths
         */
        std::vector<std::vector<std::size_t>> predecessors;
        /**
         * Number of all shortest paths to the node
         */
        std::vector<std::size_t> shortest_paths;
        /**
         * Distance to the node
         */
        std::vector<long long> distance;
        /**
         * Betweenness value for the node
         */
        std::vector<Brandes::WeightType> delta;

        /**
         * Initializes arrays before performing calculations
         * @param node starting node
         */
        void init(const std::shared_ptr<Brandes::Node> &node);
        /**
         * Applies calculated weights to the graph
         * @param node starting node
         */
        void apply(const std::shared_ptr<Brandes::Node> &node);
    public:
        /**
         * Constructs new BrandesScope
         * @param graph graph for which to calculate betweenness
         */
        BrandesScope(Brandes::Graph &graph);
        /**
         * Calculates betweenness for the graph starting with given node
         * @param node starting node
         */
        void execute(std::shared_ptr<Brandes::Node> node);
    };

    BrandesScope::BrandesScope(Brandes::Graph &graph) : graph(graph) {
        std::size_t graph_size = graph.get_size();
        predecessors = std::vector<std::vector<std::size_t>>(graph_size);
        shortest_paths = std::vector<std::size_t>(graph_size);
        distance = std::vector<long long>(graph_size);
        delta = std::vector<Brandes::WeightType >(graph_size);
    }

    void BrandesScope::init(const std::shared_ptr<Brandes::Node> &node) {
        for (std::size_t i = 0u; i < graph.get_size(); i++) {
            predecessors[i].clear();
            shortest_paths[i] = 0;
            distance[i] = -1;
            delta[i] = 0;
        }

        shortest_paths[node->get_order()] = 1;
        distance[node->get_order()] = 0;
        queue.push(node);
    }

    void BrandesScope::execute(ScopeTask node) {
        init(node);

        while (!queue.empty()) {
            std::weak_ptr<Brandes::Node> current_node_weak = queue.front();
            auto current_node = current_node_weak.lock();

            std::size_t order_c = current_node->get_order();

            queue.pop();
            stack.push(current_node);

            for (const auto &neighbour_weak : current_node->get_neighbours()) {
                auto neighbour = neighbour_weak.lock();
                std::size_t order_n = neighbour->get_order();

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
        std::size_t order_n = node->get_order();

        while (!stack.empty()) {
            std::shared_ptr<Brandes::Node> &current_node = stack.top();
            std::size_t order_c = current_node->get_order();

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
    void calculate_weights(const std::size_t &threads_count, Graph &graph) {
        graph.clear_weights();
        Synchronization::Scheduler<BrandesScope, Graph>
                scheduler{ threads_count, std::reference_wrapper<Graph>(graph) };
        for (const auto &node : graph.get_nodes()) {
            scheduler.schedule(node.second);
        }
        scheduler.join();
    }
}
