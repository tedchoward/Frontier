#include "odb_engine.h"
#include "odb_errors.h"
#include "odb_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

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

static Handle make_pascal_string_handle(const char *text) {
	if (!text) {
		return NULL;
	}
	size_t len = strlen(text);
	if (len > 255) {
		len = 255;
	}
	bigstring bs;
	bs[0] = (unsigned char)len;
	if (len > 0) {
		memcpy(&bs[1], text, len);
	}
	hdlstring hstring = NULL;
	if (!newheapstring(bs, &hstring)) {
		return NULL;
	}
	return (Handle)hstring;
}

static int copy_file(const char *src, const char *dst) {
	FILE *in = fopen(src, "rb");
	if (!in) {
		return 0;
	}
	FILE *out = fopen(dst, "wb");
	if (!out) {
		fclose(in);
		return 0;
	}
	unsigned char buffer[4096];
	size_t read_bytes = 0;
	while ((read_bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
		if (fwrite(buffer, 1, read_bytes, out) != read_bytes) {
			fclose(in);
			fclose(out);
			return 0;
		}
	}
	fclose(in);
	fclose(out);
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
	int32_t count = 0;

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

	odb_string_from_c(bspath, "Root.Workspace.FirstName");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "case-insensitive path should resolve");
	if (ok && handle_to_c_string(value.data.stringvalue, buffer, sizeof(buffer))) {
		TEST_ASSERT(strcmp(buffer, "Ted") == 0, "case-insensitive firstName should be Ted");
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

static void test_write_values(void) {
	odbRef odb = NULL;
	const char *fixture = fixture_path();
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for new.root");
	if (!ok) {
		return;
	}

	odbString bspath;
	odbValueRecord value;
	char buffer[256];

	odb_string_from_c(bspath, "root.workspace.firstName");
	value.valuetype = stringvaluetype;
	value.data.stringvalue = make_pascal_string_handle("Ada");
	TEST_ASSERT(value.data.stringvalue != NULL, "string handle should be created");
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for firstName");
	if (value.data.stringvalue) {
		disposehandle(value.data.stringvalue);
	}
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for updated firstName");
	if (ok && handle_to_c_string(value.data.stringvalue, buffer, sizeof(buffer))) {
		TEST_ASSERT(strcmp(buffer, "Ada") == 0, "firstName should be Ada");
	}
	if (ok) {
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.age");
	value.valuetype = longvaluetype;
	value.data.longvalue = 44;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for age");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for updated age");
	if (ok) {
		TEST_ASSERT(value.data.longvalue == 44, "age should be 44");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.active");
	value.valuetype = booleanvaluetype;
	value.data.flvalue = 1;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for active");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for active");
	if (ok) {
		TEST_ASSERT(value.data.flvalue == 1, "active should be true");
		odbDisposeValue(odb, &value);
	}

	odbCloseFile(odb);
}

static void test_save_persists_values(void) {
	const char *fixture = fixture_path();
	char path[512];
	snprintf(path, sizeof(path), "/tmp/odb_save_test.root");
	TEST_ASSERT(copy_file(fixture, path) == 1, "fixture copy should succeed");

	odbRef odb = NULL;
	odbBool ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for save test db");
	if (!ok) {
		return;
	}

	odbString bspath;
	odbValueRecord value;

	odb_string_from_c(bspath, "root.workspace.firstName");
	value.valuetype = stringvaluetype;
	value.data.stringvalue = make_pascal_string_handle("Ada");
	TEST_ASSERT(value.data.stringvalue != NULL, "string handle should be created");
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for firstName");
	if (value.data.stringvalue) {
		disposehandle(value.data.stringvalue);
	}

	odb_string_from_c(bspath, "root.workspace.age");
	value.valuetype = longvaluetype;
	value.data.longvalue = 44;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for age");

	ok = odbSaveFile(odb);
	TEST_ASSERT(ok == 1, "odbSaveFile should succeed");
	odbCloseFile(odb);

	odb = NULL;
	ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "reopen should succeed after save");
	if (!ok) {
		return;
	}

	char buffer[256];
	odb_string_from_c(bspath, "root.workspace.firstName");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for saved firstName");
	if (ok && handle_to_c_string(value.data.stringvalue, buffer, sizeof(buffer))) {
		TEST_ASSERT(strcmp(buffer, "Ada") == 0, "firstName should persist as Ada");
	}
	if (ok) {
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.age");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for saved age");
	if (ok) {
		TEST_ASSERT(value.data.longvalue == 44, "age should persist as 44");
		odbDisposeValue(odb, &value);
	}

	odbCloseFile(odb);
}

static void test_delete_persists(void) {
	const char *fixture = fixture_path();
	char path[512];
	snprintf(path, sizeof(path), "/tmp/odb_delete_test.root");
	TEST_ASSERT(copy_file(fixture, path) == 1, "fixture copy should succeed");

	odbRef odb = NULL;
	odbBool ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for delete test db");
	if (!ok) {
		return;
	}

	odbString bspath;
	int32_t count = 0;

	odb_string_from_c(bspath, "root.workspace");
	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed before delete");
	TEST_ASSERT(count == 3, "workspace should start with 3 items");

	odb_string_from_c(bspath, "root.workspace.lastName");
	ok = odbDelete(odb, bspath);
	TEST_ASSERT(ok == 1, "odbDelete should succeed for lastName");

	odb_string_from_c(bspath, "root.workspace");
	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed after delete");
	TEST_ASSERT(count == 2, "workspace should have 2 items after delete");

	ok = odbSaveFile(odb);
	TEST_ASSERT(ok == 1, "odbSaveFile should succeed after delete");
	odbCloseFile(odb);

	odb = NULL;
	ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "reopen should succeed after delete save");
	if (!ok) {
		return;
	}
	odb_string_from_c(bspath, "root.workspace");
	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed after reopen");
	TEST_ASSERT(count == 2, "workspace should persist with 2 items");
	odbCloseFile(odb);
}

