#ifndef BRANDES_GRAPH_H
#define BRANDES_GRAPH_H


#include <cstddef>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <memory>

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
    public:
        Node(IdentifierType id);
        const std::vector<std::weak_ptr<Node>> &get_neighbours() const;
        const size_t get_order() const;
        Node &operator+=(WeightType weight);
    };

    class Graph {
    private:
        std::map<IdentifierType, std::shared_ptr<Node>> nodes;
        std::shared_ptr<Node> create_node(IdentifierType id);
        std::shared_ptr<Node> get_node(IdentifierType id);
        void reorder();

    public:
        void load(std::istream &istream);
        void save(std::ostream &ostream) const;
        void clear_weights();
        const std::map<IdentifierType, std::shared_ptr<Node>> get_nodes() const;
        const size_t get_size() const;
    };
}


#endif //BRANDES_GRAPH_H
