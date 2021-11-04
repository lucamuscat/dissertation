#ifndef FILTER_LOCK_H
#define FILTER_LOCK_H

#include <stdbool.h>
#include <stddef.h>

typedef struct filter_lock_t
{
    size_t* level;
    size_t* victim;
    size_t number_of_threads;
} filter_lock_t;

int create_lock(size_t number_of_threads, filter_lock_t** lock);
void free_lock(filter_lock_t*);

// Code design question: Should all the methods be grouped in a single header
// ex. wait_lock_atomic, wait_lock_stm, wait_lock_membar
// OR
// One single wait_lock method be used and is compiled differently
// depending on provided flags.

// Lock mutex if not already locked
// If already locked, wait
void wait_lock(size_t thread_id, filter_lock_t* lock);
void wait_lock_stm(size_t thread_id, filter_lock_t* lock);
void unlock(size_t thread_id, filter_lock_t* lock);
void unlock_stm(size_t thread_id, filter_lock_t* lock);
bool stay_level(size_t index, size_t thread_id, filter_lock_t* lock);
bool stay_level_stm(size_t index, size_t thread_id, filter_lock_t* lock);

#endif