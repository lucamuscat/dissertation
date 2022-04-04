#ifndef TAGGED_PTR_H
#define TAGGED_PTR_H

#include <stdint.h>

// Followed using:
// https ://www.boost.org/doc/libs/1_78_0/boost/lockfree/detail/tagged_ptr_ptrcompression.hpp

#define TAG_INDEX 3

// (1L<<48L)-1, zeros out the 16 most significant bits.
#define PTR_MASK 0xffffffffffffUL
#define TAG_BITS 16
#define PTR_BITS 48

typedef uintptr_t tagged_ptr_t;

inline void* extract_ptr(volatile tagged_ptr_t i)
{
    return (void*)(i & PTR_MASK);
}

inline tagged_ptr_t extract_tag(volatile tagged_ptr_t i)
{
    // Shift the by 48 to move the 16 MSBs to the LSBs
    return (uint16_t)(i >> PTR_BITS);
}

inline tagged_ptr_t pack_ptr(volatile void* ptr, uint16_t tag)
{
    return ((uintptr_t)ptr) | ((uint64_t)tag << PTR_BITS);
}

#endif