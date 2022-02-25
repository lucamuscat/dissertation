#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include "increment_counter_utils.h"
#include "../../lock.h"
#include "../../globals.h"
#include "../../../test_utils.h"

#ifndef TEST_ITERATIONS
#define TEST_ITERATIONS 1000
#endif

int main(int argc, char* argv[])
{
    int number_of_threads = handle_args(argc, argv);
    if (number_of_threads == -1)
        return -1;
    for (size_t i = 0; i < TEST_ITERATIONS; ++i)
    {
        DEBUG_LOG_F("\n============\nIteration: %ld\n============\n", i);
        counter_test(number_of_threads);
    }
    return 0;
}