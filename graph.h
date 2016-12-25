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
    using IdentifierType = unsigned int;
    using WeightType = double;

    class Node {
        friend class Graph;
    private:
        std::vector<std::weak_ptr<Node>> neighbours;
        IdentifierType id;
        size_t order;
        WeightType weight;
        std::mutex mutex;
    public:
        Node(IdentifierType id);
        const std::vector<std::weak_ptr<Node>> &get_neighbours() const noexcept;
        const size_t get_order() const noexcept;
        Node &operator+=(WeightType weight);
    };

    class Graph {
    private:
        std::map<IdentifierType, std::shared_ptr<Node>> nodes;
        std::shared_ptr<Node> create_node(IdentifierType id);
        std::shared_ptr<Node> get_node(IdentifierType id);
        void reorder() noexcept;

    public:
        void load(std::istream &istream);
        void save(std::ostream &ostream) const;
        void clear_weights() noexcept;
        const std::map<IdentifierType, std::shared_ptr<Node>> get_nodes() const
                noexcept;
        const size_t get_size() const noexcept;
    };
}


#endif //BRANDES_GRAPH_H
