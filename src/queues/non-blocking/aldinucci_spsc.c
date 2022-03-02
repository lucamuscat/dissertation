#include "aldinucci_spsc.h"

int enqueue(void* queue, void* in_item)
{
    return spsc_enqueue(queue, in_item);
}

int dequeue(void* queue, void** out_item)
{
    return spsc_dequeue(queue, out_item);
}

int create_queue(void** out_queue)
{
    return create_queue(out_queue);
}