#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "../globals.h"
#include "../filter_lock.h"

void* delay(filter_lock_t* lock)
{
    wait_lock(0, lock);
    DEBUG_LOG("Sleeping for 2 seconds");
    sleep(2);
    unlock(0, lock);
    return NULL;
}

bool spinlock_test()
{
    filter_lock_t* lock;
    create_lock(2, &lock);
    pthread_t thread;
    pthread_create(&thread, NULL, (void*)delay, lock);
    wait_lock(1, lock);
    DEBUG_LOG("Entered CS after 2s");
    unlock(1, lock);
    pthread_join(thread, NULL);
    return true;
}

int main()
{
    spinlock_test();
    return 0;
}