#ifndef TESTS_H
#define TESTS_H

#include <stdbool.h>
#include <stddef.h>

/// A test that spawns multiple threads that
/// will increment a single value inside of a shared
/// struct.
/// @param n: Number of threads.
/// @returns: True if passed.
/// False if failed.
bool counter_test(int n);

/// Tests that threads will spin lock
/// when a mutex is already locked.
/// @param n: Number of threads.
/// @returns: True if passed.
/// False if failed.
// bool spinlock_test(size_t n);

#endif