#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <omp.h>
#include "lock.h"
#include "globals.h"

#define P_LOCK ((filter_lock_t*)lock)
#define P_LOCK_2 ((filter_lock_t*)*lock)

typedef struct filter_lock_t
{
    int* level;
    int* victim;
} filter_lock_t;

int create_lock(void** lock)
{
    const int num_of_threads = omp_get_num_threads();
    *lock = malloc(sizeof(filter_lock_t));
    if (*lock != NULL)
    {
        //(*temp)->level = (int*)malloc(sizeof(int) * num_of_threads);
        P_LOCK_2->level = (int*)calloc(num_of_threads, sizeof(int));
        if (P_LOCK_2->level == NULL)
            return errno;
        // (*temp)->victim = (int*)malloc(sizeof(int) * num_of_threads);
        P_LOCK_2->victim = (int*)calloc(num_of_threads, sizeof(int));
        if (P_LOCK_2->victim == NULL)
            return errno;
        return 0;
    }
    return errno;
}

void free_lock(void* lock)
{
    free(P_LOCK->level);
    free(P_LOCK->victim);
    free(P_LOCK);
}

void wait_lock(void* lock)
{
    const int thread_id = omp_get_thread_num();
    const int num_of_threads = omp_get_num_threads();
    for (int i = 1; i < num_of_threads; ++i)
    {
        P_LOCK->level[thread_id] = i;
        P_LOCK->victim[i] = thread_id;
        // Spin lock
        bool stay = true;
        FULL_BARRIER;
        while (stay)
        {
            stay = false;
            for (int k = 0; k < num_of_threads; ++k)
            {
                if (k == thread_id)
                    continue;
                if (P_LOCK->level[k] >= i && P_LOCK->victim[i] == thread_id)
                {
                    stay = true;
                    break;
                }
            }
        }
    }
}

void unlock(void* lock)
{
    FULL_BARRIER;
    P_LOCK->level[omp_get_thread_num()] = 0;
}
