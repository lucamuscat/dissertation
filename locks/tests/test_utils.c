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

// Adapted from http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
int64_t diff(timespec* start,timespec* end)
{
    timespec temp;
    if ((end->tv_nsec - start->tv_nsec) < 0)
    {
        temp.tv_sec = end->tv_sec - start->tv_sec - 1;
        temp.tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
    }
    else
    {
        temp.tv_sec = end->tv_sec - start->tv_sec;
        temp.tv_nsec = end->tv_nsec - start->tv_nsec;
    }
    return temp.tv_nsec;
}