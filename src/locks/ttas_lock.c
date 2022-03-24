#include <stdatomic.h>
#include <errno.h>
#include <stdlib.h>
#include <immintrin.h>
#include "lock.h"
#include "globals.h"

typedef struct ttas_lock_t
{
    atomic_bool busy;
} ttas_lock_t;

#define P_LOCK ((ttas_lock_t*)lock)


bool create_lock(void** lock)
{
    ttas_lock_t** temp = (ttas_lock_t**)lock;
    *temp = (ttas_lock_t*)malloc(sizeof(ttas_lock_t));
    if (*temp == NULL)
        return false;
    atomic_init(&(*temp)->busy, 0);
    return true;
}

void destroy_lock(void** lock)
{
    free(*lock);
}

// Source: https://rigtorp.se/spinlock/
void wait_lock(void* lock)
{
    do
    {
        // Spin whilst busy == 1
        while (atomic_load_explicit(&P_LOCK->busy, memory_order_acquire))
            _mm_pause();
        // Set busy to 1, if busy is already 1, loop again.
        // The method loops again on the condition at least two threads
        // see the busy flag being set to zero at the same time.
    } while (atomic_exchange_explicit(&P_LOCK->busy, 1, memory_order_acquire));
}

void unlock(void* lock)
{
    ttas_lock_t* temp = (ttas_lock_t*)lock;
    atomic_store_explicit(&temp->busy, 0, memory_order_release);
}

char* get_lock_name()
{
    return "TTAS";
}