#include <stdatomic.h>
#include <errno.h>
#include <stdlib.h>
#include "lock.h"
#include "globals.h"

typedef struct ttas_lock_t
{
    atomic_bool busy;
} ttas_lock_t;

#define P_LOCK ((ttas_lock_t*)lock)


int create_lock(void** lock)
{
    *lock = malloc(sizeof(ttas_lock_t));
    if (*lock == NULL)
        return errno;
    atomic_init(&P_LOCK->busy, 0);
    return 0;
}

void free_lock(void* lock){/**/}

void wait_lock(void* lock)
{
    do
    {
        while (atomic_load_explicit(&(P_LOCK->busy), memory_order_acquire));
    } while (atomic_exchange_explicit(&(P_LOCK->busy), 1, memory_order_acquire));
}
void unlock(void* lock)
{
    atomic_flag_clear_explicit(&(P_LOCK->busy), memory_order_release);
}
