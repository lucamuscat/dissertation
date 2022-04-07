#ifndef TAGGED_PTR_H
#define TAGGED_PTR_H

#include <stdint.h>

// Followed using:
// https ://www.boost.org/doc/libs/1_78_0/boost/lockfree/detail/tagged_ptr_ptrcompression.hpp

#define TAG_INDEX 3

// (1L<<48L)-1, zeros out the 16 most significant bits.
#define PTR_MASK 0xffffffffffffUL
#define TAG_BITS 16
#define TAG_POS 15
#define PTR_BITS 48
#define UINT15T_MASK 0x7fff

typedef uintptr_t tagged_ptr_t;

#define extract_ptr(ptr) ((void*)((ptr) & PTR_MASK))
#define extract_tag(ptr) ((uint16_t)((ptr) >> PTR_BITS))
#define pack_ptr(ptr, tag) (((uintptr_t)ptr) | (((uint64_t)tag) << PTR_BITS))

// tag % UINT15T_MASK makes sure that a 16 bit unsigned int's overflow/wrap
// around behaviour is the same as that of a 15 bit unsigned int

/**
 * @brief Create a tagged_ptr_t with a boolean flag and a 15 bit counter
 * 
 */
#define pack_ptr_with_flag(ptr, tag, flag) (((uintptr_t)ptr) | (((uint64_t)(((tag) % (UINT15T_MASK)) | (((uint16_t)flag) << TAG_POS))) << PTR_BITS))

/**
 * @brief Extract the flag from a tagged_ptr with a flag.
 * Assumes that the tagged_ptr was created using pack_ptr_with_flag
 */
#define extract_flag(ptr) ((bool)((ptr) >> 63))

/**
 * @brief Extract a tag from a tagged_ptr that was created using pack_ptr_with_flag
 * 
 */
#define extract_flagged_tag(i) ((uint16_t)(((i) >> PTR_BITS) & UINT15T_MASK))

#endif