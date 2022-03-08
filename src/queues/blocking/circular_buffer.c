#include <stdlib.h>
#include <assert.h>
#include "../queue.h"
#include "../../locks/lock.h"
#include "../../test_utils.h"

#define EMPTY 1
#define FULL 1

#define P_QUEUE(x) ((circular_buffer*) x)

/* Potential scalability issues as the circular buffer is shared across each thread.
 * One possible improvement would be to make sure that each field takes its own
 * cacheline.
 * Another potential improvement could be to keep a private copy of the cache line
 * and update the global context (act locally).

 * Currently unscalable, as writes are shared. 
*/
typedef struct circular_buffer
{
    void* mutex;
    void** buffer;
    int read;
    int write;
} circular_buffer;

bool create_queue(void** out_queue)
{
    *out_queue = calloc(1, sizeof(circular_buffer));
    P_PASS(*out_queue);
    // Initialize buffer with 0s (nulls)
    P_QUEUE(*out_queue)->buffer = (void**)calloc(CIRCULAR_BUFFER_SIZE, sizeof(void*));
    PASS(create_lock(&P_QUEUE(*out_queue)->mutex));
    assert((P_QUEUE(*out_queue)->mutex != NULL));
    return true;
}

// Should I add a semaphore that waits when its full for enq, or empty for deq?
bool enqueue(void* queue, void* in_item)
{
    circular_buffer* temp = (circular_buffer*)queue;
    wait_lock(temp->mutex);
    if (temp->buffer[temp->write] == NULL)
    {
        temp->buffer[temp->write++] = in_item;
        if (temp->write == CIRCULAR_BUFFER_SIZE)
        {
            temp->write = 0;
        }
        unlock(temp->mutex);
        return true;
    }
    unlock(temp->mutex);
    return false;
}

bool dequeue(void* queue, void** out_item)
{
    circular_buffer* temp = (circular_buffer*)queue;
    wait_lock(temp->mutex);
    if (temp->buffer[temp->read] == NULL)
    {
        unlock(temp->mutex);
        return true;
    }
    *out_item = temp->buffer[temp->read++];
    if (temp->read == CIRCULAR_BUFFER_SIZE)
    {
        temp->read = 0;
    }
    unlock(temp->mutex);
    return false;
}

char* get_queue_name()
{
    char* buffer = (char*)malloc(sizeof(char) * 64);
    snprintf(buffer, 64, "%s %s", "Circular Buffer", get_lock_name());
    return buffer;
}