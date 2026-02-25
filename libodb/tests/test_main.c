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

static void test_open_fixture(void) {
	odbRef odb = NULL;
#ifdef ODB_TEST_FIXTURE_DIR
	char path[512];
	snprintf(path, sizeof(path), "%s/Sample.root", ODB_TEST_FIXTURE_DIR);
	const char *fixture = path;
#else
	const char *fixture = "databases/Sample.root";
#endif
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for Sample.root");
	if (ok) {
		TEST_ASSERT(odbCloseFile(odb) == 1, "close should succeed");
	}
}

int main(void) {
	test_open_missing_file();
	test_open_fixture();
	if (tests_failed) {
		printf("%d tests failed\n", tests_failed);
		return 1;
	}
	printf("all tests passed\n");
	return 0;
}
