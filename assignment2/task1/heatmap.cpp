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

#define OUTPUT_NAME "output.txt"
#define CELL_T double

class Map {
    public:
        CELL_T *cells;
        int width;
        int height;

        Map(int width, int height)
        {
            this->height = height;
            this->width = width;
            this->cells = new CELL_T [height * width];

            for (int j = 0; j < height; j++)
            {
                for (int i = 0; i < width; i++)
                {
                    cells[j * width + i] = 0;
                }
            }
        }
        Map(const Map& old)
        {
            this->height = old.height;
            this->width  = old.width;
            this->cells = new CELL_T [height * width];
             for (int j = 0; j < height; j++)
            {
                for (int i = 0; i < width; i++)
                {
                    cells[j * width + i] = old.cells[j * width + i];
                }
            }
        }

        virtual ~Map()
        {
            delete cells;
        }

        void print()
        {
            std::ofstream output(OUTPUT_NAME);
            for (int j = 0; j < height; j++) {
                for (int i = 0; i < width; i++) {
                    auto value = cells[j * width + i];
                    if (value > .9)
                    {
                        output << "X";
                    }
                    else
                    {
                        value += 0.09;
                        value *= 10;
                        output << ((int) value) % 10;
                    }
                }
                output << std::endl;
            }
        }

        void print(std::string coordinateFile)
        {
            std::ifstream file(coordinateFile);
            std::ofstream output(OUTPUT_NAME);
            std::string line;

            // throw away the header
            std::getline(file, line);

            while(std::getline(file, line))
            {
                std::replace(line.begin(), line.end(), ',', ' ');
                int x, y;
                std::stringstream lineStream(line);
                lineStream >> x >> y;
                output << cells[x + y * width] << std::endl;
            }

        }
};

struct Hotspot
{
    int x;
    int y;
    int start_round;
    int end_round;

    Hotspot(int x, int y, int start, int end)
    {
        this->x = x;
        this->y = y;
        this->start_round = start;
        this->end_round = end;
    }
};

struct WorkPack
{
    Map * map_aggregated;
    Map * map_temp;
    int row;
};


void update_cell(Map * map_aggregated, Map * map_temp, int x, int y)
{
    auto offset = map_aggregated->width;
    double acc = 0;
    for (int i = std::max(x-1, 0); i < std::min(x+2, offset); i++)
    {
        for (int j = std::max(y-1, 0); j < std::min(y+2, map_aggregated->height); j++)
        {
            acc += (map_aggregated->cells)[j * offset + i];
        }
    }

    (map_temp->cells)[x + y * offset] = acc / 9.;
}

void update_row(Map * map_aggregated, Map * map_temp, int y)
{
    for (int x = 0; x < map_aggregated->width; x++)
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
        hotspots.push_back(Hotspot(x,y,start,end));
    }

    return hotspots;
}

int main(int argc, char* argv[])
{
    if (argc < 5)
    {
        printf("usage: ./heatmap width height rounds input-file [coordinates-file]");
        return EXIT_FAILURE;
    }

    int rows =  atoi(argv[2]);
    int columns = atoi(argv[1]);
    Map * map_aggregated = new Map(columns, rows);
    
    int rounds = atoi(argv[3]);
    char * input_file = argv[4];

    auto hotSpots = read_input(input_file);

    for (int round = 0; round < rounds; round++)
    {
        Map * map_temp = new Map(atoi(argv[1]), atoi(argv[2]));

        for (auto hotSpot: hotSpots)
        {
            if ((round >= hotSpot.start_round) && (round < hotSpot.end_round))
            {
                (map_aggregated->cells)[hotSpot.y * map_aggregated->width + hotSpot.x] = 1;
            }
        }
        #pragma parallel for
        for (int j = 0; j < rows; j++)
        {
            update_row(map_aggregated,map_temp,j);
        }

        delete map_aggregated;
        map_aggregated = map_temp;
    }

    for (auto hotSpot: hotSpots)
    {
        if ((rounds >= hotSpot.start_round) && (rounds < hotSpot.end_round))
        {
            (map_aggregated->cells)[hotSpot.y * map_aggregated->width + hotSpot.x] = 1;
        }
    }

    if (argc == 6)
    {
        auto coordinateFile = argv[5];
        map_aggregated->print(coordinateFile);
    }
    else
    {
        map_aggregated->print();
    }
    
    delete map_aggregated;

    return EXIT_SUCCESS;
 }