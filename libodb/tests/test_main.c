#include "odb_engine.h"
#include "odb_errors.h"

#include <stdio.h>
#include <string.h>

extern long gethandlesize(Handle h);

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

static const char *fixture_path(void) {
#ifdef ODB_TEST_FIXTURE_DIR
	static char path[512];
	snprintf(path, sizeof(path), "%s/new.root", ODB_TEST_FIXTURE_DIR);
	return path;
#else
	return "databases/new.root";
#endif
}

static void test_open_fixture(void) {
	odbRef odb = NULL;
	const char *fixture = fixture_path();
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for new.root");
	if (ok) {
		TEST_ASSERT(odbCloseFile(odb) == 1, "close should succeed");
	}
}

static void odb_string_from_c(odbString out, const char *msg) {
	if (!out) {
		return;
	}
	if (!msg) {
		out[0] = 0;
		return;
	}
	size_t len = strlen(msg);
	if (len > 255) {
		len = 255;
	}
	out[0] = (unsigned char)len;
	if (len > 0) {
		memcpy(&out[1], msg, len);
	}
}

static int handle_to_c_string(Handle h, char *out, size_t out_size) {
	if (!out || out_size == 0) {
		return 0;
	}
	out[0] = '\0';
	if (!h || !*h) {
		return 0;
	}
	long size = gethandlesize(h);
	if (size <= 0) {
		return 0;
	}
	unsigned char *data = *h;
	if (size >= 4) {
		uint32_t be = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | (uint32_t)data[3];
		uint32_t le = ((uint32_t)data[3] << 24) | ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[0];
		uint32_t len = 0;
		if (be <= (uint32_t)(size - 4)) {
			len = be;
		} else if (le <= (uint32_t)(size - 4)) {
			len = le;
		}
		if (len > 0) {
			if (len >= out_size) {
				len = (uint32_t)(out_size - 1);
			}
			memcpy(out, data + 4, len);
			out[len] = '\0';
			return 1;
		}
	}
	if (data[0] <= (unsigned char)(size - 1)) {
		unsigned char len = data[0];
		if ((size_t)len >= out_size) {
			len = (unsigned char)(out_size - 1);
		}
		memcpy(out, data + 1, len);
		out[len] = '\0';
		return 1;
	}
	if ((size_t)size >= out_size) {
		size = (long)out_size - 1;
	}
	memcpy(out, data, (size_t)size);
	out[size] = '\0';
	return 1;
}

static void test_read_workspace_values(void) {
	odbRef odb = NULL;
	const char *fixture = fixture_path();
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for new.root");
	if (!ok) {
		return;
	}

	odbString bspath;
	odbString bsname;
	OSType type = 0;
	odbValueRecord value;
	char buffer[256];
	long count = 0;

	odb_string_from_c(bspath, "root");
	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed for root");
	TEST_ASSERT(count == 1, "root should contain 1 item");
	if (ok && count >= 1) {
		ok = odbGetNthItem(odb, bspath, 1, bsname);
		TEST_ASSERT(ok == 1, "odbGetNthItem should succeed for root item 1");
		if (ok) {
			TEST_ASSERT(bsname[0] == 9 && memcmp(&bsname[1], "workspace", 9) == 0,
				"root item 1 should be workspace");
		}
	}

	odb_string_from_c(bspath, "root.workspace");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for root.workspace");
	TEST_ASSERT(type == (OSType)'tabl', "root.workspace should be a table");
	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed for root.workspace");
	TEST_ASSERT(count == 3, "root.workspace should contain 3 items");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for root.workspace");
	if (ok) {
		TEST_ASSERT(value.valuetype == tablevaluetype, "workspace should return tablevaluetype");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.firstName");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for firstName");
	TEST_ASSERT(type == (OSType)'TEXT', "firstName should be TEXT");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for firstName");
	if (ok && handle_to_c_string(value.data.stringvalue, buffer, sizeof(buffer))) {
		TEST_ASSERT(strcmp(buffer, "Ted") == 0, "firstName should be Ted");
	}
	if (ok) {
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.lastName");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for lastName");
	TEST_ASSERT(type == (OSType)'TEXT', "lastName should be TEXT");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for lastName");
	if (ok && handle_to_c_string(value.data.stringvalue, buffer, sizeof(buffer))) {
		TEST_ASSERT(strcmp(buffer, "Howard") == 0, "lastName should be Howard");
	}
	if (ok) {
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.age");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for age");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for age");
	if (ok) {
		if (type == (OSType)'long') {
			TEST_ASSERT(value.data.longvalue == 43, "age should be 43 (long)");
		} else if (type == (OSType)'shor') {
			TEST_ASSERT(value.data.intvalue == 43, "age should be 43 (short)");
		} else if (type == (OSType)'doub') {
			double d = (value.data.doublevalue && *value.data.doublevalue) ? **value.data.doublevalue : 0.0;
			TEST_ASSERT((int)d == 43, "age should be 43 (double)");
		} else {
			TEST_ASSERT(0, "age should be a numeric type");
		}
		odbDisposeValue(odb, &value);
	}
	odbCloseFile(odb);
}

int main(void) {
	test_open_missing_file();
	test_open_fixture();
	test_read_workspace_values();
	if (tests_failed) {
		printf("%d tests failed\n", tests_failed);
		return 1;
	}
	printf("all tests passed\n");
	return 0;
}
