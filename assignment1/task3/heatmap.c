#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

typedef struct map{
    double *cells;
    int width;
    int height;
} map;

int update_cell(map map,int x,int y)
{
    double acc = 0;
    for(int i = MAX(x-1, 0); i <= MIN(x+1, map.width); i++)
        for(int j = MAX(y-1, 0); j <= MIN(y+1, map.height); j++)
            acc += map.cells[i+j*map.width];
    
    map.cells[x+y*map.width] = acc / 9.;
}

int main(int argc, char* argv[])
{
    if(argc<4)
    {
        printf("usage: ./parsum <threads> <start> <end>");
        return EXIT_FAILURE;
    }
    double c[] = {1,2,3
            ,1,0,1
            ,1,2,3};
    
    map map;
    map.cells = c;
    map.width=3;
    map.height =3;

    update_cell(map, 0,0);
    printf("%f",c[0]);

    return EXIT_SUCCESS;
 }