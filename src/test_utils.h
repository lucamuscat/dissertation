#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#define CPU_GHZ 2.6

/**

 * @brief Processes the number of threads provided in the command line argument.
 * 
 * @param argc Number of arguments provided by the user
 * @param argv Array of arguments provided by the user
 * @return Number of threads specified in the command line arguments.
 */
int handle_args(int argc, char* argv[]);

/**
 * @brief Generate a random number 
 * 
 * @param min The minimum value that can be generated
 * @param max The maximum value that can be generated
 * @return int Randomly generated integer.
 */
int bounded_random(int min, int max);

int* generate_random_numbers(int n, int min, int max);

void _delay(size_t ns);

// TODO: Make usage of ints or size_t's more consistent.
void handle_queue_args(int argc, char** argv, int* out_num_of_thread, int* delay_ns);

/**
 * @brief Get the number of iterations each thread has to run.
 * 
 * @param num_of_threads The total number of threads that will be used in the test.
 * @param thread_num A zero-based id for the thread being evaluated.
 * @param total_iterations Total number of iterations in the test.
 * @return int Number of iterations for the thread.
 */
int iterations_per_thread(int num_of_threads, int thread_num, int total_iterations);

double mean(double* values, size_t N);

double stdev(double* values, size_t N);

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
    exit(EXIT_FAILURE); \
}

// https://github.com/chaoran/fast-wait-free-queue/blob/d41ec16e5169c864e5fdbe05e1988358bd335fa0/align.h#L10
#define CACHE_LINE_SIZE 64
#define CACHE_ALIGNED __attribute__((aligned(CACHE_LINE_SIZE)))

/**
 * @brief Delay for NS nano seconds. For some reason, multiplying NS*CPU_GHZ
 * inside of _delay causes the delay to last at least two times longer.
 */
#define DELAY(NS) _delay(NS*CPU_GHZ);

#endif