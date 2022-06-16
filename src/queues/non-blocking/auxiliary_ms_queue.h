#ifndef AUX_MS_QUEUE_H
#define AUX_MS_QUEUE_H
#include <threads.h>
#include <string.h>
#include "../auxiliary_stats.h"

typedef union stats_t
{
    struct counters_t
    {
        int enqueue_count; // Between E3-E4
        int enqueue_consistent_tail_count; // Between E7-E8
        int enqueue_next_ptr_null_count; // Between E8-E9
        int enqueue_next_ptr_not_null_count; // Between E12-E13
        int enqueue_cas_tail_count; // E9-E10
        int dequeue_count; // D4-D5
        int dequeue_consistent_head_count; // D5-D6
        int dequeue_empty_queue_count; // D7-D8
        int dequeue_tail_falling_behind_count; // D10-D11
        int dequeue_queue_not_empty_count; // D12-D13
        int dequeue_failed_to_swing_count; // D15-D16
    } counters_t;
    struct counters_t counters;
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