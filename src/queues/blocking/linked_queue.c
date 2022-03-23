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

typedef struct node_t
{
    void* value;
    struct node_t* next;
} node_t;

typedef struct queue
{
    void* head_lock;
    void* tail_lock;
    node_t* head; // Head
    node_t* tail; // Tail/Rear
} queue;

bool create_node(void* data, node_t** out_node)
{
    *out_node = malloc(sizeof(node_t));
    ASSERT_NOT_NULL(out_node);
    (*out_node)->next = NULL;
    (*out_node)->value = data;
    return true;
}

bool create_queue(void** out_queue)
{
    *out_queue = calloc(1, sizeof(queue));
    ASSERT_NOT_NULL(*out_queue);
    queue** temp = (queue**)out_queue;
    node_t* sentinel;
    PASS(create_node(NULL, &sentinel));
    PASS(create_lock(&(*temp)->head_lock));
    PASS(create_lock(&(*temp)->tail_lock));
    (*temp)->head = sentinel;
    (*temp)->tail = sentinel;

    return true;
}

bool enqueue(void* in_queue, void* in_data)
{
    queue* temp = in_queue;
    node_t* node; // New node
    create_node(in_data, &node); // New node
    node->next = NULL; // Set next pointer to null
    wait_lock(temp->tail_lock); // Acquire enq lock
    temp->tail->next = node; // Link node at the end of the linked list
    temp->tail = node; // Swing tail to node
    unlock(temp->tail_lock); // Release
    return true;
}

bool dequeue(void* in_queue, void** out_data)
{
    queue* temp = in_queue;
    wait_lock(temp->head_lock);
    node_t* node = temp->head;
    node_t* new_head = node->next;
    if (new_head == NULL)
    {
        unlock(temp->head_lock);
        return false;
    }
    *out_data = new_head->value;
    temp->head = new_head;
    unlock(temp->head_lock);
    free(node);
    return true;
}

char* get_queue_name()
{
    char* buffer = (char*)malloc(sizeof(char) * 64);
    snprintf(buffer, 64, "%s %s", "Linked Queue", get_lock_name());
    return buffer;
}