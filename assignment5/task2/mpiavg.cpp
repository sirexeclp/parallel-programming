#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char ** argv) {
    std::vector<int> inputNumbersInt;
    std::vector<double> inputNumbersDouble;
    
    if (argc < 4)
    {
        std::cerr << "Wrong number of arguments!" << std::endl;
        exit(1);
    }

    std::string filename = argv[1];
    int mpi_integer = atoi(argv[2]);
    int mpi_double = atoi(argv[3]);

    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "Could not open the input file!" << std::endl;
        exit(1);
    }

    std::string line;
    while (std::getline(file, line))
    {
        double number_double = atof(line.c_str());
        int number_int = atoi(line.c_str());

        inputNumbersInt.push_back(number_int);
        inputNumbersDouble.push_back(number_double);
    }

    /**
     * Next steps:
     * 
     * Read input only, if you are master (rank 0)
     * Create 2 communicators (for int and double)
     * Broadcast/allocat numbers
     * Use MPI-Reduce to do Reduction (Average) : https://mpitutorial.com/tutorials/mpi-reduce-and-allreduce/
     * If master (rank 0): print result
    */
}
