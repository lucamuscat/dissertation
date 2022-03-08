#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "test_utils.h"

// TODO: Find a way to find the current CPU clock frequency on runtime

int handle_args(int argc, char* argv[])
{
    if (argc != 2)
    {
        puts("Missing args");
        return -1;
    }

    char* pEnd;
    const int decimal = 10;
    const long int number_of_threads = strtol(argv[1], &pEnd, decimal);
    return number_of_threads;
}

// https://stackoverflow.com/a/18386648
int bounded_random(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

int* generate_random_numbers(int n, int min, int max)
{
    assert(min >= 0 && max > 0 && max > min);
    int* result = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; ++i)
    {
        result[i] = bounded_random(min, max);
        assert(result[i] >= min && result[i] <= max);
    }
    return result;
}

/**
 * @brief Do not use. Use the DELAY macro instead.
 * 
 */
__attribute__((always_inline)) inline void _delay(size_t ns)
{
    for (size_t i = 0; i < ns; ++i)
        __asm__("nop");
}