static void test_new_table_persists(void) {
	const char *fixture = fixture_path();
	char path[512];
	snprintf(path, sizeof(path), "/tmp/odb_newtable_test.root");
	TEST_ASSERT(copy_file(fixture, path) == 1, "fixture copy should succeed");

	odbRef odb = NULL;
	odbBool ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for newtable test db");
	if (!ok) {
		return;
	}

	odbString bspath;
	odbValueRecord value;
	int32_t count = 0;

	odb_string_from_c(bspath, "root.newtable");
	ok = odbNewTable(odb, bspath);
	TEST_ASSERT(ok == 1, "odbNewTable should succeed for root.newtable");

	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for new table");
	if (ok) {
		TEST_ASSERT(value.valuetype == tablevaluetype, "new table should return tablevaluetype");
		odbDisposeValue(odb, &value);
	}

	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed for new table");
	TEST_ASSERT(count == 0, "new table should be empty");

	ok = odbSaveFile(odb);
	TEST_ASSERT(ok == 1, "odbSaveFile should succeed after new table");
	odbCloseFile(odb);

	odb = NULL;
	ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "reopen should succeed after new table save");
	if (!ok) {
		return;
	}

	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed after reopen");
	if (ok) {
		TEST_ASSERT(value.valuetype == tablevaluetype, "new table should persist as tablevaluetype");
		odbDisposeValue(odb, &value);
	}

	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed after reopen");
	TEST_ASSERT(count == 0, "new table should remain empty");
	odbCloseFile(odb);
}

static void test_new_file_creates_db(void) {
	const char *path = "/tmp/odb_newfile_test.root";
	odbBool ok = odbNewPath(path);
	TEST_ASSERT(ok == 1, "odbNewPath should succeed");
	if (!ok) {
		return;
	}

	odbRef odb = NULL;
	ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for new file");
	if (!ok) {
		return;
	}

	odbString bspath;
	OSType type = 0;
	int32_t count = 0;

	odb_string_from_c(bspath, "root");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for root on new db");
	TEST_ASSERT(type == (OSType)'tabl', "root should be a table on new db");

	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed for root on new db");
	TEST_ASSERT(count == 0, "new db root should be empty");

	odbCloseFile(odb);
}

static void test_defined_paths(void) {
	odbRef odb = NULL;
	const char *fixture = fixture_path();
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for new.root");
	if (!ok) {
		return;
	}

	odbString bspath;
	odb_string_from_c(bspath, "root");
	ok = odbDefined(odb, bspath);
	TEST_ASSERT(ok == 1, "root should be defined");

	odb_string_from_c(bspath, "root.workspace");
	ok = odbDefined(odb, bspath);
	TEST_ASSERT(ok == 1, "workspace should be defined");

	odb_string_from_c(bspath, "root.workspace.missing");
	ok = odbDefined(odb, bspath);
	TEST_ASSERT(ok == 0, "missing path should not be defined");

	odbCloseFile(odb);
}

static void test_read_errors(void) {
	odbRef odb = NULL;
	const char *fixture = fixture_path();
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for new.root");
	if (!ok) {
		return;
	}

	odbString bspath;
	OSType type = 0;
	int32_t count = 0;
	odbString bsname;

	odb_string_from_c(bspath, "root.workspace.missing");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 0, "odbGetType should fail for missing path");
	TEST_ASSERT(odbGetLastErrorCode() == ODB_ERR_NOT_FOUND, "missing path should set NOT_FOUND");

	odb_string_from_c(bspath, "root.workspace.firstName");
	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 0, "odbCountItems should fail for non-table path");
	TEST_ASSERT(odbGetLastErrorCode() != ODB_ERR_NONE, "non-table count should set error");

	odb_string_from_c(bspath, "root.workspace");
	ok = odbGetNthItem(odb, bspath, 0, bsname);
	TEST_ASSERT(ok == 0, "odbGetNthItem should fail for n=0");
	TEST_ASSERT(odbGetLastErrorCode() != ODB_ERR_NONE, "n=0 should set error");
	ok = odbGetNthItem(odb, bspath, 4, bsname);
	TEST_ASSERT(ok == 0, "odbGetNthItem should fail for out of range");
	TEST_ASSERT(odbGetLastErrorCode() != ODB_ERR_NONE, "out of range should set error");

	odbCloseFile(odb);
}

int main(void) {
	test_open_missing_file();
	test_open_fixture();
	test_read_workspace_values();
	test_write_values();
	test_save_persists_values();
	test_delete_persists();
	test_new_table_persists();
	test_new_file_creates_db();
	test_defined_paths();
	test_read_errors();
	if (tests_failed) {
		printf("%d tests failed\n", tests_failed);
		return 1;
	}
	printf("all tests passed\n");
	return 0;
}
