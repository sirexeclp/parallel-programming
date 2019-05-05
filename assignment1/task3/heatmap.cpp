#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <pthread.h>

struct Map{
    double *cells;
    int width;
    int height;
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
        start_round = start;
        end_round = end;
    }
};


int update_cell(Map map,int x,int y)
{
    double acc = 0;
    for(int i = std::max(x-1, 0); i <= std::min(x+1, map.width); i++)
        for(int j = std::max(y-1, 0); j <= std::min(y+1, map.height); j++)
            acc += map.cells[i+j*map.width];
    
    map.cells[x+y*map.width] = acc / 9.;
}

std::vector<Hotspot> read_input(std::string file_name)
{
    std::ifstream file(file_name);
    std::vector<Hotspot> hotspots;
    std::string line;
    //throw away the header
    std::getline(file, line);

    while(std::getline(file, line))
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
    if(argc<5)
    {
        printf("usage: ./heatmap width height rounds input-file [coordinates-file]");
        return EXIT_FAILURE;
    }
    Map map;
    map.width = atoi(argv[1]);
    map.height = atoi(argv[2]);
    int rounds = atoi(argv[3]);
    char * input_file = argv[4];

    auto result = read_input(input_file);
    for(auto item: result)
    {
        std::cout << "x "<< item.x << " y " << item.y  << " start "<< item.start_round <<" end "<< item.end_round << "\n";
    }

    // double c[] = {1,2,3
    //         ,1,0,1
    //         ,1,2,3};
    
    // map.cells = c;
    // map.width=3;
    // map.height =3;

    // update_cell(map, 0,0);
    // printf("%f",c[0]);

    return EXIT_SUCCESS;
 }