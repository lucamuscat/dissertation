// https://link.springer.com/content/pdf/10.1007%2F978-3-642-32820-6_65.pdf
// Fig.1
#include<stdlib.h>
#include "../queue.h"

#define N 64

typedef struct queue
{
    void** buffer;
    // The original paper assumes that variables are naturally aligned.
    int read __attribute__((aligned(sizeof(int))));
    int write __attribute__((aligned(sizeof(int))));
} queue;

int create_queue(void** out_queue)
{
    queue** temp = (queue**)out_queue;
    P_PASS((*temp = (queue*)calloc(1, sizeof(queue))));
    P_PASS(((*temp)->buffer = calloc(N, sizeof(void*))));
    return 0;
}

int enqueue(void* queue, void* in_item)
{
    struct queue* temp = queue;
    if (temp->buffer[temp->write] == NULL)
    {
        asm("sfence"); // WMB
        temp->buffer[temp->write++] = in_item;
        temp->write %= N;
        return 0;
    }
    return -1;
}

int dequeue(void* queue, void** out_item)
{
    struct queue* temp = queue;
    if (temp->buffer[temp->read] == NULL)
        return -1;
    *out_item = temp->buffer[temp->read];
    temp->buffer[temp->read++] = NULL;
    temp->read %= N;
    return 0;
}

