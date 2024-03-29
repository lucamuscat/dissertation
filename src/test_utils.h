#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "alignment_utils.h"

#define CPU_GHZ 2.6
// 10^7 test iterations
#define TEST_ITERATIONS 10000000LL
// 10^6 warm up iterations.
#define WARMUP_ITERATIONS 1000000LL

/**
 * @brief Size of the queue before running the p_enqueue_dequeue benchmark.
 * This is necessary, as long runs of dequeues on an empty queue will mostly
 * trigger the empty dequeue path of each queue.
 */
#define PREFILL_SIZE 1000
#define TEST_RERUNS 10
#define NANO_TO_MINUTE(x) ((double)x/(6e10))
#define LIKELY(x) __builtin_expect((x), 1)

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

// TODO: Make usage of ints or size_t's more consistent.
void handle_queue_args(int argc, char** argv, size_t* out_num_of_thread, size_t* delay_ns);

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

double mean_2d(double** values, size_t N_x, size_t N_y);
double stdev_2d(double** values, size_t N_x, size_t N_y);

/**
 * @brief An abstraction for taking readings from PAPI
 * 
 */
typedef struct readings_t
{
    double* cycles;
    double* nano_seconds;
    size_t index;
} readings_t;

typedef struct delay_t
{
    size_t delay_ns;
    size_t num_of_nops;
} delay_t;

void _delay(size_t ns);
void calibrate_delay(delay_t* out_delay, size_t expected_delay_ns);

/**
 * @brief Create and initialize a readings_t object.
 * @param in_out_readings: Pointer to readings object to be initialized.
 * @param N: Number of readings that will be taken.
 */
readings_t* create_readings(size_t N);

readings_t** create_readings_2d(size_t N_x, size_t N_y);

/**
 * @brief Sets the cycles and nano_seconds fields of readings_t to the result
 * of PAPI_get_real_cyc() and PAPI_get_real_nsec(). Not thread safe.
 *
 * @param readings: A readings_t object that has already been initialized. The contents
 * of the object will be overwritten with the new readings.
 */
void start_readings(readings_t* readings);

/**
 * @brief Calculate the elapsed time of a reading. Not thread safe
 * @param readings: A readings_t object that has already been initalized. The contents
 * of the object will be updated with the elapsed readings.
 * @param num_of_iterations: Number of iterations that were recorded. The total
 * time will be divided by this value to get the average time per iteration.
 */
void delta_readings(readings_t* readings, size_t num_of_iterations);

void adjust_readings_for_delay(readings_t* readings, delay_t* delay);

/**
 * @brief Calculate the mean and the standard deviation of multiple readings_t
 * objects.
 *
 * @param readings A 2d array of size N_x by N_y; typically, N_x represents the
 * number of threads, and N_y represents the number of reruns.
 * @param N_x Number of columns in readings; this is typically the number of threads
 * used when taking readings.
 * @param N_y Number of rows in the readings; this is typically the number of reruns.
 * @return readings_t* Returns a reading_t where the first element of each field
 * represents the mean, and the second element represents the standard deviation.
 *
 * Eg. result->cycles[0] = Mean Cycles, result->cycles[1] = Standard Deviation of mean cycles.
 */
readings_t* aggregate_readings_2d(readings_t** readings, size_t N_x, size_t N_y);

readings_t* aggregate_readings(readings_t*, size_t N);

/**
 * @brief Display the mean and the standard deviation of the readings obtained.
 * Readings must be aggregated before being displayed. 
 * @param readings Aggregated readings to be displayed.
 */
void display_readings(readings_t* aggregated_readings);

/**
 * @brief Free resources taken up by a readings_t object.
 * 
 * @param readings readings_t object to be freed.
 */
void destroy_readings(readings_t** readings);

size_t count_enqueues_from_probabilities(double* probabilities, double p, size_t iterations);

/**
 * @brief Delay for NS nano seconds. For some reason, multiplying NS*CPU_GHZ
 * inside of _delay causes the delay to last at least two times longer.
 */
#define DELAY(NS) _delay(NS*CPU_GHZ);

#define DELAY_OPS(OPS) _delay(OPS);

#define coefficient_of_variance(_mean, _std) (_std/_mean)*100

#endif