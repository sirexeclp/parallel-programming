#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

int main(int argc, char ** argv) {
    int worldRank, worldSize;
    std::vector<int> inputNumbersInt;
    std::vector<double> inputNumbersDouble;
    int numElements = 0;

    if (argc < 4)
    {
        std::cerr << "Wrong number of arguments!" << std::endl;
        exit(1);
    }

    std::string filename = argv[1];
    int mpiInteger = atoi(argv[2]);
    int mpiDouble = atoi(argv[3]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

    if (mpiInteger + mpiDouble != worldSize) {
        std::cerr << "Wrong number of mpi integer and double ranks!" << std::endl;
        exit(1);
    }

    int group = (worldRank < mpiInteger) ? 0 : 1;

    MPI_Comm split;
    MPI_Comm_split(MPI_COMM_WORLD, group, worldRank, &split);

    int splitRank, splitSize;
    MPI_Comm_rank(split, &splitRank);
    MPI_Comm_size(split, &splitSize);

    printf("WORLD RANK/SIZE: %d/%d \t SPLIT RANK/SIZE: %d/%d\n", worldRank, worldSize, splitRank, splitSize);

    std::vector<int> displs;

    if (worldRank == 0)
    {
        std::ifstream file(filename);
        if (file.fail()) {
            std::cerr << "Could not open the input file!" << std::endl;
            exit(1);
        }

        std::string line;
        while (std::getline(file, line))
        {
            int number = atoi(line.c_str());
            inputNumbersInt.push_back(number);
        }

        numElements = inputNumbersInt.size();
    }
    else if (splitRank == 0)
    {
        std::ifstream file(filename);
        if (file.fail()) {
            std::cerr << "Could not open the input file!" << std::endl;
            exit(1);
        }

        std::string line;
        while (std::getline(file, line))
        {
            double number = atof(line.c_str());
            inputNumbersDouble.push_back(number);
        }

        numElements = inputNumbersDouble.size();
    }

    std::vector<int> partSizes;
    if (splitRank == 0)
    {
        int count = numElements;
        int procs = splitSize;

        while(count > 0)
        {
            int partSize = int(std::ceil(count / std::max(1.0f, float(procs))));
            partSizes.push_back(partSize);
            procs -= 1;
            count -= partSize;
        }

        while(procs > 0)
        {
            partSizes.push_back(0);
            procs -= 1;
        }

        assert(partSizes.size() == mpiInteger);

        int sum = 0;
        for (int i = 0; i < mpiInteger; i++) {
            displs.push_back(sum);
            sum += partSizes[i];
        }
    }

    int size = 0;

    if (worldRank < mpiInteger)
    {
        MPI_Scatter(static_cast<void*>(partSizes.data()),
                    1,          // size is only one integer
                    MPI_INT,
                    &size,
                    1,          // receive one integer
                    MPI_INT,
                    0,          // root
                    split);
    }

    int *rec_buf = new int[size];

    if (worldRank < mpiInteger)
    {
        MPI_Scatterv(   static_cast<void*>(inputNumbersInt.data()),
                        static_cast<int*>(partSizes.data()),
                        static_cast<int*>(displs.data()),
                        MPI_INT,
                        rec_buf,
                        size,
                        MPI_INT,
                        0,
                        split);
    }

    // if (worldRank < mpiInteger)
    // {
    //     for (auto num : inputNumbersInt)
    //     {
    //         std::cout << "[Before " << worldRank << " | " << splitRank << "]" << num << std::endl;
    //     }
    //     MPI_Barrier(split);
    //     MPI_Bcast(static_cast<void*>(inputNumbersInt.data()), inputNumbersInt.size(), MPI_INT, 0, split);
    //     MPI_Barrier(split);
    //     for (auto num : inputNumbersInt)
    //     {
    //         std::cout << "[After " << worldRank << " | " << splitRank << "]" << num << std::endl;
    //     }
    // }
    // else
    // {
    //     // for (auto num : inputNumbersDouble)
    //     // {
    //     //     std::cout << "[Before " << worldRank << " | " << splitRank << "]" << num << std::endl;
    //     // }
    //     // MPI_Bcast(static_cast<void*>(inputNumbersDouble.data()), inputNumbersDouble.size(), MPI_DOUBLE, 0, split);
    //     // for (auto num : inputNumbersDouble)
    //     // {
    //     //     std::cout << "[After " << worldRank << " | " << splitRank << "]" << num << std::endl;
    //     // }
    // }


    /**
     * Next steps:
     * 
     * Broadcast/allocat numbers
     * Use MPI-Reduce to do Reduction (Average) : https://mpitutorial.com/tutorials/mpi-reduce-and-allreduce/
     * If master (rank 0): print result
    */

    MPI_Barrier(split);

    MPI_Comm_free(&split);

    MPI_Finalize();
}
