#ifndef ALIGNMENT_UTILS_H
#define ALIGNMENT_UTILS_H


// https://github.com/chaoran/fast-wait-free-queue/blob/d41ec16e5169c864e5fdbe05e1988358bd335fa0/align.h#L10
#define CACHE_LINE_SIZE 64
#define DOUBLE_CACHE_LINE_SIZE 128
#define DWCAS_ALIGNMENT 16
#define CACHE_ALIGNED __attribute__((aligned(CACHE_LINE_SIZE)))
#define PAD_TO_CACHELINE(type_of_previous_field) (CACHE_LINE_SIZE - sizeof(type_of_previous_field))

/**
 * https://stackoverflow.com/q/29199779 - [1]
 * Intel Optimization Manual Order Number: 248966-045 Feb 2022 - [2]
 * §E.2.5.4 Data Prefetching
 * "Spatial Prefetcher: This prefetcher strives to complete every cache line fetched to the L2 cache with
 * the pair line that completes it to a 128 - byte aligned chunk."
 */
#define DOUBLE_CACHE_ALIGNED __attribute__((aligned(DOUBLE_CACHE_LINE_SIZE)))
#define DWCAS_ALIGNED __attribute__((aligned(DWCAS_ALIGNMENT)))



#endif