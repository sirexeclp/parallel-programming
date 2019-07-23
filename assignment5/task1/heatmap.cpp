#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <pthread.h>
#include <fstream>
#include <mpi.h>
#include <cmath>
#define OUTPUT_NAME "output.txt"
#define CELL_T double

struct Hotspot
{
    int x;
    int y;
    int start_round;
    int end_round;

    Hotspot() {}

    Hotspot(int x, int y, int start, int end)
    {
        this->x = x;
        this->y = y;
        this->start_round = start;
        this->end_round = end;
    }
};

class Map
{
public:
    CELL_T *cells;
    int width;
    int height;

    Map(int width, int height)
    {
        this->height = height;
        this->width = width;
        this->cells = new CELL_T[height * width];

        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {

                cell(i, j) = 0;
            }
        }
    }
    Map(const Map &old)
    {
        this->height = old.height;
        this->width = old.width;
        this->cells = new CELL_T[height * width];
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {

                cell(i, j) = old.cells[j + height * i];
            }
        }
    }

    virtual ~Map()
    {
        delete cells;
    }
    CELL_T &cell(int x, int y)
    {
        return cells[y + height * x];
    }

    // const CELL_T const operator()(int x, int y) {
    //      return cell(x,y);
    // }

    CELL_T *column(int x)
    {

        return &cells[x * height];
    }

    void print()
    {
        std::ofstream output(OUTPUT_NAME);
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                auto value = cell(i, j);
                if (value > .9)
                {
                    output << "X";
                }
                else
                {
                    value += 0.09;
                    value *= 10;
                    output << ((int)value) % 10;
                }
            }
            output << std::endl;
        }
    }

    void setHotspots(std::vector<Hotspot> hotSpots, int round)
    {
        for (auto hotSpot : hotSpots)
        {
            if ((round >= hotSpot.start_round) && (round < hotSpot.end_round))
            {
                if ((hotSpot.y < height) && (hotSpot.x < width))
                    cell(hotSpot.x, hotSpot.y) = 1;
            }
        }
    }

    void print(std::string coordinateFile)
    {
        std::ifstream file(coordinateFile);
        std::ofstream output(OUTPUT_NAME);
        std::string line;

        // throw away the header
        std::getline(file, line);

        while (std::getline(file, line))
        {
            std::replace(line.begin(), line.end(), ',', ' ');
            int x, y;
            std::stringstream lineStream(line);
            lineStream >> x >> y;
            output << cell(x, y) << std::endl;
        }
    }
};

struct WorkPack
{
    Map *map_aggregated;
    Map *map_temp;
    int row;
};

void update_cell(Map *map_aggregated, Map *map_temp, int x, int y)
{
    auto offset = map_aggregated->width;
    double acc = 0;
    for (int i = std::max(x - 1, 0); i < std::min(x + 2, offset); i++)
    {
        for (int j = std::max(y - 1, 0); j < std::min(y + 2, map_aggregated->height); j++)
        {
            acc += map_aggregated->cell(i, j);
        }
    }

    map_temp->cell(x, y) = acc / 9.;
}

// void update_row(Map *map_aggregated, Map *map_temp, int y)
// {
//     for (int x = 0; x < map_aggregated->width; x++)
//     {
//         update_cell(map_aggregated, map_temp, x, y);
//     }
// }

void update_column(Map *map_aggregated, Map *map_temp, int x)
{
    for (int y = 0; y < map_aggregated->height; y++)
    {
        update_cell(map_aggregated, map_temp, x, y);
    }
}

std::vector<Hotspot> read_input(std::string file_name)
{
    std::ifstream file(file_name);
    std::vector<Hotspot> hotspots;
    std::string line;

    // throw away the header
    std::getline(file, line);

    while (std::getline(file, line))
    {
        std::replace(line.begin(), line.end(), ',', ' ');
        int x, y, start, end;
        std::stringstream lineStream(line);
        lineStream >> x >> y >> start >> end;
        hotspots.push_back(Hotspot(x, y, start, end));
    }

    return hotspots;
}

