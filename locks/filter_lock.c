#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <omp.h>
#include "lock.h"
#include "globals.h"

#define P_LOCK ((filter_lock_t*)lock)

typedef struct filter_lock_t
{
    int* level;
    int* victim;
    size_t number_of_threads;
} filter_lock_t;

int create_lock(void** lock)
{
    const int num_of_threads = omp_get_num_threads();
    filter_lock_t** temp = (filter_lock_t**)lock;
    *temp = (filter_lock_t*)malloc(sizeof(filter_lock_t));
    if (temp != NULL)
    {
        (*temp)->level = (int*)malloc(sizeof(int) * num_of_threads);
        if ((*temp)->level == NULL)
            return errno;
        (*temp)->victim = (int*)malloc(sizeof(int) * num_of_threads);
        if ((*temp)->victim == NULL)
            return errno;
        for (int i = 0; i < num_of_threads; ++i)
        {
            (*temp)->level[i] = 0;
            (*temp)->victim[i] = 0;
        }
        return 0;
    }
    return errno;
}

void free_lock(void* lock)
{
    //free(P_LOCK->level);
    //free(P_LOCK->victim);
    //free(P_LOCK);
}

void wait_lock(void* lock)
{
    int num_of_threads = omp_get_num_threads();
    int thread_id = omp_get_thread_num();
    for (int i = 1; i < num_of_threads; ++i)
    {
        P_LOCK->level[thread_id] = i;
        P_LOCK->victim[i] = thread_id;
        // Spin lock
        bool stay = true;
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
    P_LOCK->level[omp_get_thread_num()] = 0;
}
