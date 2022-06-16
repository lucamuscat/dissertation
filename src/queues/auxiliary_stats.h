#include <threads.h>
#include <string.h>

/**
 * @brief Get the number of counters inside of a stats object
 */
size_t get_num_of_counters();
void* get_thread_local_counters();

void add_stats(int* a, int* b, int* dest);

/**
 * @brief Zero-out the provided counter.
 *
 * @param a Counter that is zeroed-out.
 */
void reset_stats(int* a);

/**
 * @brief Copy a stats struct onto another. `src` is copied into `dest`.
 *
 */
void copy_stats(int* dest, int* src);

/**
 * @brief Print array of counters in their integer form
 *
 * @param src Array of counters that are displayed
 * @param size The number of items in `src`
 */
void display_stats(int** src, size_t size);

/**
 * @brief Create multiple stat objects
 *
 * @param size Number of stat objects to be created.
 * @return void* A zeroed, contiguously allocated array containing `size` number
 * of stats objects.
 */
int** create_stats(size_t size);

/**
 * @brief Copy thread_local stats.
 *
 * @param dest Destination to where the stats will be copied to.
 */
void get_stats(int* dest);