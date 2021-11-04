#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <errno.h>
#include "filter_lock.h"

bool stay_level(size_t index, size_t thread_id, filter_lock_t* lock)
{
    for (size_t k = 0; k < lock->number_of_threads; ++k)
    {
        // There exists k != me
        if (k == thread_id)
            continue;
        if (lock->level[k] >= index && lock->victim[index] == thread_id)
            return true;
    }
    return false;
}

int create_lock(size_t number_of_threads, filter_lock_t** lock)
{
    *lock = (filter_lock_t*)malloc(sizeof(filter_lock_t));
    if (lock != NULL)
    {
        (*lock)->level = (size_t*)malloc(sizeof(size_t)*number_of_threads);
        if ((*lock)->level == NULL)
            return errno;
        (*lock)->victim = (size_t*)malloc(sizeof(size_t) * number_of_threads);
        if ((*lock)->victim == NULL)
            return errno;
        for (size_t i = 0; i < number_of_threads; ++i)
        {
            (*lock)->level[i] = 0;
            (*lock)->victim[i] = 0;
        }
        return 0;
    }
    return errno;
}

void free_lock(filter_lock_t* lock)
{
    free(lock->level);
    free(lock->victim);
    free(lock);
}

void wait_lock(size_t thread_id, filter_lock_t* lock)
{
    for (size_t i = 1; i < lock->number_of_threads; ++i)
    {
        lock->level[thread_id] = i;
        lock->victim[i] = thread_id;
        // Spin lock
        while (stay_level(i, thread_id, lock));
    }
}

void unlock(size_t thread_id, filter_lock_t* lock)
{
    lock->level[thread_id] = 0;
}

