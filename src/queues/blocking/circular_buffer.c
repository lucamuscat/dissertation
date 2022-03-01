#include <stdlib.h>
#include <assert.h>
#include "../queue.h"
#include "../../locks/lock.h"

#define N 64
#define EMPTY 1
#define FULL 1

#define P_QUEUE(x) ((circular_buffer*) x)

typedef struct circular_buffer
{
    void** mutex;
    void** buffer;
    int read;
    int write;
} circular_buffer;

//TODO: Need to add better error checking
int create_queue(void** out_queue)
{
    *out_queue = malloc(sizeof(circular_buffer));
    P_QUEUE(*out_queue)->read = 0;
    P_QUEUE(*out_queue)->write = 0;
    // Initialize buffer with 0s (nulls)
    P_QUEUE(*out_queue)->buffer = (void**)calloc(N, sizeof(void*));
    P_QUEUE(*out_queue)->mutex = NULL;
    int status = create_lock(P_QUEUE(out_queue)->mutex);
    assert((status != -1));
    assert((P_QUEUE(*out_queue)->mutex != NULL));
    return status;
}

int enqueue(void* queue, void* in_item)
{
    circular_buffer* temp = (circular_buffer*)queue;
    wait_lock(temp->mutex);
    if (temp->buffer[temp->write] == NULL)
    {
        temp->buffer[temp->write++] = in_item;
        temp->write %= N;
        unlock(temp->mutex);
        return 0;
    }
    unlock(temp->mutex);
    return FULL;
}

int dequeue(void* queue, void** out_item)
{
    circular_buffer* temp = (circular_buffer*)queue;
    wait_lock(temp->mutex);
    if (temp->buffer[temp->read] == NULL)
    {
        unlock(temp->mutex);
        return FULL;
    }
    out_item = temp->buffer[temp->read++];
    temp->read %= N;
    unlock(temp->mutex);
    return 0;
}