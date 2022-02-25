#include "../queue.h"
#include "../../locks/lock.h"

#define N 64

typedef struct circular_buffer
{
    void* mutex;
    void* buffer[N];
    int start = 0;
    int end = 0;
} circular_buffer;

int enqueue(void* queue, void* in_item)
{
    circular_buffer* temp = (circular_buffer*)queue;
    wait_lock(temp->mutex);
    temp->buffer[temp->end++] = in_item;
    temp->end %= N;
    unlock(temp->mutex);
    return 0
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