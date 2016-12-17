#include "brandes.h"

Brandes::Brandes(const size_t &threads_count, const std::string &input_filename, const std::string &output_filename)
        : threads_count(threads_count) {
    input_file.open(input_filename);
    output_file.open(output_filename);
}

Brandes::~Brandes() {
    input_file.close();
    output_file.close();
}
