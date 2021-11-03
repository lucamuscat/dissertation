#ifndef FILTER_LOCK_H
#define FILTER_LOCK_H

#include <stddef.h>

typedef struct filter_lock
{
    size_t* level;
    size_t* victim;
    size_t number_of_threads;
} filter_lock;

int create_lock(size_t number_of_threads, filter_lock* lock);

// Lock mutex if not already locked
// If already locked, wait
void wait_lock(size_t thread_id, filter_lock* lock);
void unlock(size_t thread_id, filter_lock* lock);

#endif