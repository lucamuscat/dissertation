#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <x86intrin.h>
#include <math.h>
#include <stdint.h>
#include <papi.h>
#include "test_utils.h"
#include "assertion_utils.h"

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

__attribute__((always_inline)) inline void calibrate_delay(delay_t* delay, size_t expected_delay_ns)
{
    if (expected_delay_ns == 0)
    {
        delay->delay_ns = 0;
        delay->num_of_nops = 0;
        return;
    }

    const size_t RERUNS = 5;
    const size_t iterations = 10000000;
    readings_t* delay_readings = create_readings(RERUNS);
    for (size_t j = 0; j < RERUNS; ++j)
    {
        start_readings(delay_readings);
        for (size_t i = 0; i < iterations; ++i)
        {
            DELAY(expected_delay_ns);
        }
        delta_readings(delay_readings, iterations);
    }
    readings_t* aggregated_readings = aggregate_readings(delay_readings, RERUNS);
    double mean_ns = aggregated_readings->nano_seconds[0];
    double error_ns = (fabs((double)expected_delay_ns - mean_ns) / expected_delay_ns);
    delay->delay_ns = expected_delay_ns;
    delay->num_of_nops = expected_delay_ns * CPU_GHZ * (1 - error_ns);
    destroy_readings(&delay_readings);
}

/**
 * @brief Do not use. Use the DELAY macro instead.
 * Source: https://github.com/pramalhe/ConcurrencyFreaks/blob/c61189546805c67792df7931f9484e09a3cda3bf/CPP/pqueues/pfences.h#L33
 */
__attribute__((always_inline)) inline void _delay(uint64_t cycles)
{
    /*
    for (size_t i = 0; i < cycles; ++i)
        __asm__("nop");
    */
    uint64_t stop;
    uint64_t start = _rdtsc();
    do
    {
        stop = _rdtsc();
    } while (stop - start < cycles);
}

void handle_queue_args(int argc, char** argv, size_t* out_num_of_thread, size_t* out_delay_ns)
{
    if (argc != 3)
    {
        fprintf(stderr, "Missing args,\n\targ1 - Thread count,\n\targ2 - reentrancy delay (ns)\n");
        exit(EXIT_FAILURE);
    }

    char* pEnd;
    char* pEnd2;
    *out_num_of_thread = strtoul(argv[1], &pEnd, 10);
    *out_delay_ns = strtoul(argv[2], &pEnd2, 10);
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

double sum_2d(double** values, size_t N_x, size_t N_y)
{
    double sum = 0.0;
    for (size_t i = 0; i < N_x; ++i)
    {
        for (size_t j = 0; j < N_y; ++j)
        {
            sum += values[i][j];
        }
    }
    return sum;
}

double mean_2d(double** values, size_t N_x, size_t N_y)
{
    return sum_2d(values, N_x, N_y) / (N_x * N_y);
}

double stdev_2d(double** values, size_t N_x, size_t N_y)
{
    double average = mean_2d(values, N_x, N_y);
    double acc = 0.0;
    for (size_t i = 0; i < N_x; ++i)
    {
        for (size_t j = 0; j < N_y; ++j)
        {
            acc += (values[i][j] - average) * (values[i][j] - average);
        }
    }

    acc /= N_x * N_y;
    return sqrt(acc);
}

readings_t** create_readings_2d(size_t N_x, size_t N_y)
{
    readings_t** result = (readings_t**)malloc(sizeof(readings_t*) * N_x);
    ASSERT_NOT_NULL(result);
    for (size_t i = 0; i < N_x; ++i)
    {
        result[i] = create_readings(N_y);
    }
    return result;
}

readings_t* create_readings(size_t N)
{
    readings_t* result = (readings_t*)malloc(sizeof(readings_t));
    ASSERT_NOT_NULL(result);
    result->cycles = calloc(N, sizeof(double));
    ASSERT_NOT_NULL(result->cycles);
    result->nano_seconds = calloc(N, sizeof(double));
    ASSERT_NOT_NULL(result->nano_seconds);
    result->index = 0;
    return result;
}

void start_readings(readings_t* readings)
{
    readings->cycles[readings->index] = (double)PAPI_get_real_cyc();
    readings->nano_seconds[readings->index] = (double)PAPI_get_real_nsec();
}

// TODO: Add some functionality to remove delay from readings.
void delta_readings(readings_t* readings, size_t num_of_iterations)
{
    assert(num_of_iterations != 0);
    const size_t index = readings->index;
    double* cycles = readings->cycles;
    double* nano_seconds = readings->nano_seconds;
    cycles[index] = (PAPI_get_real_cyc() - cycles[index]) / num_of_iterations;
    nano_seconds[index] = (PAPI_get_real_nsec() - nano_seconds[index]) / num_of_iterations;
    readings->index++;
}

void adjust_readings_for_delay(readings_t* readings, delay_t* delay)
{
    readings->cycles[readings->index - 1] -= delay->num_of_nops;
    readings->nano_seconds[readings->index - 1] -= delay->delay_ns;
}

void destroy_readings(readings_t** readings)
{
    free((*readings)->cycles);
    free((*readings)->nano_seconds);
    free(*readings);
}

readings_t* aggregate_readings(readings_t* readings, size_t N)
{
    readings_t* result = create_readings(N);
    double* cycles = (double*)malloc(sizeof(double) * N);
    double* nano_seconds = (double*)malloc(sizeof(double) * N);

    for (size_t i = 0; i < N; ++i)
    {
        cycles[i] = readings->cycles[i];
        nano_seconds[i] = readings->nano_seconds[i];
    }

    result->cycles[0] = mean(cycles, N);
    result->cycles[1] = stdev(cycles, N);
    result->nano_seconds[0] = mean(nano_seconds, N);
    result->nano_seconds[1] = stdev(nano_seconds, N);
    free(cycles);
    free(nano_seconds);
    return result;
}

readings_t* aggregate_readings_2d(readings_t** readings, size_t N_x, size_t N_y)
{
    readings_t* result = create_readings(2);
    double** cycles = (double**)malloc(sizeof(double*) * N_x);
    double** nano_seconds = (double**)malloc(sizeof(double*) * N_x);
    for (size_t i = 0; i < N_x; ++i)
    {
        cycles[i] = readings[i]->cycles;
        nano_seconds[i] = readings[i]->nano_seconds;
    }

    result->cycles[0] = mean_2d(cycles, N_x, N_y);
    result->cycles[1] = stdev_2d(cycles, N_x, N_y);
    result->nano_seconds[0] = mean_2d(nano_seconds, N_x, N_y);
    result->nano_seconds[1] = stdev_2d(nano_seconds, N_x, N_y);
    return result;
}

size_t count_enqueues_from_probabilities(double* probabilities, double p, size_t iterations)
{
    size_t acc = 0;
    for (size_t i = 0; i < iterations; ++i)
    {
        acc += probabilities[i] < p;
    }
    return acc;
}

void display_readings(readings_t* aggregated_readings)
{
    //const double mean_cycles = aggregated_readings->cycles[0];
    //const double stdev_cycles = aggregated_readings->cycles[1];
    const double mean_nano_seconds = aggregated_readings->nano_seconds[0];
    const double stdev_nano_seconds = aggregated_readings->nano_seconds[1];

    printf("%f,", mean_nano_seconds);
    printf("%f,", (mean_nano_seconds * TEST_ITERATIONS) / 1e9);
    printf("%f,", stdev_nano_seconds);
    printf("%f", stdev_nano_seconds / mean_nano_seconds);
}