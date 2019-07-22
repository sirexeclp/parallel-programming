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

    Hotspot()
    {

    }

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
    
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_id;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

    std::vector<Hotspot> hotSpots;
    int width, height, rounds;
    int numHotspots;
    char * input_file;
    if(my_id == 0)
    {
        if (argc < 5)
        {
            printf("usage: ./heatmap width height rounds input-file [coordinates-file]");
            return EXIT_FAILURE;
        }
        width =   atoi(argv[1]);
        height =  atoi(argv[2]);
        rounds = atoi(argv[3]);
        input_file = argv[4];        
        //read hotspots
        hotSpots = read_input(input_file);
        numHotspots = hotSpots.size();
    }

    MPI_Bcast(&width,1,MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height,1,MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rounds,1,MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numHotspots,1,MPI_INT, 0, MPI_COMM_WORLD);

    if(my_id != 0)
        hotSpots = std::vector<Hotspot>(numHotspots);

    MPI_Bcast(&hotSpots[0],sizeof(Hotspot)* numHotspots,MPI_INT, 0, MPI_COMM_WORLD);


    Map * map_aggregated = new Map(width, height);


    for (auto hotSpot: hotSpots)
    {
        std::cout << my_id << " x:" <<  hotSpot.x<< " y:" << hotSpot.y <<"\n";  
    }




    // MPI_Request req;
    // MPI_Status  status;

    // int numThreads = std::min(map_aggregated->height,world_size);
    
    // int above_nbr, below_nbr;
    // above_nbr = (my_id -1)%numThreads;
    // below_nbr = (my_id +1)%numThreads;
    
  
    // int workPerThread = std::ceil(map_aggregated->height / (float)numThreads);
    // int startRow = my_id * workPerThread;
    // int endRow = std::min((my_id +1)* workPerThread, map_aggregated->height);
    
    // for (int round = 0; round < 1; round++)
    // {
    //     Map * map_temp = new Map(width, height);
    //     for (auto hotSpot: hotSpots)
    //     {
    //         if ((round >= hotSpot.start_round) && (round < hotSpot.end_round))
    //         {
    //             (map_aggregated->cells)[hotSpot.y * map_aggregated->width + hotSpot.x] = 1;
    //         }
    //     }
    //     int i;
    //     for(i = startRow; i < endRow; i++ )
    //     {
    //         update_row(map_aggregated, map_temp, i);
    //     }
       
    //      std::cout << round << "/" << rounds  << "id" <<my_id << "\n";
    //     if (my_id==0)
    //     {
    //         //recieve from below write to last row +1 
    //         MPI_Recv(&(map_temp->cells[map_temp->width*(endRow+1) ]),
    //             map_temp->width, MPI_DOUBLE,below_nbr,2,MPI_COMM_WORLD ,&status);
            
    //         //send last row to below
    //         MPI_Send(&(map_temp->cells[map_temp->width*endRow])
    //         , map_temp->width, MPI_DOUBLE, below_nbr, 1, MPI_COMM_WORLD);
        
    //     }
    //     else if (my_id == (numThreads-1))
    //     {
            
    //           //send to above
    //         MPI_Send(&(map_temp->cells[map_temp->width*(startRow)])
    //         , map_temp->width, MPI_DOUBLE, above_nbr, 2, MPI_COMM_WORLD);
            
            
    //         //recieve from above
    //         MPI_Recv(&(map_temp->cells[map_temp->width * (startRow -1) ]),
    //             map_temp->width, MPI_DOUBLE,above_nbr,1,MPI_COMM_WORLD ,&status);

            
    //     }
    //     else
    //     {

    //         //send to above
    //         MPI_Send(&(map_temp->cells[map_temp->width*(startRow)])
    //         , map_temp->width, MPI_DOUBLE, above_nbr, 2, MPI_COMM_WORLD);
            
    //         //recieve from below write to last row +1 
    //         MPI_Recv(&(map_temp->cells[map_temp->width*(endRow+1) ]),
    //             map_temp->width, MPI_DOUBLE,below_nbr,2,MPI_COMM_WORLD ,&status);

    //         //recieve from above
    //         MPI_Recv(&(map_temp->cells[map_temp->width * (startRow -1) ]),
    //             map_temp->width, MPI_DOUBLE,above_nbr,1,MPI_COMM_WORLD ,&status);
            
    //         //send last row to below
    //         MPI_Send(&(map_temp->cells[map_temp->width*endRow])
    //         , map_temp->width, MPI_DOUBLE, below_nbr, 1, MPI_COMM_WORLD);

            

    //     }
        
    //     // std::cout << round << "/" << rounds<< "\n";


    //     //delete map_aggregated;
    //     map_aggregated = map_temp;
    //     // std::swap(map_aggregated,map_temp );    
    // }
    // std::cout << "done";

    // for (auto hotSpot: hotSpots)
    // {
    //     if ((rounds >= hotSpot.start_round) && (rounds < hotSpot.end_round))
    //     {
    //         (map_aggregated->cells)[hotSpot.y * map_aggregated->width + hotSpot.x] = 1;
    //     }
    // }
    // if(my_id!=0)
    // {
    //     MPI_Send(&map_aggregated->cells[map_aggregated->width*my_id ]
    //         , map_aggregated->width, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    // }
    // else{
    //      MPI_Recv(&map_aggregated->cells[map_aggregated->width*my_id ],
    //          map_aggregated->width, MPI_DOUBLE,0,0,MPI_COMM_WORLD ,&status);

    // }
  

    // if (argc == 6)
    // {
    //     auto coordinateFile = argv[5];
    //     map_aggregated->print(coordinateFile);
    // }
    // else
    // {
    //     map_aggregated->print();
    // }
    
    // delete map_aggregated;
    // //delete map_temp;
    
    MPI_Finalize();
    return EXIT_SUCCESS;
 }