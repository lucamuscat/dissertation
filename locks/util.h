#ifndef UTIL_H
#define UTIL_H

#include "filter_lock.h"

typedef struct mutex
{
    size_t counter;
    filter_lock_t* lock;
} mutex;

typedef struct thread_data
{
    mutex* obj;
    size_t thread_id;
} thread_data;

mutex* create_mutex_object(size_t);
thread_data** create_thread_data(mutex*, size_t);

#endif