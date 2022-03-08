#ifndef TEST_UTILS_H
#define TEST_UTILS_H

/**

 * @brief Processes the number of threads provided in the command line argument.
 * 
 * @param argc Number of arguments provided by the user
 * @param argv Array of arguments provided by the user
 * @return Number of threads specified in the command line arguments.
 */
int handle_args(int argc, char* argv[]);


/**
 * @brief Exit if x is NULL. This macro can be used to ensure that mallocs and
 * callocs succeed.
 */
#define P_PASS(x) if(x == NULL) {\
    perror("Error:"); \
    exit(EXIT_FAILURE); \
}

/// Return x if it is false.
#define PASS(x) if(!x) {\
    return x; \
}

#define PASS_LOG(x, message) if(!x) {\
    printf(message); \
}

// https://github.com/chaoran/fast-wait-free-queue/blob/d41ec16e5169c864e5fdbe05e1988358bd335fa0/align.h#L10
#define CACHE_LINE_SIZE 64
#define CACHE_ALIGNED __attribute__((aligned(CACHE_LINE_SIZE)))

#endif