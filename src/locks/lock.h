#ifndef LOCK_H
#define LOCK_H

#include <stdbool.h>

bool create_lock(void** lock);
void destroy_lock(void** lock);

// Code design question: Should all the methods be grouped in a single header
// ex. wait_lock_atomic, wait_lock_stm, wait_lock_membar
// OR
// One single wait_lock method be used and is compiled differently
// depending on provided flags.

// Lock mutex if not already locked
// If already locked, wait
void wait_lock(void* lock);
void unlock(void* lock);

/**
 * @brief Get the name of the lock currently being used. This will be used in each
 * tests' output.
 *
 * @return char* 
 */
char* get_lock_name();

#endif