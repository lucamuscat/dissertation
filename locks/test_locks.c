#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "util.h"
#include "filter_lock.h"

// Each function call will increase the counter by 100
void* increment_counter(thread_data* data)
{
    for (int i = 0; i < 10; ++i)
    {
        wait_lock(data->thread_id, data->obj->lock);
        // CS
        data->obj->counter += 10;
        unlock(data->thread_id, data->obj->lock);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        return -1;

    char* pEnd;
    const long int number_of_threads = strtol(argv[1], &pEnd, 10);

    mutex* mutex = create_mutex_object(number_of_threads);
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t*) * number_of_threads);
    thread_data** thread_parameters = create_thread_data(mutex, number_of_threads);
    
    for (long int i = 0; i < number_of_threads; ++i)
    {
        pthread_create(&threads[i], NULL, (void*)increment_counter, thread_parameters[i]);
    }
    for (long int i = 0; i < number_of_threads; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    
    printf("%ld\n", mutex->counter);
    return 0;
}