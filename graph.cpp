#include "graph.h"


namespace Brandes {
    Graph::Node::Node(Brandes::Graph::T identifier, std::vector<Brandes::Graph::T> edges)
            : identifier(identifier), edges(edges), weight(0) {}

    void Graph::load(std::istream &istream) {
        T in, out;
        while (istream >> in >> out) {
            try {
                nodes.at(in).edges.push_back(out);
            } catch (std::out_of_range e) {
                nodes.emplace(std::make_pair(in, Node{in, { out }}));
            }
        }
    }


    void Graph::save(std::ostream &ostream) const {
        for (auto node : nodes) {
            ostream << node.second.identifier << " " << node.second.weight << std::endl;
        }
    }
}
