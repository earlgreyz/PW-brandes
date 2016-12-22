#ifndef BRANDES_GRAPH_H
#define BRANDES_GRAPH_H


#include <vector>
#include <string>
#include <map>
#include <fstream>

namespace Brandes {
    class Graph {
    private:
        using T = unsigned int;
        using W = double;

        struct Node {
            Node(T identifier, std::vector<T> edges);

            T identifier;
            std::vector<T> edges;
            W weight;
        };

        std::map<T, Node> nodes;
    public:
        void load(std::istream &istream);
        void save(std::ostream &ostream) const;
    };
}


#endif //BRANDES_GRAPH_H
