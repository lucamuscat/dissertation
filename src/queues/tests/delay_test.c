/**
 * @file delay_test.c
 * @brief Measures the accuracy of the DELAY macro. Make sure that CPU frequency
 * scaling and turbo boost are disabled on the CPU before running.
 */

#include <papi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "../../test_utils.h"

#define TEST_ITERATIONS 10000000
#define PRERUN 2500
#define RERUNS 3
#define CPU_GHZ 2.6

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

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Missing args, arg1 - Delay in nano seconds\n");
        exit(EXIT_FAILURE);
    }

    char* pEnd;
    const size_t delay_ns = strtoul(argv[1], &pEnd, 10);

    PAPI_library_init(PAPI_VER_CURRENT);

    puts("Warming up...");
    for (size_t i = 0; i < PRERUN; ++i)
    {
        DELAY(delay_ns);
    }

    puts("Starting test");

    double readings[RERUNS];

    for (size_t j = 0; j < RERUNS; ++j)
    {
        long long elapsed = PAPI_get_real_nsec();
        for (size_t i = 0; i < TEST_ITERATIONS; ++i)
        {
            DELAY(delay_ns);
        }
        readings[j] = ((double)PAPI_get_real_nsec() - elapsed)/TEST_ITERATIONS;
    }
    double avg = mean(readings, RERUNS);
    double standard_deviation = stdev(readings, RERUNS);
    //printf("First: %fns\n", readings[0]);
    printf("Average Delay Time: %fns\n", avg);
    printf("Stdev: +-%f (%f%%)\n", standard_deviation, (standard_deviation / avg) * 100);
    printf("Expected Delay Time: %ldns\n", delay_ns);
    return 0;
}