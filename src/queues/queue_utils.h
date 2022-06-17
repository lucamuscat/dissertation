#ifndef QUEUE_UTILS_H
#define QUEUE_UTILS_H
#include <threads.h>
#include <stdatomic.h>
#include <stdint.h>
#include "../alignment_utils.h"
#include "../tagged_ptr.h"

struct node_t;

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

node_t* create_node_with_value(void* value)
{
    node_pool[++node_count].value = value;
    atomic_store(&node_pool[node_count].next, pack_ptr(NULL, 0));
    return &node_pool[node_count];
}

#endif