#ifndef QUEUE_H
#define QUEUE_H

/**
 * @brief Enqueue an item inside the queue.
 * 
 * @param queue Queue in which the item will be enqueued.
 * @param in_item Item which will be enqueued inside of the queue.
 * @return Status
 */
int enqueue(void* queue, void* in_item);

/**
 * @brief Dequeue an item from the queue.
 * 
 * @param queue Queue in which an item will be dequeued
 * @param out_item Item in which the dequeued item will be stored.
 * @return Status
 */
int dequeue(void* queue, void** out_item);

/**
 * @brief Initializes a queue.
 * 
 * @param out_queue Pointer to the queue that is to be initialized
 * @return Status
 */
int create_queue(void** out_queue);

// A randomly poke method might come in handy in the future:
// https://youtu.be/_qaKkHuHYE0?t=1073

#endif
