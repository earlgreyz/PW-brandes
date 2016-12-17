#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>

size_t threads_count;
std::string input_filename;
std::string output_filename;

void usage() {
    std::cerr << "Usage: " << std::endl;
}

void parse_args(const int &argc,  char * const argv[]) {
    if (argc < 4) {
        throw std::invalid_argument("Not enough arguments");
    } else if (argc > 4) {
        throw std::invalid_argument("Too many arguments");
    }

    std::istringstream parser{ argv[1] };
    if (!(parser >> threads_count) || threads_count > 100) {
        throw std::invalid_argument("Threads count must be an unsigned integer smaller than 100");
    }

    parser.str(argv[2]);
    parser.clear();
    if (!(parser >> input_filename)) {
        throw std::invalid_argument("Invalid input filename");
    }

    parser.str(argv[3]);
    parser.clear();
    if (!(parser >> output_filename)) {
        throw std::invalid_argument("Invalid output filename");
    }
}

int main(int argc, char *argv[]) {
    try {
        parse_args(argc, argv);
    } catch (std::invalid_argument e) {
        std::cerr << e.what() << std::endl;
        usage();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}