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
    class BrandesScope : public Synchronization::Scope<std::size_t> {
    private:
        /**
         * Reference to the graph
         */
        Brandes::Graph &graph;
        /**
         * Stack of node orders
         */
        std::stack<std::size_t> stack;
        /**
         * Queue of node orders
         */
        std::queue<std::size_t> queue;

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
         * @param node starting node order
         */
        void init(Task node);
        /**
         * Applies calculated weights to the graph
         * @param node starting node order
         */
        void apply(Task node);
    public:
        /**
         * Constructs new BrandesScope
         * @param graph graph for which to calculate betweenness
         */
        BrandesScope(Brandes::Graph &graph);
        /**
         * Calculates betweenness for the graph starting with given node
         * @param node starting node order
         */
        void execute(Task node) override;
    };

    BrandesScope::BrandesScope(Brandes::Graph &graph) : graph(graph) {
        std::size_t graph_size = graph.get_size();
        predecessors = std::vector<std::vector<std::size_t>>(graph_size);
        shortest_paths = std::vector<std::size_t>(graph_size);
        distance = std::vector<long long>(graph_size);
        delta = std::vector<Brandes::WeightType >(graph_size);
    }

    void BrandesScope::init(Task node) {
        for (std::size_t i = 0u; i < graph.get_size(); i++) {
            predecessors[i].clear();
            shortest_paths[i] = 0;
            distance[i] = -1;
            delta[i] = 0;
        }

        shortest_paths[node] = 1;
        distance[node] = 0;
        queue.push(node);
    }

    void BrandesScope::execute(Task node) {
        init(node);

        while (!queue.empty()) {
            std::size_t current_node = queue.front();
            queue.pop();
            stack.push(current_node);

            for (const auto &neighbour : graph[current_node]->get_neighbours()) {
                  if (distance[neighbour] < 0) {
                    queue.push(neighbour);
                    distance[neighbour] = distance[current_node] + 1;
                }

                if (distance[neighbour] == distance[current_node] + 1) {
                    shortest_paths[neighbour] += shortest_paths[current_node];
                    predecessors[neighbour].push_back(current_node);
                }
            }
        }

        apply(node);
    }

    void BrandesScope::apply(Task node) {
        while (!stack.empty()) {
            std::size_t current_node = stack.top();
            stack.pop();

            for (const auto &predecessor : predecessors[current_node]) {
                Brandes::WeightType short_p = shortest_paths[predecessor];
                Brandes::WeightType short_c = shortest_paths[current_node];
                Brandes::WeightType delta_c = delta[current_node];
                delta[predecessor] += (short_p / short_c) * (1 + delta_c);
            }

            if (current_node != node) {
                graph[current_node]->increase_weight(delta[current_node]);
            }
        }
    }
}

namespace Brandes {
    void calculate_weights(const std::size_t &threads_count, Graph &graph) {
        graph.clear_weights();
        Synchronization::Scheduler<BrandesScope, Graph>
                scheduler{ threads_count, std::reference_wrapper<Graph>(graph) };
        for (std::size_t i = 0; i < graph.get_size(); i++) {
            scheduler.schedule(i);
        }
        scheduler.join();
    }
}
