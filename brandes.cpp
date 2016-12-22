#include "brandes.h"
#include <iostream>

Brandes::Brandes(const size_t &threads_count, const std::string &input_filename, const std::string &output_filename)
        : pool(threads_count) {
    input_file.open(input_filename);
    load_input();
    output_file.open(output_filename);
}

Brandes::~Brandes() {
    input_file.close();
    output_file.close();
}

void Brandes::calculate() {
    save_output();
}

void Brandes::load_input() {
    Node from, to;
    while (input_file >> from >> to) {
        auto from_iterator = graph.find(from);
        if (from_iterator == graph.end()) {
            from_iterator = graph.insert(std::make_pair(from, BrandesNode{ from })).first;
        }
        from_iterator->second.edges.push_back(to);
    }
}

void Brandes::save_output() {

}
