#ifndef ALDINUCCI_H
#define ALDINUCCI_H

// https://link.springer.com/content/pdf/10.1007%2F978-3-642-32820-6_65.pdf
// Fig.1
#include<stdlib.h>
#include "../queue.h"

typedef struct queue
{
    void** buffer;
    // The original paper assumes that variables are naturally aligned.
    int read __attribute__((aligned(sizeof(int))));
    int write __attribute__((aligned(sizeof(int))));
} queue;

int _create_queue(void** out_queue)
{
    queue** temp = (queue**)out_queue;
    *temp = (queue*)calloc(1, sizeof(queue));
    P_PASS(*temp);
    (*temp)->buffer = (void**)calloc(CIRCULAR_BUFFER_SIZE, sizeof(void*));
    P_PASS((void*)(*temp)->buffer);
    return 0;
}

int _enqueue(void* queue, void* in_item)
{
    struct queue* temp = queue;
    if (temp->buffer[temp->write] == NULL)
    {
        asm("sfence"); // WMB
        temp->buffer[temp->write++] = in_item;
        temp->write %= CIRCULAR_BUFFER_SIZE;
        return 0;
    }
    return -1;
}

int _dequeue(void* queue, void** out_item) \
{
    struct queue* temp = queue;
    if (temp->buffer[temp->read] == NULL)
        return -1;
    *out_item = temp->buffer[temp->read];
    temp->buffer[temp->read++] = NULL;
    temp->read %= CIRCULAR_BUFFER_SIZE;
    return 0;
}

#endif