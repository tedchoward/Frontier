#include "odb_compat.h"

#include <stdio.h>

static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
	do { \
		if (!(cond)) { \
			printf("FAIL: %s\n", msg); \
			tests_failed++; \
		} \
	} while (0)

static void test_handle_clear_alloc(void) {
	Handle h = NULL;
	odbBool ok = newclearhandle(88, &h);
	TEST_ASSERT(ok == 1, "newclearhandle should succeed");
	TEST_ASSERT(h != NULL, "handle pointer should be non-null");
	TEST_ASSERT(*h != NULL, "handle data should be non-null");
	unsigned char *data = *h;
	int all_zero = 1;
	for (int i = 0; i < 88; ++i) {
		if (data[i] != 0) {
			all_zero = 0;
			break;
		}
	}
	TEST_ASSERT(all_zero == 1, "handle data should be zeroed");
	data[0] = 0x5A;
	TEST_ASSERT((*h)[0] == 0x5A, "handle data should be writable");
	disposehandle(h);
}

int main(void) {
	test_handle_clear_alloc();
	if (tests_failed) {
		printf("%d tests failed\n", tests_failed);
		return 1;
	}
	printf("all tests passed\n");
	return 0;
}
