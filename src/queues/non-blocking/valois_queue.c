#include <assert.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <threads.h>
#include <stdint.h>
#include "../queue.h"
#include "../../test_utils.h"
#include "../../assertion_utils.h"
#include "../queue_utils.h"
#include "auxiliary_valois_queue.h"

void create_sentinel_node()
{
    sentinel = (node_t*)calloc(1, sizeof(node_t));
    assert(atomic_is_lock_free(&sentinel->next));
    atomic_store(&sentinel->next, pack_ptr(NULL, 0));
}

bool create_queue(void** out_queue)
{
    queue_t** queue = (queue_t**)out_queue;
    *queue = (queue_t*)malloc(sizeof(queue_t));
    ASSERT_NOT_NULL(*queue);

    create_sentinel_node();
    
    tagged_ptr_t sentinel_ptr = pack_ptr(sentinel, 0);
    atomic_store(&(*queue)->head, sentinel_ptr);
    atomic_store(&(*queue)->tail, sentinel_ptr);
    return true;
}
/* Sources: 
 * 1: https://stackoverflow.com/a/49332468 - Suboptimal and blocking implementation
 * 2: https://github.com/supermartian/lockfree-queue/blob/2cdf763421a4cf20b656260b55bf8813664b3fde/pkt_queue.c
 *
 * Even though source two's implementation is not as clean as source one's
 * implementation, and does not correctly load atomics, the structure of the code
 * allows potential backoff protcols only on CAS failures
 */
bool enqueue(void* in_queue, void* in_item)
{
    queue_t* queue = in_queue;
    node_t* new_node = create_node_with_value(in_item);

    tagged_ptr_t p = atomic_load(&queue->tail);
    tagged_ptr_t oldp = p;

    internal_stats.counters.enqueue_count++;
    while (true)
    {
        // Since Valois' definition of Compare-and-Swap omits x86s side-effect
        // on a falsey comparison, null_ptr will be set to p->next in the Compare-
        // and-Swap in the until block. Defining null_ptr inside of the loop
        // ensures that null_ptr remains semantically immutable.
        tagged_ptr_t null_ptr = pack_ptr(NULL, 0);
        tagged_ptr_t next_ptr = atomic_load(&get_next_ptr(p));
        while (true)
        {
            if (equals(next_ptr, null_ptr))
                break;
            p = next_ptr;
            next_ptr = atomic_load(&get_next_ptr(p));
        };
        tagged_ptr_t new_node_tagged = pack_ptr(new_node, extract_tag(oldp) + 1);
        if (atomic_compare_exchange_strong(&get_next_ptr(p), &null_ptr, new_node_tagged))
        {
            atomic_compare_exchange_strong(&queue->tail, &oldp, new_node_tagged);
            return true;
        }
        internal_stats.counters.enqueue_retry_count++;
    }
}

bool dequeue(void* in_queue, void** out_item)
{
    queue_t* queue = (queue_t*)in_queue;
    tagged_ptr_t old_node = atomic_load(&queue->head);
    internal_stats.counters.dequeue_count++;
    while (true)
    {
        if (equals(get_next_ptr(old_node), pack_ptr(NULL, 0)))
        {
            internal_stats.counters.dequeue_empty_count++;
            return false;
        }
        if (atomic_compare_exchange_weak(&queue->head, &old_node, get_next_ptr(old_node)))
        {
            // Recall that old_node = queue->head on CAS failure.
            // Spurious failures (ie. CAS returning false even when &queue->head == &old_node)
            // will not cause any serious errors.
            old_node = atomic_load(&get_next_ptr(old_node));
            *out_item = ((node_t*)extract_ptr(old_node))->value;
            internal_stats.counters.dequeue_non_empty_count++;
            return true;
        }
        internal_stats.counters.dequeue_retry_count++;
        // Potentially employ backoff schemes
    }
}

char* get_queue_name()
{
    #ifdef DWCAS
    return "Valois Queue using DWCAS";
    #else
    return "Valois Queue using Tagged Pointers";
    #endif
}