#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
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
    assert(min >= 0);
    assert(max > 0);
    assert(max > min);
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

void handle_queue_args(int argc, char** argv, int* out_num_of_thread, int* delay_ns)
{
    if (argc != 3)
    {
        fprintf(stderr, "Missing args, arg1 - Thread count, arg2 - reentrancy delay (ns)\n");
        exit(EXIT_FAILURE);
    }

    char* pEnd;
    char* pEnd2;
    *out_num_of_thread = strtol(argv[1], &pEnd, 10);
    *delay_ns = strtol(argv[2], &pEnd2, 10);
}

inline int iterations_per_thread(int num_of_threads, int thread_num, int total_iterations)
{
    assert(num_of_threads > 0);
    const int iterations_per_thread = (int)floor(((double)total_iterations) / num_of_threads);
    const int final_thread_iterations = total_iterations - (iterations_per_thread * (num_of_threads - 1));
    assert((iterations_per_thread * (num_of_threads - 1) + final_thread_iterations) == total_iterations);
    if (thread_num == num_of_threads - 1)
        return final_thread_iterations;
    return iterations_per_thread;
}

double mean(double* values, size_t N)
{
    double acc = values[0];
    for (size_t i = 1; i < N; ++i)
    {
        acc += values[i];
    }
    return acc / (double)N;
}

double stdev(double* values, size_t N)
{
    double avg = mean(values, N);
    double acc = 0.0;
    for (size_t i = 0; i < N; ++i)
    {
        double temp = ((double)values[i]) - avg;
        acc += temp * temp;
    }
    acc /= N;
    return sqrt(acc);
}