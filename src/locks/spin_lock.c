#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

#include <stdatomic.h>
#include <errno.h>
#include <stdlib.h>
#include <immintrin.h>
#include "lock.h"
#include "../alignment_utils.h"

#define P_LOCK ((spin_lock_t*)lock)

typedef struct spin_lock_t
{
    CACHE_ALIGNED atomic_flag busy;
} spin_lock_t;

bool create_lock(void** lock)
{
    spin_lock_t** temp = (spin_lock_t**)lock;
    *temp = malloc(sizeof(spin_lock_t));
    if (*lock == NULL)
        return false;
    atomic_flag_clear(&(*temp)->busy);
    return true;
}
void destroy_lock(void** lock)
{
    free(*lock);
}

void wait_lock(void* lock)
{
    // If test & set returns 0 => the current thread locked the thread.
    while (atomic_flag_test_and_set_explicit(&(P_LOCK->busy), memory_order_acquire))
    {
        _mm_pause();
    }
}

void unlock(void* lock)
{
    atomic_flag_clear_explicit(&(P_LOCK->busy), memory_order_release);
}

char* get_lock_name()
{
    return "TAS";
}

#endif