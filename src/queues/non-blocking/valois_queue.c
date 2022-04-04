#include <stdlib.h>
#include <stdatomic.h>
#include <threads.h>
#include "../queue.h"
#include "../../test_utils.h"

typedef struct node_t
{
    void* value;
    struct node_t* _Atomic next;
} node_t;

typedef struct valois_queue_t
{
    DOUBLE_CACHE_ALIGNED node_t* _Atomic head;
    DOUBLE_CACHE_ALIGNED node_t* _Atomic tail;
} valois_queue_t;

thread_local node_t* node_pool;
thread_local int node_count;
// Keep track of the sentinel node so that we may free it once the thread is
// destroyed.
thread_local node_t* sentinel;

void register_thread(size_t num_of_iterations)
{
    node_pool = (node_t*)calloc(num_of_iterations, sizeof(node_t));
    ASSERT_NOT_NULL(node_pool);
    node_count = -1;
}

void cleanup_thread()
{
    free(node_pool);
}

inline void create_node(node_t** out_node)
{
    *out_node = &node_pool[++node_count];
}

inline void create_sentinel_node()
{
    sentinel = (node_t*)calloc(1, sizeof(node_t));
    ASSERT_NOT_NULL(sentinel);
}

inline void destroy_sentinel_node()
{
    free(sentinel);
}

bool create_queue(void** out_queue)
{
    valois_queue_t** queue = (valois_queue_t**)out_queue;
    *queue = (valois_queue_t*)malloc(sizeof(valois_queue_t));
    ASSERT_NOT_NULL(*queue);

    create_sentinel_node();

    atomic_store(&(*queue)->head, sentinel);
    atomic_store(&(*queue)->tail, sentinel);
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
    valois_queue_t* queue = in_queue;
    node_t* q;
    create_node(&q);
    q->value = in_item;
    atomic_store(&q->next, NULL);

    node_t* p = atomic_load(&queue->tail);
    node_t* oldp = p;
    node_t* null_ptr = NULL;

    while (true)
    {
        /*
         * This is the doorway section of the function, where one successful
         * CAS causes every other enqueuing thread's CAS to fail.
        */
        if (atomic_compare_exchange_strong(&p->next, &null_ptr, q))
        {
            // The tail is only a hint to the location of the last item in the queue. 
            // This CAS returns true iff there are no contending enqueuers, or the enqueuing
            // thread is the first thread to pass the doorway.
            atomic_compare_exchange_strong(&queue->tail, &oldp, q);
            return true;
        }
        // Recall that in stdatomic.h (and in the intel64 instruction set),
        // when atomic_compare_exchange_strong returns false, the second parameter
        // is set to the value of the first. 

        // It is important that the function does not spuriously fail
        // (may occur when using atomic_compare_exchange_weak) as a spurious 
        // failure will lead to a null pointer dereference.
        p = null_ptr; // null_ptr = p->next
        null_ptr = NULL;
    }
}

bool dequeue(void* in_queue, void** out_item)
{
    valois_queue_t* queue = (valois_queue_t*)in_queue;
    node_t* old_node = atomic_load(&queue->head);

    while (true)
    {
        if (old_node->next == NULL)
            return false;
        if (atomic_compare_exchange_weak(&queue->head, &old_node, old_node->next))
        {
            // Recall that old_node = queue->head on CAS failure.
            // Spurious failures (ie. CAS returning false even when &queue->head == &old_node)
            // will not cause any serious errors.
            old_node = atomic_load(&old_node->next);
            *out_item = old_node->value;
            return true;
        }
        // Potentially employ backoff schemes        
    }
}

void destroy_queue(void** out_queue)
{
    free(*out_queue);
    destroy_sentinel_node();
}

char* get_queue_name()
{
    return "Valois MPMC Lock-Free Queue";
}