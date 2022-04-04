#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <threads.h>
#include "../queue.h"
#include "../../test_utils.h"
#include "../../tagged_ptr.h"

// Implemented following 

/**
 *  Sources [1, 2, 3] implement the Michael Scott Lock-Free queue in C++, whilst
 *  [4] implements it in C.
 *
 *  [1, 2, 4] all make use of double-width compare and swap operations, however, Chaoran
 *  moved away from using the double-width compare and swap operation, and adopted hazard pointers
 *  as a means of protecting against the ABA problem.
 *
 *  The Boost Lock-Free library implements the Michael-Scott queue as its MPMC
 *  lock-free queue [3]. The authors were able to implement the queue using single
 *  compare and swaps by placing the version-counter inside of the least significant
 *  16 bits of their pointers.
 *
 *  Aligning the head and tail pointers to double the size of the cache line led
 *  to a 3x speedup, presumably due to reduced false sharing. This technique is
 *  used inside of [2, 3, 4].
 *
 *  https://github.com/cagao/LockFree/blob/master/MichaelScottLockFreeQueue.cpp [1]
 *  https://github.com/pramalhe/ConcurrencyFreaks/blob/master/CPP/queues/MichaelScottQueue.hpp [2]
 *  https://www.boost.org/doc/libs/1_78_0/boost/lockfree/queue.hpp [3]
 *  https://github.com/chaoran/fast-wait-free-queue/commit/56a83e14c0b6ec1f65db70b6aa31474096255697 [4]
 */

// The lock-free MS queue makes use of a counter to reduce the chances of the
// ABA problem occuring.
// The only time the counter won't protect against the ABA is when it overflows
// and goes back to the same count; whilst this may be a rare occurrence, there
// is still a possibility of this occurring.

struct node_t;

typedef struct node_t
{
    tagged_ptr_t _Atomic next;
    void* value;
} node_t;

typedef struct queue_t
{
    // Needs to be atomic, these pointers will be accessed from multiple threads.
    DOUBLE_CACHE_ALIGNED tagged_ptr_t _Atomic head;
    DOUBLE_CACHE_ALIGNED tagged_ptr_t _Atomic tail;
} queue_t;

thread_local node_t* node_pool;
thread_local int node_count;
thread_local node_t* sentinel;

void register_thread(size_t num_of_iterations)
{
    node_pool = (node_t*)calloc(num_of_iterations, sizeof(node_t));
    node_count = -1;
    ASSERT_NOT_NULL(node_pool);
}

void cleanup_thread()
{
    free(node_pool);
}

/**
 * @brief Create a node object, initialized with null values
 * @note Assumes that the method is being called inside of a thread that has
 * called the register_thread(1) method.
 * @param out_node
 */
inline void create_node(node_t** out_node)
{
    *out_node = &node_pool[++node_count];
}

inline void create_sentinel_node()
{
    sentinel = (node_t*)calloc(1, sizeof(node_t));
    tagged_ptr_t null_node = pack_ptr(NULL, 0);
    assert(atomic_is_lock_free(&sentinel->next));
    atomic_store(&sentinel->next, null_node);
}

// Beware that malloc might not be lock-free, making the algorithm
// also not lock-free.
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

void destroy_queue(void** out_queue)
{
    free(*out_queue);
    free(sentinel);
}

bool enqueue(void* in_queue, void* in_item)
{
    queue_t* queue = (queue_t*)in_queue;
    node_t* node;
    create_node(&node);
    node->value = in_item;
    tagged_ptr_t null_node = pack_ptr(NULL, 0 );
    atomic_store(&node->next, null_node);

    tagged_ptr_t tail;
    
    while (true) // loop
    {
        // Check if the memory ordering can be weakened safely.
        tail = atomic_load(&queue->tail);
        if (tail == atomic_load(&queue->tail))
        {
            node_t* tail_ptr = extract_ptr(tail);
            tagged_ptr_t next = atomic_load(&tail_ptr->next);
            if (extract_ptr(next) == NULL)
            {
                tagged_ptr_t new_ptr = pack_ptr(node, extract_tag(next) + 1);
                // TODO: Check if performance boost is gained when using
                // atomic_compare_exchange_weak inside of a while loop.
                if (atomic_compare_exchange_strong(&tail_ptr->next, &next, new_ptr))
                {
                    // Can be optimized by directly changing new_ptr;
                    new_ptr = pack_ptr(node, extract_tag(tail) + 1);
                    atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
                    return true;
                }
            }
            else
            {
                tagged_ptr_t new_ptr = pack_ptr(extract_ptr(next), extract_tag(tail) + 1);
                atomic_compare_exchange_strong(&queue->tail, &next, new_ptr);
            }
        }
    }
}

bool dequeue(void* in_queue, void** out_item)
{
    queue_t* queue = (queue_t*)in_queue;
    tagged_ptr_t head;
    while (true)
    {
        head = atomic_load(&queue->head);
        tagged_ptr_t tail = atomic_load(&queue->tail);
        if (head == atomic_load(&queue->head))
        {
            node_t* head_ptr = extract_ptr(head);
            node_t* tail_ptr = extract_ptr(tail);

            tagged_ptr_t next = atomic_load(&head_ptr->next);
            node_t* next_ptr = extract_ptr(next);

            if (head_ptr == tail_ptr)
            {
                if (next_ptr == NULL)
                    return false;
                tagged_ptr_t new_ptr = pack_ptr(next_ptr, extract_tag(tail) + 1);
                atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
            }
            else
            {
                *out_item = next_ptr->value;
                tagged_ptr_t new_ptr = pack_ptr(next_ptr, extract_tag(head) + 1);
                if (atomic_compare_exchange_strong(&queue->head, &head, new_ptr))
                    return true;
            }
        }
    }
}

char* get_queue_name()
{
    return "MS Lock-Free Queue";
}


