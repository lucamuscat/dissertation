#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include "../queue.h"
#include "../../test_utils.h"
#include "../../assertion_utils.h"
#include "../../tagged_ptr.h"
#include "../queue_utils.h"
#include "./auxiliary_ms_queue.h"

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

char* get_queue_name()
{
#ifdef DWCAS
    return "MS Queue using DWCAS";
#else
    return "MS Queue using Tagged Pointers";
#endif
}

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
    // Make sure that double width compare and swap is available on the system
    // may return false in the case that the compiler does not emit the assembly
    // instruction directly.
    tagged_ptr_t sentinel_ptr = pack_ptr(sentinel, 0);
    atomic_store(&(*queue)->head, sentinel_ptr);
    atomic_store(&(*queue)->tail, sentinel_ptr);
    return true;
}

bool enqueue(void* in_queue, void* in_item)
{
    queue_t* queue = (queue_t*)in_queue;
    node_t* node = create_node_with_value(in_item); // E1 - E3
    tagged_ptr_t tail, next;
    internal_stats.counters.enqueue_count++;
    while (true) // loop E4
    {
        tail = atomic_load(&queue->tail); // E5
        next = atomic_load(&get_next_ptr(tail)); // E6
        if (equals(tail, atomic_load(&queue->tail))) // E7: tail == Q->Tail
        {
            internal_stats.counters.enqueue_consistent_tail_count++;
            if (extract_ptr(next) == NULL) // E8: next.ptr == NULL
            {
                internal_stats.counters.enqueue_next_ptr_null_count++;
                tagged_ptr_t new_ptr = pack_ptr(node, extract_tag(next) + 1); // E9
                if (atomic_compare_exchange_strong(&get_next_ptr(tail), &next, new_ptr)) // E9
                {
                    // Replace E10 with E17 & return true exit function
                    internal_stats.counters.enqueue_cas_tail_count++;
                    tagged_ptr_t new_ptr = pack_ptr(node, extract_tag(tail) + 1);
                    atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
                    return true; // E10: Emulates break behaviour
                }
            }
            else
            {
                internal_stats.counters.enqueue_next_ptr_not_null_count++;
                tagged_ptr_t new_ptr = pack_ptr(extract_ptr(next), extract_tag(tail) + 1); // E13
                atomic_compare_exchange_strong(&queue->tail, &next, new_ptr); // E13
            }
        }
    }
}

bool dequeue(void* in_queue, void** out_item)
{
    queue_t* queue = (queue_t*)in_queue;
    tagged_ptr_t head, tail, next;
    while (true) // D1
    {
        internal_stats.counters.dequeue_count++;
        head = atomic_load(&queue->head); // D2
        tail = atomic_load(&queue->tail); // D3
        next = atomic_load(&get_next_ptr(head)); // D4
        if (equals(head, atomic_load(&queue->head))) // D5
        {
            internal_stats.counters.dequeue_consistent_head_count++;
            if (extract_ptr(head) == extract_ptr(tail)) // D6
            {
                if (extract_ptr(next) == NULL) // D7
                {
                    internal_stats.counters.dequeue_empty_queue_count++;
                    return false; // D8
                }
                tagged_ptr_t new_ptr = pack_ptr(extract_ptr(next), extract_tag(tail) + 1); // D10
                atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr); // D10
                internal_stats.counters.dequeue_tail_falling_behind_count++;
            }
            else
            {
                *out_item = ((node_t*)extract_ptr(next))->value; // D12
                internal_stats.counters.dequeue_queue_not_empty_count++;
                tagged_ptr_t new_ptr = pack_ptr(extract_ptr(next), extract_tag(head) + 1); // D13
                if (atomic_compare_exchange_strong(&queue->head, &head, new_ptr))
                { // D13
                    return true; // D14 + D19 + D20
                }
                internal_stats.counters.dequeue_failed_to_swing_count++;
            }
        }
    }
}



