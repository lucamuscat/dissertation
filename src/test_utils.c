#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "test_utils.h"

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