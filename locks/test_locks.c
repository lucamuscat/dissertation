#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "tests.h"
#include "globals.h"

#ifndef TEST_ITERATIONS
#define TEST_ITERATIONS 1000
#endif

int main(int argc, char* argv[])
{
    if (argc != 2)
        return -1;

    char* pEnd;
    const long int number_of_threads = strtol(argv[1], &pEnd, 10);

    for (size_t i = 0; i < TEST_ITERATIONS; ++i)
    {
        DEBUG_LOG_F("\n============\nIteration: %ld\n============\n", i);
        counter_test(number_of_threads);
    }

    return 0;
}