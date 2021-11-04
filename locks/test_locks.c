#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "tests.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
        return -1;

    char* pEnd;
    const long int number_of_threads = strtol(argv[1], &pEnd, 10);

    counter_test(number_of_threads);

    return 0;
}