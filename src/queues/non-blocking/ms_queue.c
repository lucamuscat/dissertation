#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <threads.h>
#include "../queue.h"
#include "../../test_utils.h"

// Implemented following https://github.com/cagao/LockFree/blob/master/MichaelScottLockFreeQueue.cpp

// The lock-free MS queue makes use of a counter to reduce the chances of the
// ABA problem occuring.
// The only time the counter won't protect against the ABA is when it overflows
// and goes back to the same count; whilst this may be a rare occurrence, there
// is still a possibility of this occurring.

struct node_t;
struct node_pointer_t;

typedef struct node_pointer_t
{
    struct node_t* ptr;
    uint64_t count;
} DWCAS_ALIGNED node_pointer_t;
// Align to 16 bytes to ensure that cmpxcgh16b is used.

typedef struct node_t
{
    void* value;
    struct node_pointer_t _Atomic next;
} node_t;

typedef struct queue_t
{
    // Needs to be atomic, these pointers will be accessed from multiple threads.
    DOUBLE_CACHE_ALIGNED node_pointer_t _Atomic head;
    DOUBLE_CACHE_ALIGNED node_pointer_t _Atomic tail;
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
    node_pointer_t null_node = { NULL, 0 };
    assert(atomic_is_lock_free(&sentinel->next));
    atomic_init(&sentinel->next, null_node);
}

// Beware that malloc might not be lock-free, making the algorithm
// also not lock-free.
bool create_queue(void** out_queue)
{
    queue_t** queue = (queue_t**)out_queue;
    *queue = (queue_t*)malloc(sizeof(queue_t));
    ASSERT_NOT_NULL(*queue);

    create_sentinel_node();
    // Make sure that double width compare and swap is available on the system
    // may return false in the case that the compiler does not emit the assembly
    // instruction directly.
    node_pointer_t sentinel_ptr = { sentinel, 0 };
    atomic_init(&(*queue)->head, sentinel_ptr);
    atomic_init(&(*queue)->tail, sentinel_ptr);
    return true;
}

void destroy_queue(void** out_queue)
{
    free(*out_queue);
    free(sentinel);
}

inline bool equals(node_pointer_t a, node_pointer_t b)
{
    return a.count == b.count && a.ptr == b.ptr;
}

bool enqueue(void* in_queue, void* in_item)
{
    queue_t* queue = (queue_t*)in_queue;
    node_t* node;
    create_node(&node);
    node->value = in_item;
    node_pointer_t null_node = { NULL, 0 };
    atomic_init(&node->next, null_node);

    node_pointer_t tail;
    
    while (true) // loop
    {
        // Check if the memory ordering can be weakened safely.
        tail = atomic_load(&queue->tail);
        if (equals(tail, atomic_load(&queue->tail)))
        {
            node_pointer_t next = atomic_load(&tail.ptr->next);
            if (next.ptr == NULL)
            {
                node_pointer_t new_ptr = { node, next.count + 1 };
                // TODO: Check if performance boost is gained when using
                // atomic_compare_exchange_weak inside of a while loop.
                if (atomic_compare_exchange_strong(&tail.ptr->next, &next, new_ptr))
                    break;
            }
            else
            {
                node_pointer_t new_ptr = { next.ptr, tail.count + 1 };
                atomic_compare_exchange_strong(&queue->tail, &next, new_ptr);
            }
        }
    }
    node_pointer_t new_ptr = { node, tail.count + 1 };
    atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
    return true;
}

bool dequeue(void* in_queue, void** out_item)
{
    queue_t* queue = (queue_t*)in_queue;
    node_pointer_t head;
    while (true)
    {
        head = atomic_load(&queue->head);
        node_pointer_t tail = atomic_load(&queue->tail);
        if (equals(head, atomic_load(&queue->head)))
        {
            node_pointer_t next = atomic_load(&head.ptr->next);
            if (head.ptr == tail.ptr)
            {
                if (next.ptr == NULL)
                    return false;
                node_pointer_t new_ptr = { next.ptr, tail.count + 1 };
                atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
            }
            else
            {
                *out_item = next.ptr->value;
                node_pointer_t new_ptr = { next.ptr, head.count + 1 };
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


