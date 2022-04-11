/**
 * @file linked_queue.c
 * @brief Two-lock concurrent queue as described in Michael & Scott's "Simple, Fast
 * and Practical Non-Blocking and Blocking Concurrent Queue Algorithms"
 *
 */

#include "../queue.h"
#include "../../locks/lock.h"
#include "../../test_utils.h"

#include <stdlib.h>
#include <threads.h>
#include <stdint.h>
#include <stdatomic.h>

typedef struct node_t
{
    void* _Atomic value;
    struct node_t* _Atomic next;
} node_t;

typedef struct queue
{
    void* head_lock;
    void* tail_lock;
    node_t* _Atomic head; // Head
    node_t* _Atomic tail; // Tail/Rear
} queue;

thread_local node_t* sentinel;
thread_local char pad1[PAD_TO_CACHELINE(node_t*)];
thread_local int64_t node_counter;
thread_local char pad3[PAD_TO_CACHELINE(int64_t)];
thread_local node_t* node_pool;
thread_local char pad2[PAD_TO_CACHELINE(node_t*)];

void register_thread(size_t num_of_iterations)
{
    node_pool = (node_t*)malloc(sizeof(node_t) * num_of_iterations);
    ASSERT_NOT_NULL(node_pool);
    node_counter = -1;
}

void cleanup_thread()
{
    free(node_pool);
}

bool create_node(void* data, node_t** out_node)
{
    *out_node = &node_pool[++node_counter];
    atomic_store(&(*out_node)->next, NULL);
    atomic_store(&(*out_node)->value, data);
    return true;
}

bool create_queue(void** out_queue)
{
    *out_queue = calloc(1, sizeof(queue));
    ASSERT_NOT_NULL(*out_queue);
    queue** temp = (queue**)out_queue;
    node_t* sentinel;
    sentinel = (node_t*)malloc(sizeof(node_t));
    atomic_store(&sentinel->next, NULL);
    atomic_store(&sentinel->value, NULL);
    PASS(create_lock(&(*temp)->head_lock));
    PASS(create_lock(&(*temp)->tail_lock));
    atomic_store(&(*temp)->head, sentinel);
    atomic_store(&(*temp)->tail, sentinel);

    return true;
}

void destroy_queue(void** out_queue)
{
    free(*out_queue);
}

bool enqueue(void* in_queue, void* in_data)
{
    queue* queue = in_queue;
    node_t* node; // New node
    create_node(in_data, &node); // New node
    wait_lock(queue->tail_lock); // Acquire enq lock

    node_t* ltail = atomic_load(&queue->tail);
    atomic_store(&ltail->next, node);
    //queue->tail->next = node; // Link node at the end of the linked list
    atomic_store(&queue->tail, node);
    //queue->tail = node; // Swing tail to node
    unlock(queue->tail_lock); // Release
    return true;
}

bool dequeue(void* in_queue, void** out_data)
{
    queue* queue = in_queue;
    wait_lock(queue->head_lock);
    node_t* node = atomic_load(&queue->head);
    node_t* new_head = atomic_load(&node->next);
    if (new_head == NULL)
    {
        unlock(queue->head_lock);
        return false;
    }
    *out_data = atomic_load(&new_head->value);
    atomic_store(&queue->head, new_head);
    unlock(queue->head_lock);
    //free(node);
    return true;
}

char* get_queue_name()
{
    char* buffer = (char*)malloc(sizeof(char) * 64);
    snprintf(buffer, 64, "%s %s", "MS Two-Lock", get_lock_name());
    return buffer;
}