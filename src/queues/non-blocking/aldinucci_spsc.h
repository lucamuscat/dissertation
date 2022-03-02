#ifndef ALDINUCCI_H
#define ALDINUCCI_H

// https://link.springer.com/content/pdf/10.1007%2F978-3-642-32820-6_65.pdf
// Fig.1
#include<stdlib.h>
#include "../queue.h"

typedef struct aldinucci_spsc
{
    void** buffer;
    // The original paper assumes that variables are naturally aligned.
    int read __attribute__((aligned(sizeof(int))));
    int write __attribute__((aligned(sizeof(int))));
} aldinucci_spsc;

bool create_spsc_queue(void** out_queue)
{
    aldinucci_spsc** temp = (aldinucci_spsc**)out_queue;
    *temp = (aldinucci_spsc*)calloc(1, sizeof(aldinucci_spsc));
    P_PASS(*temp);
    (*temp)->buffer = (void**)calloc(CIRCULAR_BUFFER_SIZE, sizeof(void*));
    P_PASS((void*)(*temp)->buffer);
    return true;
}

bool spsc_enqueue(void* queue, void* in_item)
{
    struct aldinucci_spsc* temp = queue;
    if (temp->buffer[temp->write] == NULL)
    {
        asm("sfence"); // WMB
        temp->buffer[temp->write++] = in_item;
        if (temp->write == CIRCULAR_BUFFER_SIZE)
            temp->write = 0;
        return true;
    }
    return false;
}

bool spsc_dequeue(void* queue, void** out_item) \
{
    struct aldinucci_spsc* temp = queue;
    if (temp->buffer[temp->read] == NULL)
        return false;
    *out_item = temp->buffer[temp->read];
    temp->buffer[temp->read++] = NULL;
    if (temp->read == CIRCULAR_BUFFER_SIZE)
        temp->read = 0;
    return true;
}

#endif