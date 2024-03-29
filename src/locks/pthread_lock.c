#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "lock.h"

// TODO: Add PASS and ASSERT_NOT_NULL
bool create_lock(void** lock)
{
    *lock = malloc(sizeof(pthread_mutex_t));
    if (*lock == 0) return false;
    pthread_mutex_init(*lock, NULL);
    return true;
}

void wait_lock(void* lock)
{
    pthread_mutex_lock(lock);
}

void unlock(void* lock)
{
    pthread_mutex_unlock(lock);
}

void destroy_lock(void** lock)
{
    pthread_mutex_destroy((void*)lock);
}

char* get_lock_name()
{
    return "Pthread Lock";
}