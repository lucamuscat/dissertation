#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <threads.h>
#include "../queue.h"
#include "../../test_utils.h"

struct node_t;
struct pointer_t;

typedef struct pointer_t
{
    struct node_t* ptr;
    bool deleted;
    uint32_t tag;
} DWCAS_ALIGNED pointer_t;

typedef struct node_t
{
    pointer_t _Atomic next;
    void* value;
} node_t;

typedef struct queue_t
{
    DOUBLE_CACHE_ALIGNED pointer_t _Atomic tail;
    DOUBLE_CACHE_ALIGNED pointer_t _Atomic head;
} queue_t;

thread_local node_t* sentinel;
thread_local char pad1[PAD_TO_CACHELINE(node_t*)];
thread_local int64_t node_count;
thread_local char pad3[PAD_TO_CACHELINE(int64_t)];
thread_local node_t* node_pool;
thread_local char pad2[PAD_TO_CACHELINE(node_t*)];

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

void destroy_queue(void** out_queue)
{
    free(*out_queue);
    free(sentinel);
}

/**
 * @brief Create a node object, initialized with null values
 * @note Assumes that the method is being called inside of a thread that has
 * called the register_thread(1) method.
 * @param out_node
 */
void create_node(node_t** out_node)
{
    *out_node = &node_pool[++node_count];
}

void create_sentinel_node()
{
    sentinel = (node_t*)calloc(1, sizeof(node_t));
    pointer_t null_node = { NULL, false, 0 };
    assert(atomic_is_lock_free(&sentinel->next));
    atomic_store(&sentinel->next, null_node);
}

bool create_queue(void** out_queue)
{
    queue_t** queue = (queue_t**)out_queue;
    *queue = (queue_t*)malloc(sizeof(queue_t));
    ASSERT_NOT_NULL(*queue);

    create_sentinel_node();

    pointer_t sentinel_ptr = { sentinel, false, 0 };
    atomic_store(&(*queue)->head, sentinel_ptr);
    atomic_store(&(*queue)->tail, sentinel_ptr);
    return true;
}

bool equals(pointer_t a, pointer_t b)
{
    return a.tag == b.tag && a.ptr == b.ptr && a.deleted == b.deleted;
}

bool enqueue(void* in_queue, void* in_item)
{
    queue_t* queue = (queue_t*)in_queue;
    node_t* nd;
    create_node(&nd);
    nd->value = in_item;

    while (true)
    {
        pointer_t tail = atomic_load(&queue->tail);
        if (equals(tail, atomic_load(&queue->tail)))
        {
            pointer_t next = atomic_load(&tail.ptr->next);
            if (next.ptr == NULL)
            {
                pointer_t next_ptr = { NULL, false, tail.tag + 2 };
                atomic_store(&nd->next, next_ptr);

                pointer_t new_ptr = { nd, false, tail.tag + 1 };
                if (atomic_compare_exchange_strong(&tail.ptr->next, &next, new_ptr))
                {
                    atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
                    return true;
                }
                // next = tail.ptr->next is done implicitly by CAS on failure
                while ((next.tag == tail.tag + 1) && !next.deleted)
                {
                    // backoff scheme
                    atomic_store(&nd->next, next);
                    if (atomic_compare_exchange_strong(&tail.ptr->next, &next, new_ptr))
                        return true;
                    // next = tail.ptr->next is done implicitly by CAS on failure
                }
            }
            else
            {
                pointer_t next_next;
                while (true)
                {
                    next_next = atomic_load(&next.ptr->next);
                    // You can only break out of E20 if next.ptr->next.ptr == NULL
                    // or Q->tail != tail
                    if (next_next.ptr != NULL && equals(atomic_load(&queue->tail), tail))
                    {
                        next = next_next;
                        continue;
                    }
                    break;
                }
                pointer_t new_tail = { next.ptr, false, tail.tag + 1 };
                atomic_compare_exchange_strong(&queue->tail, &tail, new_tail);
            }
        }
    }
}

// A constant defined in the paper
#define MAX_HOPS 3

bool dequeue(void* in_queue, void** out_item)
{
    queue_t* queue = (queue_t*)in_queue;
    while (true)
    {
        pointer_t head = atomic_load(&queue->head);
        pointer_t tail = atomic_load(&queue->tail);
        pointer_t next = atomic_load(&head.ptr->next);

        if (equals(head, atomic_load(&queue->head)))
        {
            if (head.ptr == tail.ptr)
            {
                if (next.ptr == NULL)
                    return false; // Queue is empty
                pointer_t next_next;
                while (true) // D09
                {
                    next_next = atomic_load(&next.ptr->next);
                    if (next_next.ptr != NULL && equals(atomic_load(&queue->tail), tail))
                    {
                        next = next_next; // D10
                        continue;
                    }
                    break;
                }
                pointer_t new_tail = { next.ptr, false, tail.tag + 1 };
                atomic_compare_exchange_strong(&queue->tail, &tail, new_tail);
            }
            else
            {
                pointer_t iter = head;
                size_t hops = 0;
                while ((next.deleted && iter.ptr != tail.ptr) && equals(atomic_load(&queue->head), head))
                {
                    iter = next;
                    next = atomic_load(&iter.ptr->next);
                    hops++;
                }
                if (!equals(atomic_load(&queue->head), head))
                    continue;
                else if (iter.ptr == tail.ptr)
                {
                    // free chain without memory reclamation
                    pointer_t new_ptr = { iter.ptr, false, head.tag + 1 };
                    atomic_compare_exchange_strong(&queue->head, &head, new_ptr);
                    //free_chain(queue, &head, &iter);
                }
                else
                {
                    void* value = next.ptr->value;
                    pointer_t new_ptr = { next.ptr, true, next.tag + 1 };
                    if (atomic_compare_exchange_strong(&iter.ptr->next, &next, new_ptr))
                    {
                        if (hops >= MAX_HOPS)
                        {
                            pointer_t new_ptr = { next.ptr, false, head.tag + 1 };
                            atomic_compare_exchange_strong(&queue->head, &head, new_ptr);
                        }
                        *out_item = value;
                        return true;
                    }
                    // back off scheme
                }

            }
        }
    }
}


char* get_queue_name()
{
    return "Baskets Queue using DWCAS";
}