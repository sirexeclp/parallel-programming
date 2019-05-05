#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define UPPER_WAIT_TIME_EAT 1.5
#define UPPER_WAIT_TIME_THINK 1.5

typedef struct
{
    pthread_t thread;
    long feedings;
} philosopher_t;

float randomWithUpperBound(int upperBound);
void* dine(void* philoId);
void eat(int philoId);
void think();

int isTableReady = 0;
philosopher_t * philosophers;
pthread_mutex_t * forks;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Invalid arguments!\nUsage: <program-name> philosophers-count runtime-in-seconds\n");
        exit(1);
    }

    int numPhilosophers = atoi(argv[1]);
    int runtime = atoi(argv[2]);

    forks = malloc(numPhilosophers * sizeof(pthread_mutex_t));
    philosophers = malloc(numPhilosophers * sizeof(philosopher_t));

    for (int i = 0; i < numPhilosophers; i++)
    {
        philosophers[i].feedings = 0;
        int rc = pthread_create(&(philosophers[i].thread), NULL, dine, (void *) i);
        if (rc != 0)
        {
            printf("[Error] when creating pthread! Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    isTableReady = 1;
    sleep(runtime);
    isTableReady = 0;
    
    for(int i = 0; i < numPhilosophers; i++)
    {
        pthread_cancel(philosophers[i].thread);
    }

    printf("%ld", philosophers[0].feedings);
    for(int i = 1; i < numPhilosophers; i++)
    {
        printf(";%ld", philosophers[i].feedings);
    }
    printf("\n");
}

// Philosopher methods

void *dine(void* arg)
{
    int philoId = (int) arg;

    while(!isTableReady);

    do {
        eat(philoId);
        think();
    } while(isTableReady);

    pthread_exit(NULL);
}

void eat(int philoId)   
{
    philosophers[philoId].feedings++;
    sleep(randomWithUpperBound(UPPER_WAIT_TIME_EAT));
}

void think()
{
    sleep(randomWithUpperBound(UPPER_WAIT_TIME_THINK));
}

// Helper

float randomWithUpperBound(int upperBound)
{
    srand(time(NULL));
    return ((float)rand()/(float)(RAND_MAX)) * upperBound;
}