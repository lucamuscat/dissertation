#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <threads.h>
#include "../queue.h"
#include "../../test_utils.h"
#include "../../tagged_ptr.h"


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
void create_node(node_t** out_node)
{
    *out_node = &node_pool[++node_count];
}

void create_sentinel_node()
{
    sentinel = (node_t*)calloc(1, sizeof(node_t));
    tagged_ptr_t null_node = pack_ptr_with_flag(NULL, 0, false);
    assert(atomic_is_lock_free(&sentinel->next));
    atomic_store(&sentinel->next, null_node);
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
    node_t* nd;
    create_node(&nd);
    nd->value = in_item;

    while (true)
    {
        tagged_ptr_t tail = atomic_load(&queue->tail);
        node_t* tail_ptr = extract_ptr(tail);
        if (tail == atomic_load(&queue->tail))
        {
            tagged_ptr_t next = atomic_load(&tail_ptr->next);
            node_t* next_ptr = extract_ptr(next);
            if (next_ptr == NULL)
            {
                uint16_t tail_tag = extract_flagged_tag(tail);
                tagged_ptr_t next_ptr = pack_ptr_with_flag(NULL, tail_tag + 2, false);
                atomic_store(&nd->next, next_ptr);

                tagged_ptr_t new_ptr = pack_ptr_with_flag(nd, tail_tag + 1, false);
                if (atomic_compare_exchange_strong(&tail_ptr->next, &next, new_ptr))
                {
                    atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
                    return true;
                }
                // next = tail.ptr->next is done implicitly by CAS on failure
                tagged_ptr_t next_tag = extract_flagged_tag(next);
                
                while ((next_tag == tail_tag + 1) && !extract_flag(next))
                {
                    // backoff scheme
                    atomic_store(&nd->next, next);
                    if (atomic_compare_exchange_strong(&tail_ptr->next, &next, new_ptr))
                        return true;
                    // next = tail.ptr->next is done implicitly by CAS on failure
                }
            }
            else
            {
                tagged_ptr_t next_next;
                while (true)
                {
                    next_next = atomic_load(&next_ptr->next);
                    // You can only break out of E20 if next.ptr->next.ptr == NULL
                    // or Q->tail != tail
                    if (extract_ptr(next_next) != NULL && atomic_load(&queue->tail) == tail)
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
        node_t* head_ptr = extract_ptr(head);
        tagged_ptr_t next = atomic_load(&head_ptr->next);
        
        if (head == atomic_load(&queue->head))
        {
            node_t* tail_ptr = extract_ptr(tail);
            if (head_ptr == tail_ptr)
            {
                if (extract_ptr(next) == NULL)
                    return false; // Queue is empty
                tagged_ptr_t next_next;
                node_t* next_ptr;
                while (true) // D09
                {
                    next_ptr = extract_ptr(next);
                    next_next = atomic_load(&next_ptr->next);
                    if (extract_ptr(next_next) != NULL && (atomic_load(&queue->tail) == tail))
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
                node_t* iter_ptr;
                size_t hops = 0;
                while ((extract_flag(next) && extract_ptr(iter) != extract_ptr(tail)) && (atomic_load(&queue->head) == head))
                {
                    iter = next;
                    iter_ptr = extract_ptr(iter);
                    next = atomic_load(&iter_ptr->next);
                    hops++;
                }
                if (atomic_load(&queue->head) != head)
                    continue;
                else if (extract_ptr(iter) == extract_ptr(tail))
                {
                    // free chain without memory reclamation
                    tagged_ptr_t new_ptr = pack_ptr_with_flag(extract_ptr(iter), extract_flagged_tag(head) + 1, false);
                    atomic_compare_exchange_strong(&queue->head, &head, new_ptr);
                    //free_chain(queue, &head, &iter);
                }
                else
                {
                    node_t* next_ptr = extract_ptr(next);
                    void* value = next_ptr->value;
                    tagged_ptr_t new_ptr = pack_ptr_with_flag(next_ptr, extract_flagged_tag(next) + 1, true);
                    node_t* iter_ptr = extract_ptr(iter);
                    if (atomic_compare_exchange_strong(&iter_ptr->next, &next, new_ptr))
                    {
                        if (hops >= MAX_HOPS)
                        {
                            //tagged_ptr_t new_ptr = { next.ptr, false, head.tag + 1 };
                            tagged_ptr_t new_ptr = pack_ptr_with_flag(next_ptr, extract_flagged_tag(head) + 1, false);
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
    return "Baskets queue";
}