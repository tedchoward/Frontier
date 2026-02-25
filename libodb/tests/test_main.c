#include "odb_engine.h"
#include "odb_errors.h"

#include <stdio.h>

static int tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
	do { \
		if (!(cond)) { \
			printf("FAIL: %s\n", msg); \
			tests_failed++; \
		} \
	} while (0)

static void test_open_missing_file(void) {
	odbRef odb = NULL;
	odbBool ok = odbOpenPath("/no/such/file.root", &odb);
	TEST_ASSERT(ok == 0, "open should fail for missing file");
	TEST_ASSERT(odbGetLastErrorCode() != ODB_ERR_NONE, "error code should be set");
}

int main(void) {
	test_open_missing_file();
	if (tests_failed) {
		printf("%d tests failed\n", tests_failed);
		return 1;
	}
	printf("all tests passed\n");
	return 0;
}
