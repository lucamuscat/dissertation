#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <errno.h>
#include "filter_lock.h"
#include <stdbool.h>

int create_lock(size_t number_of_threads, filter_lock* lock)
{
    lock = (filter_lock*)malloc(sizeof(filter_lock));
    if (lock != NULL)
    {
        lock->level = (size_t*)calloc(number_of_threads, sizeof(size_t));
        if (lock->level == NULL)
            return errno;
        lock->victim = (size_t*)calloc(number_of_threads, sizeof(size_t));
        if (lock->victim == NULL)
            return errno;
        return 0
    }
    return errno;
}

void free_lock(filter_lock* lock)
{
    free(lock->level);
    free(lock->victim);
    free(lock);
}

void wait_lock(size_t thread_id, filter_lock* lock)
{
    for (int i = 1; i < lock->number_of_threads; ++i)
    {
        lock->level[thread_id] = i;
        lock->victim[i] = thread_id;
        // Spin lock
        while (stay_level(i, thread_id, lock));
    }
}

void unlock(size_t thread_id, filter_lock* lock)
{
    lock->level[thread_id] = 0;
}

inline bool stay_level(size_t index, size_t thread_id, const filter_lock* lock)
{
    for (int k = 0; k < lock->number_of_threads; ++k)
    {
        // There exists k != me
        if (k == thread_id)
            continue;
        if (lock->level[k] >= index && lock->victim[index] == thread_id)
            return true;
    }
    return false;
}