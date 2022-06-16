#ifndef AUX_BASKETS_QUEUE
#define AUX_BASKETS_QUEUE
#include <threads.h>
#include "../auxiliary_stats.h"

typedef union stats_t
{
    struct counters_t
    {
        int enqueue_count; // E05-E06
        int enqueue_consistent_tail_count; // E06-E07
        int enqueue_next_ptr_null_count; // E07-E08
        int enqueue_cas_tail_count; // E09-E10
        int enqueue_build_basket_count; // E18-E19
        int enqueue_next_ptr_not_null_count; // E21-E22
        int dequeue_count; // D04-D05
        int dequeue_consistent_head_count; // D06-D07
        int dequeue_single_item_count; // D06-D07
        int dequeue_empty_count; // D07-D08
        int dequeue_inconsistent_head_count; // D19-D20
        int dequeue_consistent_iter_tail_count; // D21-D22
        int dequeue_cas_failed_count; // D28 - D29
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