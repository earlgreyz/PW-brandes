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
            T id;
            std::vector<T> neighbours;
            W weight;

            Node(T id, std::vector<T> neighbours);
        };

        std::map<T, Node> nodes;
    public:
        void load(std::istream &istream);
        void save(std::ostream &ostream) const;
        void clear_weights();
        const std::vector<T>& get_neighbours(T id) const;
        W& operator[](T id);
    };
}


#endif //BRANDES_GRAPH_H
