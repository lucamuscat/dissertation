#include "aldinucci_spsc.h"

int enqueue(void* queue, void* in_item)
{
    return _enqueue(queue, in_item);
}

int dequeue(void* queue, void** out_item)
{
    return _dequeue(queue, out_item);
}

int create_queue(void** out_queue)
{
    return _create_queue(out_queue);
}