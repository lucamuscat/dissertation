#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include "../../tagged_ptr.h"

#define TAG_VALUE 1
#define DUMMY_VALUE 123456

void test_tagged_ptr(int* dummy_ptr, uint16_t tag)
{
    uintptr_t tagged_ptr = pack_ptr((void*)dummy_ptr, tag);
    int* extracted_ptr = (int*)extract_ptr(tagged_ptr);
    uint16_t extracted_tag = extract_tag(tagged_ptr);
    assert(dummy_ptr == extracted_ptr);
    assert(tag == extracted_tag);
}

int main()
{
    int dummy_value = DUMMY_VALUE;
    int* dummy_value_ptr = &dummy_value;

    uint16_t one = 1;
    uint16_t max = UINT16_MAX;

    test_tagged_ptr(dummy_value_ptr, one);
    test_tagged_ptr(dummy_value_ptr, max);

    return 0;
}
