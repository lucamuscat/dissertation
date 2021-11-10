#ifndef LOCK_H
#define LOCK_H

#include <stdbool.h>
#include <stddef.h>

int create_lock(void** lock);
void free_lock(void*);

// Code design question: Should all the methods be grouped in a single header
// ex. wait_lock_atomic, wait_lock_stm, wait_lock_membar
// OR
// One single wait_lock method be used and is compiled differently
// depending on provided flags.

// Lock mutex if not already locked
// If already locked, wait
void wait_lock(void* lock);
void unlock(void* lock);

#endif