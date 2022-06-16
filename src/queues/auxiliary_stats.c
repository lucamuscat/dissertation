#ifndef AUX_STATS_H
#define AUX_STATS_H
#include <threads.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Get the number of counters inside of a stats object
 */
size_t get_num_of_counters();
void* get_thread_local_counters();

void add_stats(int* a, int* b, int* dest)
{
    for (size_t i = 0; i < get_num_of_counters(); ++i)
    {
        dest[i] = a[i] + b[i];
    }
}

/**
 * @brief Zero-out the provided counter.
 *
 * @param a Counter that is zeroed-out.
 */
void reset_stats(int* a)
{
    memset(a, 0, get_num_of_counters());
}

/**
 * @brief Copy a stats struct onto another. `src` is copied into `dest`.
 *
 */
void copy_stats(int* dest, int* src)
{
    memcpy(dest, src, get_num_of_counters());
}

/**
 * @brief Print array of counters in their integer form
 *
 * @param src Array of counters that are displayed
 * @param size The number of items in `src`
 */
void display_stats(int** src, size_t size)
{
    printf(",");
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < get_num_of_counters() / sizeof(int); ++j)
        {
            printf("%d ", src[i][j]);
        }
    }
}

/**
 * @brief Create multiple stat objects
 *
 * @param size Number of stat objects to be created.
 * @return void* A zeroed, contiguously allocated array containing `size` number
 * of stats objects.
 */
int** create_stats(size_t size)
{
    int** ret_value = malloc(sizeof(int*) * size);
    for (size_t i = 0; i < size; ++i)
    {
        ret_value[i] = calloc(size, get_num_of_counters());
    }
    return ret_value;
}

/**
 * @brief Copy thread_local stats.
 *
 * @param dest Destination to where the stats will be copied to.
 */
void get_stats(int* dest)
{
    memcpy(dest, get_thread_local_counters(), get_num_of_counters());
}

#endif