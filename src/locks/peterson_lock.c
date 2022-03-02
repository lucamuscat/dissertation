#include <omp.h>
#include <stdbool.h>
#include <stdlib.h>
#include "lock.h"
#include "globals.h"

typedef struct peterson_lock_t
{
    bool* flag;
    char victim;
} peterson_lock_t;

#define P_LOCK ((peterson_lock_t*)lock)

bool create_lock(void** lock)
{
    peterson_lock_t** temp = (peterson_lock_t**)lock;
    *temp = (peterson_lock_t*)malloc(sizeof(peterson_lock_t));
    (*temp)->flag = calloc(2, sizeof(bool));
    return true;
}

#define P_LOCK ((peterson_lock_t*)lock)

void wait_lock(void* lock)
{
    int i = omp_get_thread_num();
    int j = 1 - i;
    P_LOCK->flag[i] = true;
    P_LOCK->victim = i;
    // Full memory fence is needed as stores and loads occur before the fence
    FULL_BARRIER;
    while (P_LOCK->flag[j] && P_LOCK->victim == i) {}; // wait
}

void free_lock(void* lock){/**/}

void unlock(void* lock)
{
    // Full memory fence is needed so that the unlock code does not split
    // into the CS
    // It is possible for an unlock following a lock on multiple threads might
    // cause the unlocking code to occur after the lock 
    int me = omp_get_thread_num();
    P_LOCK->flag[me] = false;
    asm("mfence");
}