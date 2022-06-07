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

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Missing args, \n\targ1 uint - Delay in nano seconds\n");
        exit(EXIT_FAILURE);
    }

    char* pEnd[1];
    const size_t delay_ns = strtoul(argv[1], &pEnd[0], 10);

    PAPI_library_init(PAPI_VER_CURRENT);
    delay_t delay = { 0, 0 };

    puts("Calibrating Delay...");
    calibrate_delay(&delay, delay_ns);

    puts("Starting test");

    readings_t* readings = create_readings(TEST_RERUNS);

    for (size_t j = 0; j < TEST_RERUNS; ++j)
    {
        start_readings(readings);
        for (size_t i = 0; i < TEST_ITERATIONS; ++i)
        {
            DELAY_OPS(delay.num_of_nops);
        }
        delta_readings(readings, TEST_ITERATIONS);
    }
    
    double mean_ns = readings->nano_seconds[0];
    double stdev_ns = readings->nano_seconds[1];
    //double mean_cycles = aggregated_readings->cycles[0];
    //double stdev_cycles = aggregated_readings->cycles[1];
    printf("Iterations: %lld\n", TEST_ITERATIONS);
    printf("Reruns: %d\n", TEST_RERUNS);
    //printf("Average Cycles: %f\n", readings->cycles[0]);
    printf("Average Delay Time: %fns\n", mean_ns);
    printf(
        "Delay Time Coefficient of Variance: %f%%\n",
        coefficient_of_variance(mean_ns, stdev_ns));
    printf("Expected Delay Time: %ldns\n", delay_ns);
    
    double error = fabs(delay_ns - mean_ns);
    printf("Error: %f (%f%%)\n", error, (error / delay_ns) * 100);
    printf("Initial Number of nops: %zu\n", (size_t)(delay_ns * CPU_GHZ));
    printf("Number of nops: %zu\n", delay.num_of_nops);
    printf("Total Time: %fs\n", mean_ns * (TEST_ITERATIONS * TEST_RERUNS) / 1e9);
    // printf("Stdev: +-%f (%f%%)\n", stdev_ns, (stdev_ns / mean_ns) * 100);
    return 0;
}