#ifndef BRANDES_H
#define BRANDES_H

#include <cstddef>
#include <string>
#include <fstream>

#include "thread_pool.h"

class Brandes {
private:
    std::ifstream input_file;
    std::ofstream output_file;
    ThreadPool pool;

public:
    Brandes(const size_t &threads_count, const std::string &input_filename, const std::string &output_filename);
    void calculate();
    ~Brandes();
};

#endif //BRANDES_H
