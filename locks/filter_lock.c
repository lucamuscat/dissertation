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

bool stay_level_stm(size_t index, size_t thread_id, filter_lock_t* lock)
{
    __transaction_atomic{
        for (size_t k = 0; k < lock->number_of_threads; ++k)
        {
            // There exists k != me
            if (k == thread_id)
                continue;
            if (lock->level[k] >= index && lock->victim[index] == thread_id)
                return true;
        }
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

// TODO: Solve race conditions using the following concepts
// [] Atomics
// [] STM (Software-based Transactional Memory)
// [] Memory barriers
void wait_lock(size_t thread_id, filter_lock_t* lock)
{
    for (size_t i = 1; i < lock->number_of_threads; ++i)
    {
        // Can race conditions in this section be fixed through memory barriers?
        lock->level[thread_id] = i;
        lock->victim[i] = thread_id;
        // Spin lock
        while (stay_level(i, thread_id, lock));
    }
}

// wait_lock using software-based transactional memory
void wait_lock_stm(size_t thread_id, filter_lock_t* lock)
{
    for (size_t i = 1; i < lock->number_of_threads; ++i)
    {
        __transaction_atomic{
            lock->level[thread_id] = i;
            lock->victim[i] = thread_id;
        }
        while (stay_level_stm(i, thread_id, lock));
        // Spin lock
    }
}

void unlock_stm(size_t thread_id, filter_lock_t* lock)
{
    __transaction_atomic{
        lock->level[thread_id] = 0;
    }
}


void unlock(size_t thread_id, filter_lock_t* lock)
{
    lock->level[thread_id] = 0;
}

