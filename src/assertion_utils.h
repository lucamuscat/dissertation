#ifndef ASSERTION_UTILS_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Exit if x is NULL. This macro can be used to ensure that mallocs and
 * callocs succeed.
 */
#define ASSERT_NOT_NULL(x) if(x == NULL) {\
    fprintf(stderr, "%s, %d", __FILE__, __LINE__); \
    perror("Error:"); \
    exit(EXIT_FAILURE); \
}

/// Return x if it is false.
#define PASS(x) if(!x) {\
    return x; \
}

#define ASSERT_TRUE(x, message) if(!(x)) {\
    fprintf(stderr, "%s (File: %s Line: %d)", message, __FILE__, __LINE__); \
    exit(EXIT_FAILURE); \
}

#define ASSERT_SUCCESS(x) { \
int temp = (x);\
if ((temp) != 0) \
{ \
    \
    perror(""); \
    fprintf(stderr, "Error (%d) %s (File: %s Line: %d)\n", temp, #x, __FILE__, __LINE__); \
    exit(EXIT_FAILURE); \
}}
#endif