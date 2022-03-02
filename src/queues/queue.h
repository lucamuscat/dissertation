#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdbool.h>

#define CIRCULAR_BUFFER_SIZE 64

/**
 * @brief Enqueue an item inside the queue.
 * 
 * @param queue Queue in which the item will be enqueued.
 * @param in_item Item which will be enqueued inside of the queue.
 * @return Status
 */
bool enqueue(void* queue, void* in_item);

/**
 * @brief Dequeue an item from the queue.
 * 
 * @param queue Queue in which an item will be dequeued
 * @param out_item Item in which the dequeued item will be stored.
 * @return Status
 */
bool dequeue(void* queue, void** out_item);

/**
 * @brief Initializes a queue.
 * 
 * @param out_queue Pointer to the queue that is to be initialized
 * @return Status
 */
bool create_queue(void** out_queue);

// A randomly poke method might come in handy in the future:
// https://youtu.be/_qaKkHuHYE0?t=1073

/**
 * @brief Exit if x is NULL. This macro can be used to ensure that mallocs and
 * callocs succeed.
 */
#define P_PASS(x) if(x == NULL) {\
    perror("Error:"); \
    exit(EXIT_FAILURE); \
}

/// Return x if it is false.
#define PASS(x) if(!x) {\
    return x; \
}

#endif
