#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

#include "lock.h"

int create_lock(void** lock)
{
    omp_lock_t** temp = (omp_lock_t**)lock;
    *temp = (omp_lock_t*)malloc(sizeof(omp_lock_t));
    omp_init_lock(*lock);
    return 0;
}

void wait_lock(void* lock)
{
    omp_set_lock(lock);
}

void unlock(void* lock)
{
    omp_unset_lock(lock);
}