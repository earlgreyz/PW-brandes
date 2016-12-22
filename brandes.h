#ifndef BRANDES_H
#define BRANDES_H

#include <cstddef>
#include <string>
#include <fstream>
#include <map>
#include <vector>

#include "thread_pool.h"

class Brandes {

    using Node = int;
    using Weight = double;

    struct BrandesNode {
        Node id;
        std::vector<Node> edges;
        Weight weight;

        BrandesNode(Node id)
                : id{id}, edges{}, weight{0} {}
    };

private:
    std::ifstream input_file;
    std::ofstream output_file;
    ThreadPool pool;
    std::map<Node, BrandesNode> graph;

    void load_input();
    void save_output();

public:
    Brandes(const size_t &threads_count, const std::string &input_filename, const std::string &output_filename);
    void calculate();
    ~Brandes();
};

#endif //BRANDES_H
