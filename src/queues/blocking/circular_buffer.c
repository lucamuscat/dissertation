#include <stdlib.h>
#include "../queue.h"
#include "../../locks/lock.h"

#define N 64

typedef struct circular_buffer
{
    void* mutex;
    void** buffer;
    int start;
    int end;
} circular_buffer;

//TODO: Need to add better error checking
int create_queue(void** out_queue)
{
    circular_buffer** result_queue = (circular_buffer**)out_queue;
    *result_queue = (circular_buffer*)malloc(sizeof(circular_buffer));
    (*result_queue)->start = 0;
    (*result_queue)->end = 0;
    (*result_queue)->buffer = (void**)malloc(sizeof(void*) * N);
    return create_lock((void**)(&(*result_queue)->mutex));
}

int enqueue(void* queue, void* in_item)
{
    circular_buffer* temp = (circular_buffer*)queue;
    wait_lock(temp->mutex);
    temp->buffer[temp->end++] = in_item;
    temp->end %= N;
    unlock(temp->mutex);
    return 0;
}

int dequeue(void* queue, void* out_item)
{
    circular_buffer* temp = (circular_buffer*)queue;
    wait_lock(temp->mutex);
    out_item = temp->buffer[temp->start++];
    temp->start %= N;
    unlock(temp->mutex);
    return 0;
}