#ifndef AFFINITY_UTILS_H
#define AFFINITY_UTILS_H

#include <pthread.h>

#define PHYSICAL_CORES 8

pthread_attr_t create_thread_affinity_attr(size_t index)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET((index + 1) % PHYSICAL_CORES, &cpuset);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
    return attr;
}

#endif