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

// TODO: Measure the standard deviation of readings at different test iterations
// and arrange the time taken of the test to remain constant, irrespective
// of the delay.
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
    delay_t delay = { 0, 0 };

    puts("Calibrating Delay...");
    calibrate_delay(&delay, delay_ns);

    puts("Starting test");

    readings_t* readings;
    create_readings(&readings, RERUNS);

    for (size_t j = 0; j < RERUNS; ++j)
    {
        start_readings(readings);
        for (size_t i = 0; i < TEST_ITERATIONS; ++i)
        {
            DELAY_OPS(delay.num_of_nops);
        }
        delta_readings(readings, TEST_ITERATIONS);
    }

    readings_t* aggregated_readings = aggregate_readings(readings, RERUNS);
    double mean_ns = aggregated_readings->nano_seconds[0];
    double stdev_ns = aggregated_readings->nano_seconds[1];
    double mean_cycles = aggregated_readings->cycles[0];
    //double stdev_cycles = aggregated_readings->cycles[1];

    printf("Average Cycles: %f\n", mean_cycles);
    printf("Average Delay Time: %fns\n", mean_ns);
    printf("Stdev: +-%f (%f%%)\n", stdev_ns, (stdev_ns / mean_ns) * 100);
    printf("Expected Delay Time: %ldns\n", delay_ns);
    return 0;
}