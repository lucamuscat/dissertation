#ifndef TAGGED_PTR_H
#define TAGGED_PTR_H

#include <stdint.h>
#include "alignment_utils.h"

#define TAG_WORD_MAX(tag_word_size) ((((tag_word)1) << ((tag_word_size) - 1)) | \
 ((((tag_word)1) << ((tag_word_size) - 1)) - 1))

#define get_next_ptr(_ptr) (((node_t*)extract_ptr((_ptr)))->next)

#ifndef DWCAS // Used tagged pointers

// Followed using:
// https ://www.boost.org/doc/libs/1_78_0/boost/lockfree/detail/tagged_ptr_ptrcompression.hpp
// (1L<<48L)-1, zeros out the 16 most significant bits.
#define LINEAR_ADDRESS_MASK 0xffffffffffffUL
#define TAG_SIZE 16
#define TAG_INDEX (TAG_SIZE - 1)
#define LINEAR_ADDRESS_SIZE 48
#define UINT15_T_MASK ((1<<15) - 1)

typedef uint16_t tag_word;
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
#define pack_ptr_with_flag(ptr, tag, flag) (((uintptr_t)ptr) | \
    (((uint64_t)(((tag) % (UINT15_T_MASK)) | \
    (((uint16_t)flag) << TAG_INDEX))) << LINEAR_ADDRESS_SIZE))

/**
 * @brief Extract the flag from a tagged_ptr with a flag.
 * Assumes that the tagged_ptr was created using pack_ptr_with_flag
 */
#define extract_flag(ptr) ((bool)(((uintptr_t)ptr) >> 63))

/**
 * @brief Extract a tag from a tagged_ptr that was created using pack_ptr_with_flag
 * 
 */
#define extract_flagged_tag(i) ((uint16_t)((((uintptr_t)i) >> LINEAR_ADDRESS_SIZE)\
 & UINT15_T_MASK))

#define equals(a, b) ((a) == (b))

#else // DWCAS is defined
#define TAG_SIZE 64
typedef struct tagged_ptr_t
{
    void* ptr;
    union
    {
        uint64_t tag;
        struct
        {
            uint64_t flag : 1, flagged_tag : 63;
        };
    };
} DWCAS_ALIGNED tagged_ptr_t;

typedef uint64_t tag_word;

#define pack_ptr(_ptr, _tag) ((tagged_ptr_t){.ptr = (_ptr), .tag = ((uint64_t)(_tag))})
#define extract_ptr(_ptr) (((tagged_ptr_t)(_ptr)).ptr)
#define extract_tag(_ptr) (((tagged_ptr_t)(_ptr)).tag)

#define equals(a, b) ({\
    tagged_ptr_t temp1 = (tagged_ptr_t)a; \
    tagged_ptr_t temp2 = (tagged_ptr_t)b; \
    ((temp1.ptr == temp2.ptr) && (temp1.tag == temp2.tag)); \
})

#define pack_ptr_with_flag(_ptr, _tag, _flag) ((tagged_ptr_t){ \
    .ptr = (_ptr), \
    .flagged_tag = ((_tag)%(TAG_WORD_MAX(TAG_SIZE-1))), \
    .flag=((bool)(_flag))})
#define extract_flag(_ptr) ((bool)((tagged_ptr_t)(_ptr)).flag)
#define extract_flagged_tag(_ptr) (((tagged_ptr_t)(_ptr)).flagged_tag)

#endif
#endif