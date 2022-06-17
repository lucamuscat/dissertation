#ifndef MS_TWO_LOCK_BASKETS_QUEUE
#define MS_TWO_LOCK_BASKETS_QUEUE
#include <threads.h>
#include "../auxiliary_stats.h"

typedef union stats_t
{
    struct counters_t
    {
        int enqueue_count;
        int dequeue_count;
        int dequeue_empty_count;
        int dequeue_non_empty_count;
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