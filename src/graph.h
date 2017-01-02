#ifndef BRANDES_GRAPH_H
#define BRANDES_GRAPH_H


#include <cstddef>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <memory>
#include <mutex>

namespace Brandes {
    /**
     * Type used as Node.id
     */
    using IdentifierType = unsigned int;
    /**
     * Type used as Node.weight
     */
    using WeightType = double;

    /**
     * @brief Represents single node in a graph.
     */
    class Node {
        friend class Graph;
    private:
        /**
         * vector of all node neighbours
         */
        std::vector<std::weak_ptr<Node>> neighbours;
        /**
         * unique id of the node
         */
        IdentifierType id;
        /**
         * unique order of the node
         */
        size_t order;
        /**
         * weight of the node
         */
        volatile WeightType weight;
        /**
         * mutex to guarantee thread safety of increase_weight
         */
        mutable std::mutex mutex;
    public:
        /**
         * Constructs a new node with given id.
         * @param id new node id.
         */
        Node(IdentifierType id);
        /**
         * Returns vector of all node neighbours.
         * @return vector of all node neighbours.
         */
        const std::vector<std::weak_ptr<Node>> &get_neighbours() const noexcept;
        /**
         * Returns node order.
         * @return node order.
         */
        const size_t get_order() const noexcept;
        /**
         * Increases weight of the node by given amount.
         * This function is thread safe.
         * @param weight amount to increase weight by
         */
        void increase_weight(WeightType weight);
    };

    /**
     * @brief Represents directed graph with weighted nodes.
     */
    class Graph {
    private:
        /**
         * map of nodes in a graph ordered by id
         */
        std::map<IdentifierType, std::shared_ptr<Node>> nodes;
        /**
         * Creates single Node.
         * Newly created node is added to the nodes map.
         * @param id id of the newly created node
         * @return pointer to the new node
         */
        std::shared_ptr<Node> create_node(IdentifierType id);
        /**
         * Gets node from nodes map.
         * If node with given id does not exist `create_node`
         * is called and the newly created node is returned.
         * @param id id of the node
         * @return pointer to the node with given id
         */
        std::shared_ptr<Node> get_node(IdentifierType id);
        /**
         * Reorders nodes.
         * Iterates through nodes map assigning each node with unique id
         * from `0` to `nodes.size() - 1`.
         */
        void reorder() noexcept;
    public:
        /**
         * Loads graph representation.
         * Input data in the stream should be of the given format
         * ```
         * <vertex_from> <vertex_to>
         * ```
         * meaning that directed edge from `<vertex_from>` to `<vertex_to>` exists.
         * The `<vertex_from>` and `<vertex_to>` must be of the IdentifierType type.
         * @param istream stream from which to load data
         */
        void load(std::istream &istream);
        /**
         * Saves graph weights.
         * Each line in output is of the given format
         * ```
         * <vertex_id> <vertex_weight>
         * ```
         * Vertices are printed by ascending order.
         * @param ostream stream to output data to
         */
        void save(std::ostream &ostream) const;
        /**
         * Resets weights for all nodes in a graph.
         */
        void clear_weights() noexcept;
        /**
         * Returns map of nodes in a graph.
         * @return map of nodes in a graph.
         */
        const std::map<IdentifierType, std::shared_ptr<Node>> get_nodes() const
                noexcept;
        /**
         * Returns number of nodes in the graph.
         * @return number of nodes in the graph.
         */
        const size_t get_size() const noexcept;
    };
}


#endif //BRANDES_GRAPH_H
