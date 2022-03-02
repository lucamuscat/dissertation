#include "aldinucci_dspsc.h"
#include "../queue.h"

int create_queue(void** out_queue)
{
    return create_dspsc(out_queue);
}

int enqueue(void* queue, void* in_data)
{
    return dspsc_enqueue(queue, in_data);
}

int dequeue(void* queue, void** out_data)
{
    return dspsc_dequeue(queue, out_data);
}