#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <inttypes.h>

typedef unsigned __int128 uint128_t;
#define INT_TYPE  uint128_t
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef struct work_pack {
    long long start;
    long long end;
    int index;
} work_pack;

//global state
INT_TYPE *results;


/*print_u128_u function from https://stackoverflow.com/questions/11656241/how-to-print-uint128-t-number-using-gcc*/
/*      UINT64_MAX 18446744073709551615ULL */
#define P10_UINT64 10000000000000000000ULL   /* 19 zeroes */
#define E10_UINT64 19

#define STRINGIZER(x)   # x
#define TO_STRING(x)    STRINGIZER(x)

static int print_u128_u(uint128_t u128) {
    int rc;
    if (u128 > UINT64_MAX) {
        uint128_t leading = u128 / P10_UINT64;
        uint64_t trailing = u128 % P10_UINT64;
        rc = print_u128_u(leading);
        rc += printf("%." TO_STRING(E10_UINT64)
        PRIu64, trailing);
    } else {
        uint64_t u64 = u128;
        rc = printf("%"
        PRIu64, u64);
    }
    return rc;
}


void *sum(void *args) {
    work_pack *wp = (work_pack *) args;
    long long sum = 0;
    for (long long i = wp->start; i <= wp->end; i++)
        sum += i;
    results[wp->index] = sum;
}


int main(int argc, char *argv[]) {

    if (argc < 4) {
        printf("usage: ./parsum <threads> <start> <end>");
        return EXIT_FAILURE;
    }

    int numThreads = atoi(argv[1]);
    long long start = atoll(argv[2]);
    long long end = atoll(argv[3]);

    INT_TYPE total_sum = 0;
    pthread_t *threads = malloc(sizeof(pthread_t) * numThreads);
    results = malloc(sizeof(INT_TYPE) * numThreads);

    long long range = end - start;
    long long work_per_thread = range / numThreads;

    work_pack *wp = malloc(sizeof(work_pack) * numThreads);
    wp[0].start = start;
    wp[0].end = start + work_per_thread;
    wp[0].index = 0;

    if (pthread_create(&threads[0], NULL, sum, (void *) &wp[0])) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    for (int i = 1; i < numThreads; i++) {
        wp[i].start = wp[i - 1].end + 1;
        wp[i].end = MIN(wp[i].start + work_per_thread, end);
        wp[i].index = i;
        if (pthread_create(&threads[i], NULL, sum, (void *) &wp[i])) {
            printf("Error creating thread\n");
            return 1;
        }
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    for (int i = 0; i < numThreads; i++) {
        total_sum += results[i];
    }

    print_u128_u(total_sum);

    free(results);
    free(wp);
    free(threads);
}