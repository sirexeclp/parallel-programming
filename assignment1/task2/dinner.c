#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define UPPER_WAIT_TIME_EAT 5000
#define UPPER_WAIT_TIME_THINK 50000

void* dine(void* philoId);
void eat(int philoId);
void think();
void take(int i);
void putDown(int i);
void sleepRandomTimeWithUpperBound(float upperBound);

void printResultsToFile(char* filename, long* all_feedings);

int isTableReady = 0;
int numPhilosophers = 0;
pthread_t * philosophers;
pthread_mutex_t * forks;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Invalid arguments!\nUsage: <program-name> philosophers-count runtime-in-seconds\n");
        exit(1);
    }

    numPhilosophers = atoi(argv[1]);
    int runtime = atoi(argv[2]);

    forks = malloc(numPhilosophers * sizeof(pthread_mutex_t));
    for (int i = 0; i < numPhilosophers; i++)
    {
        pthread_mutex_init(&forks[i], NULL);
    }

    philosophers = malloc(numPhilosophers * sizeof(pthread_t));
    for (int i = 0; i < numPhilosophers; i++)
    {
        int rc = pthread_create(&(philosophers[i]), NULL, dine, (void *) i);
        if (rc != 0)
        {
            printf("[Error] when creating pthread! Return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    isTableReady = 1;
    sleep(runtime);
    isTableReady = 0;
    
    long * all_feedings = malloc(numPhilosophers * sizeof(long));

    for(int i = 0; i < numPhilosophers; i++)
    {
        void* feedings;
        pthread_join(philosophers[i], &feedings);
        all_feedings[i] = (long) feedings;
    }

    printResultsToFile("output.txt", all_feedings);
}

// Philosopher methods

void* dine(void* arg)
{
    int philoId = (int) arg;
    long feedings = 0;

    while(!isTableReady);

    do {
        eat(philoId);
        feedings++;
        think();
    } while(isTableReady);

    pthread_exit((void*) feedings);
}

void eat(int philoId)   
{
    int leftFork = philoId;
    int rightFork = philoId + 1;
    if (rightFork == numPhilosophers)
    {
        rightFork = 0;
    }

    if (philoId == 0)
    {
        take(leftFork);
        take(rightFork);
    }
    else
    {
        take(rightFork);
        take(leftFork);
    }

    sleepRandomTimeWithUpperBound(UPPER_WAIT_TIME_EAT);

    putDown(leftFork);
    putDown(rightFork);
}

void think()
{
    sleepRandomTimeWithUpperBound(UPPER_WAIT_TIME_THINK);
}

void take(int i)
{
    pthread_mutex_lock(&forks[i]);
}

void putDown(int i)
{
    pthread_mutex_unlock(&forks[i]);
}

// Helper

void sleepRandomTimeWithUpperBound(float upperBound)
{
    srand(time(NULL));
    float randTime = ((float) rand() / (float) (RAND_MAX)) * upperBound;
    for(int i=0; i< randTime; i++);
}

void printResultsToFile(char * filename, long * all_feedings)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    fprintf(f, "%ld", all_feedings[0]);
    for(int i = 1; i < numPhilosophers; i++)
    {
        fprintf(f, ";%ld", all_feedings[i]);
    }

    fclose(f);
}
