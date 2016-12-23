#include "graph.h"


namespace Brandes {
    Graph::Node::Node(Brandes::Graph::T id, std::vector<Brandes::Graph::T> neighbours)
            : id(id), neighbours(neighbours), weight(0) {}

    void Graph::load(std::istream &istream) {
        T in, out;
        while (istream >> in >> out) {
            try {
                nodes.at(in).neighbours.push_back(out);
            } catch (std::out_of_range e) {
                nodes.emplace(std::make_pair(in, Node{in, { out }}));
            }
        }
    }

    void Graph::save(std::ostream &ostream) const {
        for (auto node : nodes) {
            ostream << node.second.id << " " << node.second.weight << std::endl;
        }
    }

    void Graph::clear_weights() {
        for (auto node : nodes) {
            node.second.weight = 0;
        }
    }

    const std::vector<Graph::T> &Graph::get_neighbours(Graph::T id) const {
        return nodes.at(id).neighbours;
    }

    Graph::W &Graph::operator[](Graph::T id) {
        return nodes.at(id).weight;
    }
}
