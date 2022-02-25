#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "./test_utils.h"

int handle_args(int argc, char* argv[])
{
    if (argc != 2)
    {
        puts("Missing args");
        return -1;
    }

    char* pEnd;
    const long int number_of_threads = strtol(argv[1], &pEnd, 10);
    omp_set_num_threads(number_of_threads);
    return number_of_threads;
}