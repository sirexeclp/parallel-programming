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

class Map {
    public:
        double *cells;
        int width;
        int height;

        Map(int width, int height)
        {
            this->height = height;
            this->width = width;
            this->cells = new double [height * width];

            for (int j = 0; j < height; j++)
                for (int i = 0; i < width; i++)
                {
                    cells[j * width + i] = 0;
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
                output << "\n";
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
                output << cells[x+y*width] << "\n";
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
    Map *map;
    int row;
};


void update_cell(Map &map, int x, int y)
{
    double acc = 0;
    for (int i = std::max(x-1, 0); i < std::min(x+2, map.width); i++)
    {
        for (int j = std::max(y-1, 0); j < std::min(y+2, map.height); j++)
        {
            acc += map.cells[j*map.width + i];
        }
    }

    map.cells[x + y * map.width] = acc / 9.;
}

void update_row(Map &map, int y)
{
    for (int x = 0; x < map.width; x++)
    {
        update_cell(map, x, y);
    }
}

void *run_thread(void * data)
{
    WorkPack *wp = (WorkPack*) data;
    update_row(*(wp->map), wp->row);
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

    Map map(atoi(argv[1]), atoi(argv[2]));
    int rounds = atoi(argv[3]);
    char * input_file = argv[4];

    auto hotSpots = read_input(input_file);

    int numThreads = map.height;
    pthread_t *threads = new pthread_t[numThreads];
    WorkPack *wp = new WorkPack[numThreads];

    for (auto hotSpot: hotSpots)
    {
        if (hotSpot.start_round == 0)
        {
            map.cells[hotSpot.y * map.width + hotSpot.x] = 1;
        }
    }

    for (int round = 0; round < rounds; round++)
    {
        for (int j = 0; j < map.height; j++)
        {
            wp[j].map = &map;
            wp[j].row = j;
            pthread_create(&threads[j], NULL, &run_thread, (void*) &wp[j]);
        }

        for (int j = 0; j< map.height; j++)
        {
            pthread_join(threads[j], NULL);
        }

        for (auto hotSpot: hotSpots)
        {
            if ((round >= hotSpot.start_round) && (round < hotSpot.end_round))
            {
                map.cells[hotSpot.y * map.width + hotSpot.x] = 1;
            }
        }
    }

    if (argc == 6)
    {
        auto coordinateFile = argv[5];
        map.print(coordinateFile);
    }
    else
    {
        map.print();
    }

    return EXIT_SUCCESS;
 }