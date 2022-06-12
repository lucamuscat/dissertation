#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <threads.h>
#include "../queue.h"
#include "../../test_utils.h"
#include "../../tagged_ptr.h"

struct node_t;
struct pointer_t;

typedef struct node_t
{
    tagged_ptr_t _Atomic next;
    void* value;
} node_t;

typedef struct queue_t
{
    DOUBLE_CACHE_ALIGNED tagged_ptr_t _Atomic tail;
    DOUBLE_CACHE_ALIGNED tagged_ptr_t _Atomic head;
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
node_t* create_node()
{
    return &node_pool[++node_count];
}

void create_sentinel_node()
{
    sentinel = (node_t*)calloc(1, sizeof(node_t));
    assert(atomic_is_lock_free(&sentinel->next));
    atomic_store(&sentinel->next, pack_ptr_with_flag(NULL, 0, false));
}

bool create_queue(void** out_queue)
{
    queue_t** queue = (queue_t**)out_queue;
    *queue = (queue_t*)malloc(sizeof(queue_t));
    ASSERT_NOT_NULL(*queue);

    create_sentinel_node();

    tagged_ptr_t sentinel_ptr = pack_ptr_with_flag(sentinel, 0, false);
    atomic_store(&(*queue)->head, sentinel_ptr);
    atomic_store(&(*queue)->tail, sentinel_ptr);
    return true;
}

bool enqueue(void* in_queue, void* in_item)
{
    queue_t* queue = (queue_t*)in_queue;
    node_t* nd = create_node(); // E1
    nd->value = in_item; // E2
    while (true) // E3
    {
        tagged_ptr_t tail = atomic_load(&queue->tail); // E4
        tagged_ptr_t next = atomic_load(&get_next_ptr(tail)); // E5
        if (equals(tail, atomic_load(&queue->tail))) // E6
        {
            if (extract_ptr(next) == NULL) // E7
            {
                tagged_ptr_t next_ptr = pack_ptr_with_flag(NULL, extract_flagged_tag(tail) + 2, false);
                atomic_store(&nd->next, next_ptr); // E8

                tagged_ptr_t new_ptr = pack_ptr_with_flag(nd, extract_flagged_tag(tail) + 1, false); // E9
                if (atomic_compare_exchange_strong(&get_next_ptr(tail), &next, new_ptr)) // E9
                {
                    atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr); // E10
                    return true; // E11
                }
                // next = tail.ptr->next is done implicitly by CAS on failure; E12
                while((extract_flagged_tag(next) == extract_flagged_tag(tail) + 1) && !extract_flag(next)) // E13
                {
                    // backoff scheme E14
                    atomic_store(&nd->next, next); // E15
                    
                    if(atomic_compare_exchange_strong(&get_next_ptr(tail), &next, new_ptr))
                        return true;
                    // next = tail.ptr->next is done implicitly by CAS on failure
                }
            }
            else
            {
                tagged_ptr_t next_next;
                while (true)
                {
                    next_next = atomic_load(&get_next_ptr(next));
                    // You can only break out of E20 if next.ptr->next.ptr == NULL
                    // or Q->tail != tail
                    if(extract_ptr(next_next) != NULL && equals(atomic_load(&queue->tail), tail))
                    {
                        next = next_next;
                        continue;
                    }
                    break;
                }
                tagged_ptr_t new_tail = pack_ptr_with_flag(extract_ptr(next), extract_flagged_tag(tail) + 1, false);
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
        tagged_ptr_t head = atomic_load(&queue->head);
        tagged_ptr_t tail = atomic_load(&queue->tail);
        //tagged_ptr_t next = atomic_load(&head.ptr->next);
        tagged_ptr_t next = atomic_load(&get_next_ptr(head));

        if (equals(head, atomic_load(&queue->head)))
        {
            if (extract_ptr(head) == extract_ptr(tail))
            {
                if (extract_ptr(next) == NULL)
                    return false; // Queue is empty
                tagged_ptr_t next_next;
                while (true) // D09
                {
                    next_next = atomic_load(&get_next_ptr(next));
                    if (extract_ptr(next_next) != NULL && equals(atomic_load(&queue->tail), tail))
                    {
                        next = next_next; // D10
                        continue;
                    }
                    break;
                }
                tagged_ptr_t new_tail = pack_ptr_with_flag(extract_ptr(next), extract_flagged_tag(tail) + 1, false);
                atomic_compare_exchange_strong(&queue->tail, &tail, new_tail);
            }
            else
            {
                tagged_ptr_t iter = head;
                size_t hops = 0;
                while ((extract_flag(next) && extract_ptr(iter) != extract_ptr(tail)) && equals(atomic_load(&queue->head), head))
                {
                    iter = next;
                    next = atomic_load(&get_next_ptr(iter));
                    hops++;
                }
                if (!equals(atomic_load(&queue->head), head))
                    continue;
                else if (extract_ptr(iter) == extract_ptr(tail))
                {
                    // free chain without memory reclamation
                    tagged_ptr_t new_ptr = pack_ptr_with_flag(extract_ptr(iter), extract_flagged_tag(head) + 1, false);
                    atomic_compare_exchange_strong(&queue->head, &head, new_ptr);
                }
                else
                {
                    void* value = ((node_t*)extract_ptr(next))->value;
                    tagged_ptr_t new_ptr = pack_ptr_with_flag(extract_ptr(next), extract_flagged_tag(next) + 1, true);
                    if (atomic_compare_exchange_strong(&get_next_ptr(iter), &next, new_ptr))
                    {
                        if (hops >= MAX_HOPS)
                        {
                            tagged_ptr_t new_ptr = pack_ptr_with_flag(extract_ptr(next), extract_flagged_tag(head) + 1, false);
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
#ifdef DWCAS
    return "Baskets Queue using DWCAS";
#else
    return "Baskets Queue using tagged pointers";
#endif
}