#include <stdint.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <threads.h>
#include "../queue.h"
#include "../../test_utils.h"

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

struct node_t* create_node(void* value);

#ifdef DWCAS
char* get_queue_name()
{
    return "MS Queue using DWCAS";
}

// Align to 16 bytes to ensure that cmpxcgh16b is used.
typedef struct node_pointer_t
{
    struct node_t* ptr;
    uint64_t count;
} DWCAS_ALIGNED node_pointer_t;
#else
#include "../../tagged_ptr.h"
typedef tagged_ptr_t node_pointer_t;
char* get_queue_name()
{
    return "MS Queue using Tagged Pointers";
}

#endif

typedef struct node_t
{
    node_pointer_t _Atomic next;
    void* value;
} node_t;

bool equals(node_pointer_t a, node_pointer_t b);

#ifdef DWCAS
inline bool equals(node_pointer_t a, node_pointer_t b)
{
    return a.count == b.count && a.ptr == b.ptr;
}
#define pack_ptr(_ptr, _count) ((node_pointer_t){.ptr = (_ptr), .count = (_count)})
#define extract_ptr(_ptr) (node_t*)(((node_pointer_t)(_ptr)).ptr)
#define extract_tag(_ptr) (((node_pointer_t)(_ptr)).count)
#define get_next_ptr(_ptr) (((node_t*)((node_pointer_t)(_ptr)).ptr)->next)
#else
inline bool equals(node_pointer_t a, node_pointer_t b)
{
    return a == b;
}

#define get_next_ptr(_ptr) (((node_t*)(extract_ptr((_ptr))))->next)

#endif

typedef struct queue_t
{
    // Needs to be atomic, these pointers will be accessed from multiple threads.
    DOUBLE_CACHE_ALIGNED node_pointer_t _Atomic head;
    DOUBLE_CACHE_ALIGNED node_pointer_t _Atomic tail;
} queue_t;

thread_local node_t* sentinel;
thread_local char pad1[PAD_TO_CACHELINE(node_t*)];
thread_local int64_t node_count;
thread_local char pad2[PAD_TO_CACHELINE(int64_t)];
thread_local node_t* node_pool;
thread_local char pad3[PAD_TO_CACHELINE(node_t*)];

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
inline node_t* create_node(void* value)
{
    node_pool[++node_count].value = value;
    //atomic_store(&node_pool[node_count].next, ((node_pointer_t){NULL, 0}));
    atomic_store(&node_pool[node_count].next, pack_ptr(NULL, 0));

    return &node_pool[node_count];
}

void create_sentinel_node()
{
    sentinel = (node_t*)calloc(1, sizeof(node_t));
    assert(atomic_is_lock_free(&sentinel->next));
    atomic_store(&sentinel->next, pack_ptr(NULL, 0));
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
    node_pointer_t sentinel_ptr = pack_ptr(sentinel, 0);
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
    node_t* node = create_node(in_item);
    node_pointer_t tail, next;
    
    while (true) // loop
    {
        // Check if the memory ordering can be weakened safely.
        tail = atomic_load(&queue->tail);
        next = atomic_load(&get_next_ptr(tail));
        if (equals(tail, atomic_load(&queue->tail)))
        {
            if (extract_ptr(next) == NULL)
            {
                //node_pointer_t new_ptr = { node, next.count + 1 };
                node_pointer_t new_ptr = pack_ptr(node, extract_tag(next) + 1);
                // TODO: Check if performance boost is gained when using
                // atomic_compare_exchange_weak inside of a while loop.
                if (atomic_compare_exchange_strong(&get_next_ptr(tail), &next, new_ptr))
                {
                    node_pointer_t new_ptr = pack_ptr(node, extract_tag(tail) + 1);
                    atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
                    return true;
                }
            }
            else
            {
                node_pointer_t new_ptr = pack_ptr(extract_ptr(next), extract_tag(tail) + 1);
                atomic_compare_exchange_strong(&queue->tail, &next, new_ptr);
            }
        }
    }
}

bool dequeue(void* in_queue, void** out_item)
{
    queue_t* queue = (queue_t*)in_queue;
    node_pointer_t head, tail, next;
    while (true)
    {
        head = atomic_load(&queue->head);
        tail = atomic_load(&queue->tail);
        next = atomic_load(&get_next_ptr(head));
        if (equals(head, atomic_load(&queue->head)))
        {
            if (extract_ptr(head) == extract_ptr(tail))
            {
                if (extract_ptr(next) == NULL)
                    return false;
                node_pointer_t new_ptr = pack_ptr(extract_ptr(next), extract_tag(tail) + 1);
                atomic_compare_exchange_strong(&queue->tail, &tail, new_ptr);
            }
            else
            {
                *out_item = ((node_t*)extract_ptr(next))->value;
                node_pointer_t new_ptr = pack_ptr(extract_ptr(next), extract_tag(head) + 1);
                if (atomic_compare_exchange_strong(&queue->head, &head, new_ptr))
                    return true;
            }
        }
    }
}



