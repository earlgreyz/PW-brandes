#include "betweenness.h"

#include <stack>
#include <queue>
#include <iostream>
#include <condition_variable>
#include <thread>
#include <atomic>

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
        mutable std::mutex mutex;

        void init(const std::shared_ptr<Brandes::Node> &node);
        void apply(const std::shared_ptr<Brandes::Node> &node);
    public:
        BrandesWorker(const BrandesWorker&) = delete;
        BrandesWorker(BrandesWorker&&) = delete;
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

    void BrandesWorker::apply(const std::shared_ptr<Brandes::Node> &node) {
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
                current_node->increaseWeight(delta[order_c]);
            }
        }
    }

    class BrandesPool {
    private:
        std::size_t threads_waiting;
        Brandes::Graph graph;

        std::atomic_bool terminate;
        std::mutex mutex;
        std::condition_variable workers_condition;
        std::condition_variable wait_condition;

        std::vector<std::thread> workers;
        std::queue<std::shared_ptr<Brandes::Node>> queue;

        static void worker(BrandesPool* pool);
    public:
        BrandesPool(std::size_t threads_count, Brandes::Graph &graph);
        ~BrandesPool();
        void compute(const std::shared_ptr<Brandes::Node> &node);
        void wait();
    };

    BrandesPool::BrandesPool(std::size_t threads_count, Brandes::Graph &graph)
            : threads_waiting(0u), graph(graph), terminate(false) {
        workers.reserve(threads_count);
        for (std::size_t i = 0u; i < threads_count; i++) {
            workers.emplace_back(worker, this);
        }
    }

    void BrandesPool::worker(BrandesPool *pool) {
        BrandesWorker brandes_worker{ pool->graph };

        while (true) {
            std::unique_lock<std::mutex> lock{ pool->mutex };
            if (pool->queue.empty()) {
                pool->threads_waiting++;
                pool->wait_condition.notify_all();
                pool->workers_condition.wait(lock, [&]{
                   return pool->terminate || !pool->queue.empty();
                });
                pool->threads_waiting--;
            }

            if (pool->terminate) {
                break;
            }

            auto node = std::move(pool->queue.front());
            pool->queue.pop();
            lock.unlock();

            brandes_worker.compute(node);
        }
    }

    void BrandesPool::compute(const std::shared_ptr<Brandes::Node> &node) {
        std::lock_guard<std::mutex> lock{ mutex };
        queue.push(node);
        workers_condition.notify_one();
    }

    void BrandesPool::wait() {
        std::unique_lock<std::mutex> lock{ mutex };
        if (!queue.empty()) {
            wait_condition.wait(lock, [&]{
                return queue.empty();
            });
        }
    }

    BrandesPool::~BrandesPool() {
        terminate = true;
        workers_condition.notify_all();

        for (auto &t : workers) {
            if (t.joinable()) {
                t.join();
            }
        }
        wait_condition.notify_all();
    }
}

namespace Brandes {
    void betweenness(const size_t &threads_count, Graph &graph) {
        graph.clear_weights();
        BrandesPool brandes_pool{ threads_count, graph };
        for (const auto &node : graph.get_nodes()) {
            brandes_pool.compute(node.second);
        }
        brandes_pool.wait();
    }
}
