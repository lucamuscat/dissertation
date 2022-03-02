#ifndef ALDINUCCI_DSPSC_H
#define ALDINUCCI_DSPSC_H

#include "aldinucci_spsc.h"

typedef struct node
{
    void* data;
    struct node* next;
} node;

typedef struct dspsc_queue
{
    node* head;
    node* tail;
    void* cache;
} dspsc_queue;

bool create_dspsc(void** out_queue)
{
    node* dummy_node = (node*)calloc(1, sizeof(node));
    P_PASS(dummy_node);
    void* cache;
    PASS(create_spsc_queue(&cache));

    dspsc_queue** temp = (dspsc_queue**)out_queue;
    *temp = (dspsc_queue*)malloc(sizeof(dspsc_queue));
    P_PASS(*temp);
    (*temp)->head = dummy_node;
    (*temp)->tail = dummy_node;
    (*temp)->cache = cache;

    return true;
}

int dspsc_enqueue(void* queue, void* in_data)
{
    node* n;
    dspsc_queue* temp = queue;
    if (!spsc_dequeue(temp->cache, (void**)(&n)))
        n = (node*)malloc(sizeof(node*));
    n->data = in_data;
    n->next = NULL;
    asm("sfence");
    return true;
}

int dspsc_dequeue(void* queue, void** out_data)
{
    dspsc_queue* temp = queue;
    if (!temp->head->next)
        return false;
    node* n = temp->head;
    *out_data = (temp->head->next)->data;
    temp->head = temp->head->next;
    if (!spsc_enqueue(temp->cache, n)) free(n);
    return true;
}
#endif