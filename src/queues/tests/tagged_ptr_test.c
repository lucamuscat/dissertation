#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include "../../tagged_ptr.h"

#define LOG(expr) { expr; printf("Test %s passed\n", #expr); }

#define TAG_WORD_MAX(tag_word_size) ((((tag_word)1) << ((tag_word_size) - 1)) | \
 ((((tag_word)1) << ((tag_word_size) - 1)) - 1))

tag_word dummy_value = 1;
tag_word* dummy_value_ptr = &dummy_value;

void when_extract_ptr_then_return_ptr()
{
    tagged_ptr_t flagged_tagged_ptr = pack_ptr_with_flag(dummy_value_ptr, 0, true);
    tagged_ptr_t tagged_ptr = pack_ptr(dummy_value_ptr, 0);

    assert(dummy_value_ptr == extract_ptr(flagged_tagged_ptr));
    assert(dummy_value_ptr == extract_ptr(tagged_ptr));
}

void test_tagged_ptr(const int* dummy_ptr, uint16_t tag)
{
    tagged_ptr_t tagged_ptr = pack_ptr((void*)dummy_ptr, tag);
    int* extracted_ptr = (int*)extract_ptr(tagged_ptr);
    tag_word extracted_tag = extract_tag(tagged_ptr);
    assert(dummy_ptr == extracted_ptr);
    assert(tag == extracted_tag);
}

void given_tag_greater_than_max_tag_word_when_extract_flagged_tag_then_wrap_tag()
{
    tagged_ptr_t tagged_ptr = pack_ptr_with_flag((void*)dummy_value_ptr, (uint64_t)TAG_WORD_MAX(TAG_SIZE), false);
    tag_word extracted_tag = extract_flagged_tag(tagged_ptr);
    tag_word expected_tag = TAG_WORD_MAX(TAG_SIZE) % TAG_WORD_MAX(TAG_SIZE-1);
    assert(extracted_tag == expected_tag);
    assert(extract_ptr(tagged_ptr) == dummy_value_ptr);
}

void given_true_flag_when_extract_flag_then_return_true_flag()
{
    bool expected_flag = true;
    tagged_ptr_t tagged_ptr = pack_ptr_with_flag((void*)dummy_value_ptr, 0, expected_flag);
    bool extracted_flag = extract_flag(tagged_ptr);
    assert(extracted_flag == expected_flag);
}

void given_tagged_ptr_equal_when_equals_then_return_true()
{
    tagged_ptr_t foo = pack_ptr(NULL, 0);
    tagged_ptr_t bar = pack_ptr(NULL, 0);
    assert(equals(foo, bar) == true);
}

void given_tagged_ptr_not_equal_when_equals_then_return_false()
{
    tagged_ptr_t foo = pack_ptr(NULL, 0);
    tagged_ptr_t bar = pack_ptr(NULL, 1);
    assert(equals(foo, bar) == false);
}

void given_flagged_tag_ptr_equal_when_equals_then_return_true()
{
    tagged_ptr_t foo = pack_ptr_with_flag(NULL, 0, false);
    tagged_ptr_t bar = pack_ptr_with_flag(NULL, 0, false);
    assert(equals(foo, bar) == true);
}

int main()
{
    LOG(when_extract_ptr_then_return_ptr());

    LOG(given_tag_greater_than_max_tag_word_when_extract_flagged_tag_then_wrap_tag());
    LOG(given_true_flag_when_extract_flag_then_return_true_flag());
    LOG(given_tagged_ptr_equal_when_equals_then_return_true());
    LOG(given_tagged_ptr_not_equal_when_equals_then_return_false());
    LOG(given_flagged_tag_ptr_equal_when_equals_then_return_true());

    return 0;
}
