#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <time.h>

typedef struct timespec timespec;

int64_t diff(timespec* start, timespec* end);
int handle_args(int argc, char* argv[]);
#endif