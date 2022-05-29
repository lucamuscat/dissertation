#ifndef TAGGED_PTR_H
#define TAGGED_PTR_H

#include <stdint.h>

// Followed using:
// https ://www.boost.org/doc/libs/1_78_0/boost/lockfree/detail/tagged_ptr_ptrcompression.hpp
// (1L<<48L)-1, zeros out the 16 most significant bits.
#define LINEAR_ADDRESS_MASK 0xffffffffffffUL
#define TAG_INDEX 15
#define TAG_SIZE 16
#define LINEAR_ADDRESS_SIZE 48
#define UINT15_T_MASK 0x7fff

typedef uintptr_t tagged_ptr_t;

#define extract_ptr(ptr) ((void*)(((uintptr_t)ptr) & LINEAR_ADDRESS_MASK))
#define extract_tag(ptr) ((uint16_t)(((uintptr_t)ptr) >> LINEAR_ADDRESS_SIZE))
#define pack_ptr(ptr, tag) (((uintptr_t)ptr) | (((uint64_t)tag) << LINEAR_ADDRESS_SIZE))

// tag % UINT15_T_MASK makes sure that a 16 bit unsigned int's overflow/wrap
// around behaviour is the same as that of a 15 bit unsigned int

/**
 * @brief Create a tagged_ptr_t with a boolean flag and a 15 bit counter
 * 
 */
#define pack_ptr_with_flag(ptr, tag, flag) (((uintptr_t)ptr) | (((uint64_t)(((tag) % (UINT15_T_MASK)) | (((uint16_t)flag) << TAG_INDEX))) << LINEAR_ADDRESS_SIZE))

/**
 * @brief Extract the flag from a tagged_ptr with a flag.
 * Assumes that the tagged_ptr was created using pack_ptr_with_flag
 */
#define extract_flag(ptr) ((bool)(((uintptr_t)ptr) >> 63))

/**
 * @brief Extract a tag from a tagged_ptr that was created using pack_ptr_with_flag
 * 
 */
#define extract_flagged_tag(i) ((uint16_t)((((uintptr_t)i) >> LINEAR_ADDRESS_SIZE) & UINT15_T_MASK))

#endif