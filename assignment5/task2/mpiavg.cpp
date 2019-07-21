#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char ** argv) {
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

    std::vector<int> inputNumbersInt;
    std::vector<double> inputNumbersDouble;

    while (std::getline(file, line))
    {
        double number_double = atof(line.c_str());
        int number_int = atoi(line.c_str());

        inputNumbersInt.push_back(number_int);
        inputNumbersDouble.push_back(number_double);
    }

    // int world_rank, world_size, int_size, i;
    // int N = 1000;
    // // int avg, rand_nums,n , sub_avg, startval, endval, *sub_rand_nums, *sub_avgs, N=1000 ;

    // MPI_Init(&argc , &argv);
    // MPI_Comm_size(MPI_COMM_WORLD , &world_size);
    // MPI_Comm_rank(MPI_COMM_WORLD , &world_rank);

    // int startval = N * world_rank / world_size + 1;
    // int endval = N * (world_rank + 1) / world_size;

    // int n = N / world_size;

    // // Sum the numbers locally
    // int local_sum = 0;

    // for (i = startval; i <= endval; i++) {
    //     local_sum = local_sum + i;
    // }

    // // Print the random numbers on each process
    // printf("Local sum for process %d - %d, avg = %f\n", world_rank, local_sum, local_sum * 1. / n);

    // // // Reduce all of the local sums into the global sum
    // // int global_sum = 0;
    // // MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // // // Print the result
    // // if (world_rank == 0) {
    // //     printf("Total sum = %f, avg = %f\n", global_sum,
    // //     global_sum / N);
    // // }

    // MPI_Barrier(MPI_COMM_WORLD);

    // MPI_Finalize();
}
