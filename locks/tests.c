#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "tests.h"
#include "util.h"
#include "filter_lock.h"
#include "globals.h"


#define ITERATIONS 10
#define INCREMENT_VALUE 10

// Each function call will increase the counter by 100
void* increment_counter(thread_data* data)
{
    for (int i = 0; i < ITERATIONS; ++i)
    {
        wait_lock_stm(data->thread_id, data->obj->lock);
        // CS
        data->obj->counter += INCREMENT_VALUE;
        DEBUG_LOG_F("\nThread Id: %ld", data->thread_id);
        DEBUG_LOG_F("\nCounter: %ld", data->obj->counter);

        unlock_stm(data->thread_id, data->obj->lock);
    }
    return NULL;
}

bool counter_test(size_t n)
{
    mutex* mutex = create_mutex_object(n);
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t*) * n);
    thread_data** thread_parameters = create_thread_data(mutex, n);

    for (size_t i = 0; i < n; ++i)
    {
        DEBUG_LOG_F("\n[I] Created thread %ld\n", i);
        pthread_create(&threads[i], NULL, (void*)increment_counter, thread_parameters[i]);
    }
    for (size_t i = 0; i < n; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    DEBUG_LOG_F("\n%ld\n", mutex->counter);
    assert(mutex->counter == (ITERATIONS*INCREMENT_VALUE*n));

    free_lock(mutex->lock);
    free(mutex);
    free(threads);
    free_thread_data(thread_parameters, n);
    return true;
}