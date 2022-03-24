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

#define CPU_GHZ 2.6

// TODO: Measure the standard deviation of readings at different test iterations
// and arrange the time taken of the test to remain constant, irrespective
// of the delay.
int main(int argc, char** argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Missing args, \n\targ1 - Delay in nano seconds");
        fprintf(stderr, "\n\targ2 - Number of iterations");
        fprintf(stderr, "\n\targ3 - Number of reruns\n");
        exit(EXIT_FAILURE);
    }

    char* pEnd[3];
    const size_t delay_ns = strtoul(argv[1], &pEnd[0], 10);
    const size_t TEST_ITERATIONS = strtoul(argv[2], &pEnd[1], 10);
    const size_t RERUNS = strtoul(argv[3], &pEnd[2], 10);

    PAPI_library_init(PAPI_VER_CURRENT);
    delay_t delay = { 0, 0 };

    puts("Calibrating Delay...");
    calibrate_delay(&delay, delay_ns);

    puts("Starting test");

    readings_t* readings;
    create_readings(&readings, 1);

    start_readings(readings);
    for (size_t j = 0; j < RERUNS; ++j)
    {
        for (size_t i = 0; i < TEST_ITERATIONS; ++i)
        {
            DELAY_OPS(delay.num_of_nops);
        }
    }
    delta_readings(readings, TEST_ITERATIONS*RERUNS);
    
    //double mean_ns = aggregated_readings->nano_seconds[0];
    //double stdev_ns = aggregated_readings->nano_seconds[1];
    //double mean_cycles = aggregated_readings->cycles[0];
    //double stdev_cycles = aggregated_readings->cycles[1];
    printf("Iterations: %zu\n", TEST_ITERATIONS);
    printf("Reruns: %zu\n", RERUNS);
    printf("Average Cycles: %f\n", readings->cycles[0]);
    printf("Average Delay Time: %fns\n", readings->nano_seconds[0]);
    printf("Total Time: %fs\n", readings->nano_seconds[0] * (TEST_ITERATIONS * RERUNS) / 1e9);
    // printf("Stdev: +-%f (%f%%)\n", stdev_ns, (stdev_ns / mean_ns) * 100);
    printf("Expected Delay Time: %ldns\n", delay_ns);
    return 0;
}