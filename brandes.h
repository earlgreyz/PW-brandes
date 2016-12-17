#ifndef BRANDES_H
#define BRANDES_H


#include <cstddef>
#include <string>
#include <fstream>

class Brandes {
private:
    const size_t threads_count;
    std::ifstream input_file;
    std::ofstream output_file;

public:
    Brandes(const size_t &threads_count, const std::string &input_filename, const std::string &output_filename);
    ~Brandes();
};

#endif //BRANDES_H
