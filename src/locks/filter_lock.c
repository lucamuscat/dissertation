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
    int num_of_threads;
} filter_lock_t;

bool create_lock(void** lock)
{
    const int num_of_threads = omp_get_max_threads();
    DEBUG_LOG_F("Created lock of size %d\n", num_of_threads);
    *lock = malloc(sizeof(filter_lock_t));
    if (*lock != NULL)
    {
        P_LOCK_2->num_of_threads = num_of_threads;
        //(*temp)->level = (int*)malloc(sizeof(int) * num_of_threads);
        P_LOCK_2->level = (int*)calloc(num_of_threads, sizeof(int));
        if (P_LOCK_2->level == NULL)
            return false;
        // (*temp)->victim = (int*)malloc(sizeof(int) * num_of_threads);
        P_LOCK_2->victim = (int*)calloc(num_of_threads, sizeof(int));
        if (P_LOCK_2->victim == NULL)
            return false;
        return true;
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
    for (int i = 1; i < P_LOCK->num_of_threads; ++i)
    {
        P_LOCK->level[thread_id] = i;
        P_LOCK->victim[i] = thread_id;
        // Spin lock
        bool stay = true;
        FULL_BARRIER;
        while (stay)
        {
            stay = false;
            for (int k = 0; k < P_LOCK->num_of_threads; ++k)
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

char* get_lock_name()
{
    return "Filter";
}
