#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

long long sum(long long a, long long b)
{

}

int main(int argc, char* argv[])
{
    if(argc<4)
    {
        printf("usage: ./parsum <threads> <start> <end>");
        return EXIT_FAILURE;
    }
    
    int numThreads = atoi(argv[1]);
    long long start = atoi(argv[2]);
    long long end = atoi(argv[3]);

    int threads = malloc(sizeof(int) * numThreads);
    int input_size = end - start;
    int output_size = input_size/2;
    long long input = malloc(size(long long ) * input_size);
    long long output = malloc(size(long long ) * output_size);

    int threads2Spawn = min(numThreads, output_size);
    while (output_size > 1)
    {
        for (int i; i < threads2Spawn; i++)
        {
            /* create a second thread which executes inc_x(&x) */
            if(pthread_create(&threads[i], NULL, inc_x, &x)) {

            fprintf(stderr, "Error creating thread\n");
            return 1;

            }
        }
        input_size = output_size;
        output_size = input_size/2;
        memcpy(input,output,size(long long) * output_size);
    }




    /* increment y to 100 in the first thread */
    while(++y < 100);

    printf("y increment finished\n");

    /* wait for the second thread to finish */
    if(pthread_join(inc_x_thread, NULL)) {

    fprintf(stderr, "Error joining thread\n");
    return 2;

    }

    if(output_size == 1)
    {
        printf("%lld",output[0]);
    }
    printf("%d %lld %lld", numThreads, start, end);
}