#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

bool isIntegerRank(int rank, int numIntegerRanks)
{
    return rank < numIntegerRanks;
}

int main(int argc, char ** argv) {
    int worldRank, worldSize;
    std::vector<int> inputNumbersInt;
    std::vector<double> inputNumbersDouble;
    int numElements = 0;

    double avgInt = 0;
    double avgDouble = 0;

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

    int group = isIntegerRank(worldRank, mpiInteger) ? 0 : 1;

    MPI_Comm split;
    MPI_Comm_split(MPI_COMM_WORLD, group, worldRank, &split);

    int splitRank, splitSize;
    MPI_Comm_rank(split, &splitRank);
    MPI_Comm_size(split, &splitSize);

    std::vector<int> displInt;
    std::vector<int> displDouble;

    if (splitRank == 0 && isIntegerRank(worldRank, mpiInteger))
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

    // Calculate Distribution and displacements
    std::vector<int> partSizes;
    if (splitRank == 0)
    {
        int count = numElements;
        int procs = splitSize;

        while (count > 0)
        {
            int partSize = int(std::ceil(count / std::max(1.0f, float(procs))));
            partSizes.push_back(partSize);
            procs -= 1;
            count -= partSize;
        }

        while (procs > 0)
        {
            partSizes.push_back(0);
            procs -= 1;
        }

        int sum = 0;

        if (isIntegerRank(worldRank, mpiInteger))
        {
            for (int i = 0; i < mpiInteger; i++) {
                displInt.push_back(sum);
                sum += partSizes[i];
            }
        }
        else
        {
            for (int i = 0; i < mpiDouble; i++) {
                displDouble.push_back(sum);
                sum += partSizes[i];
            }
        }
    }

    int size = 0;

    // Send the number of the elements which will be send next
    MPI_Scatter(static_cast<void*>(partSizes.data()),
                1,          // size is only one integer
                MPI_INT,
                &size,
                1,          // receive one integer
                MPI_INT,
                0,          // root
                split);

    // Distribute the numbers to the ranks
    if (isIntegerRank(worldRank, mpiInteger))
    {
        int *rec_buf = new int[size];

        MPI_Scatterv(   static_cast<void*>(inputNumbersInt.data()),
                        static_cast<int*>(partSizes.data()),
                        static_cast<int*>(displInt.data()),
                        MPI_INT,
                        rec_buf,
                        size,
                        MPI_INT,
                        0,
                        split);

        // Calculate local sum
        int localSum = 0;

        for (int i = 0; i < size; i++)
        {
            localSum += rec_buf[i];
        }

        // Calculate global sum and average
        int globalSum = 0;

        MPI_Reduce(&localSum, &globalSum, 1, MPI_INT, MPI_SUM, 0, split);

        if (splitRank == 0)
        {
            avgInt = globalSum / (double) numElements;;
        }
    }
    else
    {
        double *rec_buf = new double[size];

        MPI_Scatterv(   static_cast<void*>(inputNumbersDouble.data()),
                        static_cast<int*>(partSizes.data()),
                        static_cast<int*>(displDouble.data()),
                        MPI_DOUBLE,
                        rec_buf,
                        size,
                        MPI_DOUBLE,
                        0,
                        split);

        // Calculate local sum
        double localSum = 0;

        for (int i = 0; i < size; i++)
        {
            localSum += rec_buf[i];
        }

        // Calculate global sum and average
        double globalSum = 0;

        MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, split);

        if (splitRank == 0)
        {
            avgDouble = globalSum / (double) numElements;;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    double *allAvgDoubles = NULL;
    if (worldRank == 0)
    {
        allAvgDoubles = (double *)(malloc(sizeof(double) * worldSize));
    }

    // Misuse MPI_Gather to collect double average result
    MPI_Gather(&avgDouble, 1, MPI_DOUBLE, allAvgDoubles, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (worldRank == 0)
    {
        printf("%.6f\n", avgInt);
        printf("%.6f\n", allAvgDoubles[mpiInteger]);
    }

    MPI_Comm_free(&split);

    MPI_Finalize();
}
