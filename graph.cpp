#include <algorithm>
#include "graph.h"

namespace Brandes {

    Node::Node(IdentifierType id) : id(id), order(0u), weight(0) {

    }

    const size_t Node::get_order() const noexcept {
        return order;
    }

    const std::vector<std::weak_ptr<Node>> &Node::get_neighbours() const
            noexcept {
        return neighbours;
    }

    void Node::increase_weight(WeightType weight) {
        std::lock_guard<std::mutex> lock(mutex);
        this->weight += weight;
    }

    std::shared_ptr<Node> Graph::create_node(IdentifierType id) {
        std::shared_ptr<Node> node = std::make_shared<Node>(id);
        nodes[id] = node;
        return node;
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

    void Graph::reorder() noexcept {
        size_t order = 0u;
        for (auto &node : nodes) {
            node.second->order = order++;
        }
    }

    void Graph::load(std::istream &istream) {
        IdentifierType in, out;
        while (istream >> in >> out) {
            std::shared_ptr<Brandes::Node> node = get_node(in);
            std::weak_ptr<Brandes::Node> neighbour = get_node(out);
            node->neighbours.push_back(neighbour);
        }
        reorder();
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
        for (auto &node : nodes) {
            node.second->weight = 0;
        }
    }

    const std::map<IdentifierType, std::shared_ptr<Node>> Graph::get_nodes()
            const noexcept {
        return nodes;
    }

    const size_t Graph::get_size() const noexcept {
        return nodes.size();
    }
}
