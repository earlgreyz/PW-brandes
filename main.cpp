#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>

#include "graph.h"
#include "betweenness.h"

size_t threads_count;
std::string input_filename;
std::string output_filename;

void usage() {
    std::cerr << "Usage: " << std::endl;
}

template <typename T>
void parse_argument(const char *input, T &output, const std::string &error) {
    std::istringstream parser{ input };
    if (!(parser >> output)) {
        throw std::invalid_argument(error);
    }
}

void parse_args(const int &argc,  char * const argv[]) {
    if (argc < 4) {
        throw std::invalid_argument("Not enough arguments");
    } else if (argc > 4) {
        throw std::invalid_argument("Too many arguments");
    }

    parse_argument<size_t>(argv[1], threads_count, "Threads count must be an unsigned integer");
    parse_argument<std::string>(argv[2], input_filename,  "Invalid input filename");
    parse_argument<std::string>(argv[3], output_filename, "Invalid output filename");
}

int main(int argc, char *argv[]) {
    try {
        parse_args(argc, argv);
    } catch (std::invalid_argument e) {
        std::cerr << e.what() << std::endl;
        usage();
        return EXIT_FAILURE;
    }

    try {
        Brandes::Graph graph;
        std::ifstream input_file(input_filename);
        graph.load(input_file);
        Brandes::betweenness(threads_count, graph);
        graph.save(std::cout);
    } catch (...) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}