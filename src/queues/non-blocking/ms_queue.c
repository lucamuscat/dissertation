#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
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
    node_pointer_t _Atomic head;
    node_pointer_t _Atomic tail;
} queue_t;

/**
 * @brief Create a node object, initialized with null values
 * 
 * @param out_node 
 * @return true 
 * @return false 
 */
bool create_node(node_t** out_node)
{
    *out_node = (node_t*)calloc(1, sizeof(node_t));
    ASSERT_NOT_NULL(*out_node);
    return true;
}

// Beware that malloc might not be lock-free, making the algorithm
// also not lock-free.
bool create_queue(void** out_queue)
{
    queue_t** queue = (queue_t**)out_queue;
    *queue = (queue_t*)malloc(sizeof(queue_t));
    ASSERT_NOT_NULL(*queue);
    
    node_t* node;
    create_node(&node);
    assert(atomic_is_lock_free(&node->next));
    node_pointer_t next_node = { NULL, 0 };
    node_pointer_t sentinel = { node, 0 };
    atomic_init(&node->next, next_node);
    atomic_init(&(*queue)->head, sentinel);
    atomic_init(&(*queue)->tail, sentinel);
    return true;
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

    node_pointer_t tail;
    node_pointer_t next;
    
    while (true) // loop
    {
        // Check if the memory ordering can be weakened safely.
        tail = atomic_load(&queue->tail);
        next = atomic_load(&tail.ptr->next);
        if (equals(tail, atomic_load(&queue->tail)))
        {
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
        node_pointer_t next = head.ptr->next;
        if (equals(head, atomic_load(&queue->head)))
        {
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
                    break;
            }
        }
    }
    // Look into the free function actually used by the authors.
    // free(head.ptr);
    return true;
}

char* get_queue_name()
{
    return "MS Lock-Free Queue";
}


