#include "aldinucci_dspsc.h"
#include "../queue.h"

bool create_queue(void** out_queue)
{
    return create_dspsc(out_queue);
}

bool enqueue(void* queue, void* in_data)
{
    return dspsc_enqueue(queue, in_data);
}

bool dequeue(void* queue, void** out_data)
{
    return dspsc_dequeue(queue, out_data);
}

char* get_queue_name()
{
    return "Aldinucci DSPSC";
}