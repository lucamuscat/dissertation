#ifndef LOCK_H
#define LOCK_H

#include <stdbool.h>

bool create_lock(void** lock);
void destroy_lock(void** lock);

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