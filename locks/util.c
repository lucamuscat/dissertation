#include "util.h"
#include "filter_lock.h"

#include <stdlib.h>

mutex* create_mutex_object(size_t number_of_threads)
{
    mutex* mutex_object = malloc(sizeof(mutex_object));
    mutex_object->counter = 0;
    create_lock(number_of_threads, &mutex_object->lock);
    return mutex_object;
}

thread_data** create_thread_data(mutex* mutex, size_t number_of_threads)
{
    thread_data** temp = (thread_data**)malloc(sizeof(thread_data*) * number_of_threads);

    for (size_t i = 0; i < number_of_threads; ++i)
    {
        temp[i] = (thread_data*)malloc(sizeof(thread_data));
        temp[i]->obj = mutex;
        temp[i]->thread_id = i;
    }
    return temp;
}

void free_thread_data(thread_data** thread_data, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        free(thread_data[i]);
    }
    free(thread_data);
}