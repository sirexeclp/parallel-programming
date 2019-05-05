#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

long long *input;
long long *output;
int input_size;

void * sum(void* args)
{
    int index = (int)args;
    if((index* 2) +1 == input_size)
    {
        output[index] = input[index*2];
        //printf("no sum here\n");
        return;
    }
    output[index] = input[index*2]+input[index*2+1];
    //printf("%lld + %lld = %lld\n", input[index*2], input[index*2+1], output[index]);
}

int main(int argc, char* argv[])
{
    if(argc<4)
    {
        printf("usage: ./parsum <threads> <start> <end>");
        return EXIT_FAILURE;
    }
    
    int numThreads = atoi(argv[1]);
    long long start = atoll(argv[2]);
    long long end = atoll(argv[3]);
    //printf("Summing from %lld to %lld with %d threads.\n", start, end, numThreads);

    pthread_t *threads = malloc(sizeof(pthread_t) * numThreads);
    input_size = (end - start)+1;
    int output_size = input_size/2 + (input_size % 2 != 0);
    input = malloc(sizeof(long long) * input_size);
    output = malloc(sizeof(long long) * output_size);
    int sum_index = 0;
    int threads2Spawn = numThreads;
    for(int i = 0; i < input_size; i++)
    {
        input[i] = start+i;
    }
    while (input_size != 1)
    {
        //printf("input: %d\n", input_size);
        //printf("output: %d\n\n", output_size);
        sum_index = 0;
        while(sum_index< output_size){
            int actual_threads = 0;  
            for (int i=0; i < threads2Spawn && (sum_index < output_size); i++)
            {
                /* create a second thread which executes inc_x(&x) */
                if(pthread_create(&threads[i], NULL, sum, (void *) sum_index)) {
                    //printf(stderr, "Error creating thread\n");
                    return 1;
                }
                sum_index  ++;
                actual_threads ++;
            }
            for (int i=0; i < actual_threads; i++)
            {
                /* create a second thread which executes inc_x(&x) */
                pthread_join(threads[i],NULL);
            }
         }
        memcpy((void*)input, (void*)output, sizeof(long long) * output_size);
        input_size = output_size;
        output_size = input_size/2 + (input_size % 2 != 0);
    }
    printf("%lld", output[0]);
   
    free(input);
    free(output);
    free(threads);
 }