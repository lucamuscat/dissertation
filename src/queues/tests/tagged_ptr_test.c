#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include "../../tagged_ptr.h"

#define TAG_VALUE 1
#define DUMMY_VALUE 123456
#define UINT15_MAX_EXPECTED 32767

const int dummy_value = DUMMY_VALUE;
const int* dummy_value_ptr = &dummy_value;

void test_tagged_ptr(const int* dummy_ptr, uint16_t tag)
{
    uintptr_t tagged_ptr = pack_ptr((void*)dummy_ptr, tag);
    int* extracted_ptr = (int*)extract_ptr(tagged_ptr);
    uint16_t extracted_tag = extract_tag(tagged_ptr);
    assert(dummy_ptr == extracted_ptr);
    assert(tag == extracted_tag);
}

void given_tag_greater_than_uint15_mask_when_extract_flagged_tag_then_wrap_tag()
{
    uintptr_t tagged_ptr = pack_ptr_with_flag((void*)dummy_value_ptr, UINT16_MAX, false);
    uint16_t extracted_tag = extract_flagged_tag(tagged_ptr);
    uint16_t expected_tag = UINT16_MAX % UINT15_MAX_EXPECTED;
    assert(extracted_tag == expected_tag);
    assert(extract_ptr(tagged_ptr) == dummy_value_ptr);
}

void given_true_flag_when_extract_flag_then_return_true_flag()
{
    bool expected_flag = true;
    uintptr_t tagged_ptr = pack_ptr_with_flag((void*)dummy_value_ptr, 0, expected_flag);
    bool extracted_flag = extract_flag(tagged_ptr);
    assert(extracted_flag == expected_flag);
}

int main()
{
    uint16_t one = 1;
    uint16_t max = UINT16_MAX;

    test_tagged_ptr(dummy_value_ptr, one);
    test_tagged_ptr(dummy_value_ptr, max);

    given_tag_greater_than_uint15_mask_when_extract_flagged_tag_then_wrap_tag();
    given_true_flag_when_extract_flag_then_return_true_flag();

    return 0;
}