int main(int argc, char *argv[])
{

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
    MPI_Request req;
    MPI_Status status;

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    if (argc < 5)
    {
        if (my_id == 0)
            std::cout << "usage: ./heatmap width height rounds input-file [coordinates-file]" << std::endl;

        MPI_Finalize();
        return EXIT_FAILURE;
    }

    std::vector<Hotspot> hotSpots;
    int width, height, rounds;
    int numHotspots;
    char *input_file;

    width = atoi(argv[1]);
    height = atoi(argv[2]);
    rounds = atoi(argv[3]);

    if (my_id == 0)
    {
        input_file = argv[4];
        //read hotspots
        hotSpots = read_input(input_file);
        numHotspots = hotSpots.size();
    }

    //broadcast size of hotSpots vector
    MPI_Bcast(&numHotspots, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //resize hotSpots vector
    if (my_id != 0)
        hotSpots.resize(numHotspots);

    //broadcast hotSpots vector
    MPI_Bcast(hotSpots.data(), (sizeof(Hotspot) / sizeof(int)) * hotSpots.size(), MPI_INT, 0, MPI_COMM_WORLD);

    Map *map_aggregated = new Map(width, height);
    Map *map_temp = new Map(width, height);

    //calculate threads/ranks to use
    int numThreads = std::min(width, world_size);

    //calculate ranks of neighbors
    int above_nbr, below_nbr;
    above_nbr = (my_id - 1) % numThreads;
    below_nbr = (my_id + 1) % numThreads;

    //devide by chunks of columns
    int workPerThread = width / numThreads;
    int startCol = my_id * workPerThread;
    int endCol = std::min((my_id + 1) * workPerThread, width) - 1;
    //last rank has do to a little bit more work
    if (my_id == (numThreads - 1))
    {
        endCol = width - 1;
    }

    int round = 0;
    //main loop
    for (round = 0; round < rounds; round++)
    {
        //set hotspots for this round
        map_aggregated->setHotspots(hotSpots, round);

        //calculate updates on chunk of columns
        for (int i = startCol; i <= endCol; i++)
        {
            update_column(map_aggregated, map_temp, i);
        }

        //first rank communicates only with ranks on the right side
        if (my_id == 0)
        {
            //recieve from right write to last col +1
            MPI_Recv(map_temp->column(endCol + 1),
                     height, MPI_DOUBLE, below_nbr, 2, MPI_COMM_WORLD, &status);

            //send last col to right
            MPI_Bsend(map_temp->column(endCol), height, MPI_DOUBLE, below_nbr, 1, MPI_COMM_WORLD);
        }
        //last rank communicates only with ranks on the left side
        else if (my_id == (numThreads - 1))
        {

            //send to left
            MPI_Bsend(map_temp->column(startCol), height, MPI_DOUBLE, above_nbr, 2, MPI_COMM_WORLD);

            //recieve from left
            MPI_Recv(map_temp->column(startCol - 1),
                     height, MPI_DOUBLE, above_nbr, 1, MPI_COMM_WORLD, &status);
        }
        else
        {

            //send to left
            MPI_Bsend(map_temp->column(startCol), height, MPI_DOUBLE, above_nbr, 2, MPI_COMM_WORLD);

            //recieve from right write to last col +1
            MPI_Recv(map_temp->column(endCol + 1),
                     height, MPI_DOUBLE, below_nbr, 2, MPI_COMM_WORLD, &status);

            //recieve from left
            MPI_Recv(map_temp->column(startCol - 1),
                     height, MPI_DOUBLE, above_nbr, 1, MPI_COMM_WORLD, &status);

            //send last row to right
            MPI_Bsend(map_temp->column(endCol), height, MPI_DOUBLE, below_nbr, 1, MPI_COMM_WORLD);
        }

        std::swap(map_aggregated, map_temp);
    }

    //could probably use gather here
    if (my_id == 0)
    {
        for (int i = 1; i < numThreads; i++)
        {
            startCol = i * workPerThread;
            endCol = std::min((i + 1) * workPerThread, width) - 1;
            if (i == (numThreads - 1))
            {
                endCol = width - 1;
            }

            MPI_Recv(map_aggregated->column(startCol),
                     height * (endCol - startCol + 1), MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
        }
        //set hotspots one more time
        map_aggregated->setHotspots(hotSpots, round);

        //write output to file
        if (argc == 6)
        {
            auto coordinateFile = argv[5];
            map_aggregated->print(coordinateFile);
        }
        else
        {
            map_aggregated->print();
        }
    }
    else
    {
        //send data to root
        MPI_Send(map_aggregated->column(startCol), height * (endCol - startCol + 1), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    delete map_aggregated;
    delete map_temp;

    MPI_Finalize();
    return EXIT_SUCCESS;
}