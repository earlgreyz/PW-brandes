#include <algorithm>
#include "graph.h"

namespace Brandes {

    Node::Node(IdentifierType id, size_t order)
            : id(id), order(order), weight(0) {

    }

    const size_t Node::get_order() const noexcept {
        return order;
    }

    const std::vector<size_t> &Node::get_neighbours() const
            noexcept {
        return neighbours;
    }

    void Node::increase_weight(WeightType weight) {
        std::lock_guard<std::mutex> lock(mutex);
        this->weight += weight;
    }

    std::shared_ptr<Node> Graph::create_node(IdentifierType id) {
        std::shared_ptr<Node> node = std::make_shared<Node>(id, nodes.size());
        nodes[id] = node;
        ordered_nodes.push_back(node);
        return std::move(node);
    }

    std::shared_ptr<Node> Graph::get_node(IdentifierType id) {
        std::shared_ptr<Node> node = nullptr;
        try {
            node = nodes.at(id);
        } catch (std::out_of_range e) {
            node = create_node(id);
        }
        return node;
    }

    void Graph::load(std::istream &istream) {
        IdentifierType from, to;
        while (istream >> from >> to) {
            std::shared_ptr<Brandes::Node> node = get_node(from);
            std::shared_ptr<Brandes::Node> neighbour = get_node(to);
            node->neighbours.push_back(neighbour->get_order());
        }
    }

    void Graph::save(std::ostream &ostream) const {
        for (const auto &node : nodes) {
            if (node.second->neighbours.size() > 0) {
                ostream << node.second->id << " "
                        << node.second->weight << std::endl;
            }
        }
    }

    void Graph::clear_weights() noexcept {
        for (size_t i = 0; i < ordered_nodes.size(); i++) {
            ordered_nodes[i]->weight = 0;
        }
    }

    const size_t Graph::get_size() const noexcept {
        return ordered_nodes.size();
    }

    std::shared_ptr<Node> &Graph::operator[](size_t order) {
        return ordered_nodes[order];
    }
}
