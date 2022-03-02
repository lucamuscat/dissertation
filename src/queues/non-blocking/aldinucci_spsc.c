#include "aldinucci_spsc.h"

bool enqueue(void* queue, void* in_item)
{
    return spsc_enqueue(queue, in_item);
}

bool dequeue(void* queue, void** out_item)
{
    return spsc_dequeue(queue, out_item);
}

bool create_queue(void** out_queue)
{
    return create_queue(out_queue);
}