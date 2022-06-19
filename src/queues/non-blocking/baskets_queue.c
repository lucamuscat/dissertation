#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <threads.h>
#include "../queue.h"
#include "../../test_utils.h"
#include "../../assertion_utils.h"
#include "../../tagged_ptr.h"
#include "../queue_utils.h"
#include "auxiliary_baskets_queue.h"

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
        internal_stats.counters.enqueue_count++;
        if (equals(tail, atomic_load(&queue->tail))) // E6
        {
            internal_stats.counters.enqueue_consistent_tail_count++;
            if (extract_ptr(next) == NULL) // E7
            {
                internal_stats.counters.enqueue_next_ptr_not_null_count++;
                tagged_ptr_t next_ptr = pack_ptr_with_flag(NULL, extract_flagged_tag(tail) + 2, false);
                atomic_store(&nd->next, next_ptr); // E8

                tagged_ptr_t new_ptr = pack_ptr_with_flag(nd, extract_flagged_tag(tail) + 1, false); // E9
                if (atomic_compare_exchange_strong(&get_next_ptr(tail), &next, new_ptr)) // E9
                {
                    internal_stats.counters.enqueue_cas_tail_count++;
                    internal_stats.counters.enqueue_update_tail_count +=
                        atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr); // E10
                    return true; // E11
                }
                // next = tail.ptr->next is done implicitly by CAS on failure; E12
                while((extract_flagged_tag(next) == extract_flagged_tag(tail) + 1) && !extract_flag(next)) // E13
                {
                    // backoff scheme E14
                    atomic_store(&nd->next, next); // E15
                    
                    if(atomic_compare_exchange_strong(&get_next_ptr(tail), &next, new_ptr)) // E16
                        return true; // E17
                    internal_stats.counters.enqueue_build_basket_count++;
                    // next = tail.ptr->next is done implicitly by CAS on failure
                }
            }
            else
            {
                internal_stats.counters.enqueue_next_ptr_not_null_count++;
                tagged_ptr_t next_next;
                while (true) // E20
                {
                    next_next = atomic_load(&get_next_ptr(next)); // E20
                    // You can only break out of E20 if next.ptr->next.ptr == NULL
                    // or Q->tail != tail
                    if(extract_ptr(next_next) != NULL && equals(atomic_load(&queue->tail), tail)) // E20
                    {
                        next = next_next; // E21
                        continue;
                    }
                    break;
                }
                tagged_ptr_t new_tail = pack_ptr_with_flag(extract_ptr(next), extract_flagged_tag(tail) + 1, false);
                internal_stats.counters.enqueue_update_tail_count +=
                    atomic_compare_exchange_strong(&queue->tail, &tail, new_tail); // E22
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
        internal_stats.counters.dequeue_count++;
        tagged_ptr_t head = atomic_load(&queue->head);
        tagged_ptr_t tail = atomic_load(&queue->tail);
        //tagged_ptr_t next = atomic_load(&head.ptr->next);
        tagged_ptr_t next = atomic_load(&get_next_ptr(head));

        if (equals(head, atomic_load(&queue->head)))
        {
            internal_stats.counters.dequeue_consistent_head_count++;
            if (extract_ptr(head) == extract_ptr(tail))
            {
                internal_stats.counters.dequeue_single_item_count++;
                if (extract_ptr(next) == NULL)
                {
                    internal_stats.counters.dequeue_empty_count++;
                    return false; // Queue is empty
                }
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
                internal_stats.counters.dequeue_update_tail_count +=
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
                {
                    internal_stats.counters.dequeue_inconsistent_head_count++;
                    continue;
                }
                else if (extract_ptr(iter) == extract_ptr(tail))
                {
                    // free chain without memory reclamation
                    internal_stats.counters.dequeue_consistent_iter_tail_count++;
                    tagged_ptr_t new_ptr = pack_ptr_with_flag(extract_ptr(iter), extract_flagged_tag(head) + 1, false);
                    internal_stats.counters.dequeue_freechain_update_head_count +=
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
                            internal_stats.counters.dequeue_freechain_count++;
                            tagged_ptr_t new_ptr = pack_ptr_with_flag(extract_ptr(next), extract_flagged_tag(head) + 1, false);
                            internal_stats.counters.dequeue_freechain_update_head_count +=
                                atomic_compare_exchange_strong(&queue->head, &head, new_ptr);
                        }
                        *out_item = value;
                        return true;
                    }
                    internal_stats.counters.dequeue_cas_failed_count++;
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