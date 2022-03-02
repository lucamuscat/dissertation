#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

#include "lock.h"

bool create_lock(void** lock)
{
    *lock = malloc(sizeof(omp_lock_t));
    if (*lock == 0) return false;
    omp_init_lock(*lock);
    return true;
}

void wait_lock(void* lock)
{
    omp_set_lock(lock);
}

void unlock(void* lock)
{
    omp_unset_lock(lock);
}

void free_lock(void* lock)
{
    omp_destroy_lock(lock);
}