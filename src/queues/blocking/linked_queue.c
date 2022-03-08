#include "../queue.h"
#include "../../locks/lock.h"

#include <stdlib.h>

typedef struct node
{
    void* value;
    struct node* next;
} node;

typedef struct queue
{
    void* enqueue_lock __attribute__((aligned(CACHE_LINE_SIZE)));
    void* dequeue_lock __attribute__((aligned(CACHE_LINE_SIZE)));
    node* read __attribute__((aligned(CACHE_LINE_SIZE))); // Head
    node* write __attribute__((aligned(CACHE_LINE_SIZE))); // Tail/Rear
} queue;

bool create_node(void* data, node** out_node)
{
    *out_node = malloc(sizeof(node));
    P_PASS(out_node);
    (*out_node)->next = NULL;
    (*out_node)->value = data;
    return true;
}

bool create_queue(void** out_queue)
{
    *out_queue = calloc(1, sizeof(queue));
    P_PASS(*out_queue);
    queue** temp = (queue**)out_queue;
    node* sentinel;
    PASS(create_node(NULL, &sentinel));
    PASS(create_lock(&(*temp)->enqueue_lock));
    PASS(create_lock(&(*temp)->dequeue_lock));
    (*temp)->read = sentinel;
    (*temp)->write = sentinel;

    return true;
}

bool enqueue(void* in_queue, void* in_data)
{
    queue* temp = in_queue;
    node* node;
    wait_lock(temp->enqueue_lock);
    create_node(in_data, &node);
    temp->write->next = node;
    temp->write = node;
    unlock(temp->enqueue_lock);
    return true;
}

bool dequeue(void* in_queue, void** out_data)
{
    queue* temp = in_queue;
    wait_lock(temp->dequeue_lock);
    if (temp->read->next == NULL)
    {
        unlock(temp->dequeue_lock);
        return false;
    }
    *out_data = temp->read->next->value;
    temp->read = temp->read->next;
    unlock(temp->dequeue_lock);
    return true;
}

char* get_queue_name()
{
    char* buffer = (char*)malloc(sizeof(char) * 64);
    snprintf(buffer, 64, "%s %s", "Linked Queue", get_lock_name());
    return buffer;
}