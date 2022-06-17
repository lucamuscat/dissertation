#ifndef AUX_VALOIS_QUEUE_H
#define AUX_VALOIS_QUEUE_H
#include <threads.h>
#include <string.h>
#include "../auxiliary_stats.h"

typedef union stats_t
{
    struct counters_t
    {
        int enqueue_count; // Between E3-E4
        int enqueue_retry_count;
        int dequeue_count;
        int dequeue_empty_count;
        int dequeue_non_empty_count;
        int dequeue_retry_count;
    } counters;
    int data[sizeof(struct counters_t) / sizeof(int)];
} stats_t;

thread_local stats_t internal_stats = { 0 };

size_t get_num_of_counters()
{
    return sizeof(stats_t);
}

void* get_thread_local_counters()
{
    return &internal_stats;
}

#endif