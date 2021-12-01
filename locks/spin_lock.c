#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

#include <stdatomic.h>
#include <errno.h>
#include <stdlib.h>
#include "lock.h"

#define P_LOCK ((spin_lock_t*)lock)

typedef struct spin_lock_t
{
    atomic_flag busy;
} spin_lock_t;

int create_lock(void** lock)
{
    *lock = malloc(sizeof(spin_lock_t));
    if (*lock == NULL)
        return errno;
    atomic_flag_clear(&P_LOCK->busy);
    return 0;
}
void free_lock(void* lock)
{
    //free(lock);
}

void wait_lock(void* lock)
{
    // If test & set returns 0 => the current thread locked the thread.
    while (atomic_flag_test_and_set(&(P_LOCK->busy)));
}

void unlock(void* lock)
{
    atomic_flag_clear(&(P_LOCK->busy));
}
#endif