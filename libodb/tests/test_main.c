#include "odb_engine.h"
#include "odb_errors.h"
#include "odb_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

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

static Handle make_handle_from_bytes(const void *data, size_t size) {
	if (!data || size == 0) {
		return NULL;
	}
	Handle h = NULL;
	if (!newhandle((long)size, &h)) {
		return NULL;
	}
	memcpy(*h, data, size);
	return h;
}

static int handle_equals_bytes(Handle h, const void *data, size_t size) {
	if (!h || !*h || !data) {
		return 0;
	}
	long hsize = gethandlesize(h);
	if (hsize != (long)size) {
		return 0;
	}
	return memcmp(*h, data, size) == 0;
}

static int handle_ends_with_bytes(Handle h, const void *suffix, size_t suffix_size) {
	if (!h || !*h || !suffix) {
		return 0;
	}
	long hsize = gethandlesize(h);
	if (hsize < (long)suffix_size) {
		return 0;
	}
	return memcmp(((unsigned char *)*h) + (hsize - (long)suffix_size), suffix, suffix_size) == 0;
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

static void test_sleep_ms(unsigned int ms) {
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
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
	TEST_ASSERT(type == tablevaluetype, "root.workspace should be a table");
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
	TEST_ASSERT(type == stringvaluetype, "firstName should be TEXT");
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
	TEST_ASSERT(type == stringvaluetype, "lastName should be TEXT");
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
		if (type == longvaluetype) {
			TEST_ASSERT(value.data.longvalue == 43, "age should be 43 (long)");
		} else if (type == shortvaluetype) {
			TEST_ASSERT(value.data.intvalue == 43, "age should be 43 (short)");
		} else if (type == doublevaluetype) {
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

	odb_string_from_c(bspath, "root.workspace.middleInitial");
	value.valuetype = charvaluetype;
	value.data.chvalue = 'Q';
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for middleInitial");
	ok = odbGetType(odb, bspath, &value.data.ostypevalue);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for middleInitial");
	TEST_ASSERT(value.data.ostypevalue == charvaluetype, "middleInitial should be char");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for middleInitial");
	if (ok) {
		TEST_ASSERT(value.data.chvalue == 'Q', "middleInitial should be Q");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.shoeSize");
	value.valuetype = shortvaluetype;
	value.data.intvalue = 9;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for shoeSize");
	ok = odbGetType(odb, bspath, &value.data.ostypevalue);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for shoeSize");
	TEST_ASSERT(value.data.ostypevalue == shortvaluetype, "shoeSize should be short");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for shoeSize");
	if (ok) {
		TEST_ASSERT(value.data.intvalue == 9, "shoeSize should be 9");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.updatedAt");
	value.valuetype = datevaluetype;
	value.data.datevalue = 0x12345678U;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for updatedAt");
	ok = odbGetType(odb, bspath, &value.data.ostypevalue);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for updatedAt");
	TEST_ASSERT(value.data.ostypevalue == datevaluetype, "updatedAt should be date");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for updatedAt");
	if (ok) {
		TEST_ASSERT(value.data.datevalue == 0x12345678U, "updatedAt should round-trip");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.sortOrder");
	value.valuetype = directionvaluetype;
	value.data.dirvalue = dir_sorted;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for sortOrder");
	ok = odbGetType(odb, bspath, &value.data.ostypevalue);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for sortOrder");
	TEST_ASSERT(value.data.ostypevalue == directionvaluetype, "sortOrder should be direction");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for sortOrder");
	if (ok) {
		TEST_ASSERT(value.data.dirvalue == dir_sorted, "sortOrder should be dir_sorted");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.rating");
	value.valuetype = singlevaluetype;
	value.data.singlevalue = 4.5f;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for rating");
	ok = odbGetType(odb, bspath, &value.data.ostypevalue);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for rating");
	TEST_ASSERT(value.data.ostypevalue == singlevaluetype, "rating should be single");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for rating");
	if (ok) {
		TEST_ASSERT(value.data.singlevalue == 4.5f, "rating should be 4.5");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.fileType");
	value.valuetype = string4valuetype;
	value.data.ostypevalue = ODB_FOURCC('T', 'E', 'S', 'T');
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for fileType");
	ok = odbGetType(odb, bspath, &value.data.ostypevalue);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for fileType");
	TEST_ASSERT(value.data.ostypevalue == string4valuetype, "fileType should be string4");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for fileType");
	if (ok) {
		TEST_ASSERT(value.data.ostypevalue == ODB_FOURCC('T', 'E', 'S', 'T'), "fileType should round-trip");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.status");
	value.valuetype = enumeratorvaluetype;
	value.data.enumvalue = ODB_FOURCC('D', 'O', 'N', 'E');
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for status");
	ok = odbGetType(odb, bspath, &value.data.ostypevalue);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for status");
	TEST_ASSERT(value.data.ostypevalue == enumeratorvaluetype, "status should be enum");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for status");
	if (ok) {
		TEST_ASSERT(value.data.enumvalue == ODB_FOURCC('D', 'O', 'N', 'E'), "status should round-trip");
		odbDisposeValue(odb, &value);
	}

	odbCloseFile(odb);
}

static void test_write_additional_scalar_struct_values(void) {
	odbRef odb = NULL;
	const char *fixture = fixture_path();
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for additional write tests");
	if (!ok) {
		return;
	}

	odbString bspath;
	odbValueRecord value;
	OSType type = 0;

	odb_string_from_c(bspath, "root.workspace.sessionToken");
	value.valuetype = tokenvaluetype;
	value.data.tokenvalue = 1234;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support tokenvaluetype");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for token");
	TEST_ASSERT(type == tokenvaluetype, "sessionToken should be token type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for token");
	if (ok) {
		TEST_ASSERT(value.data.tokenvalue == 1234, "token should round-trip");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.origin");
	value.valuetype = pointvaluetype;
	value.data.pointvalue.v = 12;
	value.data.pointvalue.h = 34;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support pointvaluetype");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for point");
	TEST_ASSERT(type == pointvaluetype, "origin should be point type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for point");
	if (ok) {
		TEST_ASSERT(value.data.pointvalue.v == 12 && value.data.pointvalue.h == 34, "point should round-trip");
		odbDisposeValue(odb, &value);
	}

	odbRect rect = {1, 2, 3, 4};
	Handle hrect = make_handle_from_bytes(&rect, sizeof(rect));
	TEST_ASSERT(hrect != NULL, "rect handle should be created");
	odb_string_from_c(bspath, "root.workspace.bounds");
	value.valuetype = rectvaluetype;
	value.data.rectvalue = (odbRect **)hrect;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support rectvaluetype");
	disposehandle(hrect);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for rect");
	TEST_ASSERT(type == rectvaluetype, "bounds should be rect type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for rect");
	if (ok) {
		TEST_ASSERT(value.data.rectvalue && *value.data.rectvalue, "rect handle should be non-null");
		if (value.data.rectvalue && *value.data.rectvalue) {
			odbRect *r = *value.data.rectvalue;
			TEST_ASSERT(r->top == 1 && r->left == 2 && r->bottom == 3 && r->right == 4, "rect should round-trip");
		}
		odbDisposeValue(odb, &value);
	}

	odbPattern pattern = {{0x11, 0x22, 0x33, 0x44, 0xAA, 0xBB, 0xCC, 0xDD}};
	Handle hpattern = make_handle_from_bytes(&pattern, sizeof(pattern));
	TEST_ASSERT(hpattern != NULL, "pattern handle should be created");
	odb_string_from_c(bspath, "root.workspace.fillPattern");
	value.valuetype = patternvaluetype;
	value.data.patternvalue = (odbPattern **)hpattern;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support patternvaluetype");
	disposehandle(hpattern);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for pattern");
	TEST_ASSERT(type == patternvaluetype, "fillPattern should be pattern type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for pattern");
	if (ok) {
		TEST_ASSERT(value.data.patternvalue && *value.data.patternvalue, "pattern handle should be non-null");
		if (value.data.patternvalue && *value.data.patternvalue) {
			unsigned char *bytes = (unsigned char *)*value.data.patternvalue;
			TEST_ASSERT(bytes[0] == 0x11 && bytes[7] == 0xDD, "pattern should round-trip");
		}
		odbDisposeValue(odb, &value);
	}

	odbRGBColor color = {0x1111, 0x2222, 0x3333};
	Handle hrgb = make_handle_from_bytes(&color, sizeof(color));
	TEST_ASSERT(hrgb != NULL, "rgb handle should be created");
	odb_string_from_c(bspath, "root.workspace.themeColor");
	value.valuetype = rgbvaluetype;
	value.data.rgbvalue = (odbRGBColor **)hrgb;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support rgbvaluetype");
	disposehandle(hrgb);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for rgb");
	TEST_ASSERT(type == rgbvaluetype, "themeColor should be rgb type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for rgb");
	if (ok) {
		TEST_ASSERT(value.data.rgbvalue && *value.data.rgbvalue, "rgb handle should be non-null");
		if (value.data.rgbvalue && *value.data.rgbvalue) {
			odbRGBColor *c = *value.data.rgbvalue;
			TEST_ASSERT(c->red == 0x1111 && c->green == 0x2222 && c->blue == 0x3333, "rgb should round-trip");
		}
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.scale");
	value.valuetype = fixedvaluetype;
	value.data.fixedvalue = 0x00018000;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support fixedvaluetype");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for fixed");
	TEST_ASSERT(type == fixedvaluetype, "scale should be fixed type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for fixed");
	if (ok) {
		TEST_ASSERT(value.data.fixedvalue == 0x00018000, "fixed should round-trip");
		odbDisposeValue(odb, &value);
	}

	double input_double = 3.141592653589793;
	Handle hdouble = make_handle_from_bytes(&input_double, sizeof(input_double));
	TEST_ASSERT(hdouble != NULL, "double handle should be created");
	odb_string_from_c(bspath, "root.workspace.precise");
	value.valuetype = doublevaluetype;
	value.data.doublevalue = (double **)hdouble;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support doublevaluetype");
	disposehandle(hdouble);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for double");
	TEST_ASSERT(type == doublevaluetype, "precise should be double type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for double");
	if (ok) {
		double d = (value.data.doublevalue && *value.data.doublevalue) ? **value.data.doublevalue : 0.0;
		TEST_ASSERT(d > 3.1415 && d < 3.1416, "double should round-trip");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.badRect");
	value.valuetype = rectvaluetype;
	value.data.rectvalue = NULL;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 0, "odbSetValue should reject null rect handle");

	unsigned char bad_pattern[7] = {0};
	Handle hbad_pattern = make_handle_from_bytes(bad_pattern, sizeof(bad_pattern));
	TEST_ASSERT(hbad_pattern != NULL, "bad pattern handle should be created");
	odb_string_from_c(bspath, "root.workspace.badPattern");
	value.valuetype = patternvaluetype;
	value.data.patternvalue = (odbPattern **)hbad_pattern;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 0, "odbSetValue should reject bad pattern handle size");
	disposehandle(hbad_pattern);

	unsigned char bad_double[4] = {0};
	Handle hbad_double = make_handle_from_bytes(bad_double, sizeof(bad_double));
	TEST_ASSERT(hbad_double != NULL, "bad double handle should be created");
	odb_string_from_c(bspath, "root.workspace.badDouble");
	value.valuetype = doublevaluetype;
	value.data.doublevalue = (double **)hbad_double;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 0, "odbSetValue should reject bad double handle size");
	disposehandle(hbad_double);

	odbCloseFile(odb);
}

static void test_write_phase3_handle_types(void) {
	odbRef odb = NULL;
	const char *fixture = fixture_path();
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for phase3 write tests");
	if (!ok) {
		return;
	}

	odbString bspath;
	odbValueRecord value;
	OSType type = 0;
	char buffer[256];

	unsigned char binary_bytes[] = {'d', 'a', 't', 'a', 0x00};
	Handle hbinary = make_handle_from_bytes(binary_bytes, sizeof(binary_bytes));
	TEST_ASSERT(hbinary != NULL, "binary handle should be created");
	odb_string_from_c(bspath, "root.workspace.payload");
	value.valuetype = binaryvaluetype;
	value.data.binaryvalue = hbinary;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support binaryvaluetype");
	disposehandle(hbinary);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for binary");
	TEST_ASSERT(type == binaryvaluetype, "payload should be binary type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for binary");
	if (ok) {
		TEST_ASSERT(handle_equals_bytes(value.data.binaryvalue, binary_bytes, sizeof(binary_bytes)), "binary should round-trip");
		odbDisposeValue(odb, &value);
	}

	Handle haddress = make_pascal_string_handle("root.workspace.firstName");
	TEST_ASSERT(haddress != NULL, "address handle should be created");
	odb_string_from_c(bspath, "root.workspace.ref");
	value.valuetype = addressvaluetype;
	value.data.addressvalue = haddress;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support addressvaluetype");
	disposehandle(haddress);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for address");
	TEST_ASSERT(type == addressvaluetype, "ref should be address type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for address");
	if (ok && handle_to_c_string(value.data.addressvalue, buffer, sizeof(buffer))) {
		TEST_ASSERT(strcmp(buffer, "root.workspace.firstName") == 0, "address should round-trip");
	}
	if (ok) {
		odbDisposeValue(odb, &value);
	}

	unsigned char objspec_bytes[] = {0x01, 0x02, 0x10, 0x20, 0x30};
	Handle hobj = make_handle_from_bytes(objspec_bytes, sizeof(objspec_bytes));
	TEST_ASSERT(hobj != NULL, "objspec handle should be created");
	odb_string_from_c(bspath, "root.workspace.objref");
	value.valuetype = objspecvaluetype;
	value.data.objspecvalue = hobj;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support objspecvaluetype");
	disposehandle(hobj);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for objspec");
	TEST_ASSERT(type == objspecvaluetype, "objref should be objspec type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for objspec");
	if (ok) {
		TEST_ASSERT(handle_equals_bytes(value.data.objspecvalue, objspec_bytes, sizeof(objspec_bytes)), "objspec should round-trip");
		odbDisposeValue(odb, &value);
	}

	unsigned char filespec_bytes[] = {0x41, 0x42, 0x43, 0x00, 0x44, 0x45};
	Handle hfile = make_handle_from_bytes(filespec_bytes, sizeof(filespec_bytes));
	TEST_ASSERT(hfile != NULL, "filespec handle should be created");
	odb_string_from_c(bspath, "root.workspace.fileRef");
	value.valuetype = filespecvaluetype;
	value.data.filespecvalue = hfile;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support filespecvaluetype");
	disposehandle(hfile);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for filespec");
	TEST_ASSERT(type == filespecvaluetype, "fileRef should be filespec type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for filespec");
	if (ok) {
		TEST_ASSERT(handle_equals_bytes(value.data.filespecvalue, filespec_bytes, sizeof(filespec_bytes)), "filespec should round-trip");
		odbDisposeValue(odb, &value);
	}

	unsigned char alias_bytes[] = {0xAA, 0xBB, 0xCC, 0xDD};
	Handle halias = make_handle_from_bytes(alias_bytes, sizeof(alias_bytes));
	TEST_ASSERT(halias != NULL, "alias handle should be created");
	odb_string_from_c(bspath, "root.workspace.aliasRef");
	value.valuetype = aliasvaluetype;
	value.data.aliasvalue = halias;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support aliasvaluetype");
	disposehandle(halias);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for alias");
	TEST_ASSERT(type == aliasvaluetype, "aliasRef should be alias type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for alias");
	if (ok) {
		TEST_ASSERT(handle_equals_bytes(value.data.aliasvalue, alias_bytes, sizeof(alias_bytes)), "alias should round-trip");
		odbDisposeValue(odb, &value);
	}

	unsigned char list_bytes[] = {0x11, 0x22, 0x33};
	Handle hlist = make_handle_from_bytes(list_bytes, sizeof(list_bytes));
	TEST_ASSERT(hlist != NULL, "list handle should be created");
	odb_string_from_c(bspath, "root.workspace.listRef");
	value.valuetype = listvaluetype;
	value.data.listvalue = hlist;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support listvaluetype");
	disposehandle(hlist);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for list");
	TEST_ASSERT(type == listvaluetype, "listRef should be list type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for list");
	if (ok) {
		TEST_ASSERT(handle_equals_bytes(value.data.listvalue, list_bytes, sizeof(list_bytes)), "list should round-trip");
		odbDisposeValue(odb, &value);
	}

	unsigned char record_bytes[] = {0x44, 0x55, 0x66, 0x77};
	Handle hrecord = make_handle_from_bytes(record_bytes, sizeof(record_bytes));
	TEST_ASSERT(hrecord != NULL, "record handle should be created");
	odb_string_from_c(bspath, "root.workspace.recordRef");
	value.valuetype = recordvaluetype;
	value.data.recordvalue = hrecord;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support recordvaluetype");
	disposehandle(hrecord);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for record");
	TEST_ASSERT(type == recordvaluetype, "recordRef should be record type");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for record");
	if (ok) {
		TEST_ASSERT(handle_equals_bytes(value.data.recordvalue, record_bytes, sizeof(record_bytes)), "record should round-trip");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.badBinary");
	value.valuetype = binaryvaluetype;
	value.data.binaryvalue = NULL;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 0, "odbSetValue should reject null binary handle");

	odb_string_from_c(bspath, "root.workspace.badAddress");
	value.valuetype = addressvaluetype;
	value.data.addressvalue = NULL;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 0, "odbSetValue should reject null address handle");

	odbCloseFile(odb);
}

static void test_write_phase4_external_types(void) {
	odbRef odb = NULL;
	const char *fixture = fixture_path();
	odbBool ok = odbOpenPath(fixture, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for phase4 write tests");
	if (!ok) {
		return;
	}

	odbString bspath;
	odbValueRecord value;
	OSType type = 0;
	int32_t count = -1;

	unsigned char script_blob[] = {0x10, 0x20, 0x30, 0x40};
	Handle hscript = make_handle_from_bytes(script_blob, sizeof(script_blob));
	TEST_ASSERT(hscript != NULL, "script handle should be created");
	odb_string_from_c(bspath, "root.workspace.scriptBlob");
	value.valuetype = scriptvaluetype;
	value.data.externalvalue = hscript;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support scriptvaluetype");
	disposehandle(hscript);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for script external");
	TEST_ASSERT(type == scriptvaluetype, "scriptBlob should report scriptvaluetype");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for script external");
	if (ok) {
		TEST_ASSERT(value.valuetype == externalvaluetype, "script external should read as externalvaluetype payload");
		TEST_ASSERT(handle_ends_with_bytes(value.data.externalvalue, script_blob, sizeof(script_blob)), "script payload should round-trip");
		odbDisposeValue(odb, &value);
	}

	unsigned char wp_blob[] = {0x41, 0x42, 0x43};
	Handle hwp = make_handle_from_bytes(wp_blob, sizeof(wp_blob));
	TEST_ASSERT(hwp != NULL, "wptext handle should be created");
	odb_string_from_c(bspath, "root.workspace.wpBlob");
	value.valuetype = wptextvaluetype;
	value.data.externalvalue = hwp;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support wptextvaluetype");
	disposehandle(hwp);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for wptext external");
	TEST_ASSERT(type == wptextvaluetype, "wpBlob should report wptextvaluetype");

	unsigned char pic_blob[] = {0x99, 0x88, 0x77};
	Handle hpic = make_handle_from_bytes(pic_blob, sizeof(pic_blob));
	TEST_ASSERT(hpic != NULL, "picture handle should be created");
	odb_string_from_c(bspath, "root.workspace.picBlob");
	value.valuetype = picturevaluetype;
	value.data.externalvalue = hpic;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support picturevaluetype");
	disposehandle(hpic);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for picture external");
	TEST_ASSERT(type == picturevaluetype, "picBlob should report picturevaluetype");

	unsigned char ext_blob[] = {0x01, 0x02, 0x03, 0x04};
	Handle hext = make_handle_from_bytes(ext_blob, sizeof(ext_blob));
	TEST_ASSERT(hext != NULL, "external handle should be created");
	odb_string_from_c(bspath, "root.workspace.extBlob");
	value.valuetype = externalvaluetype;
	value.data.externalvalue = hext;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support externalvaluetype with opaque payload");
	disposehandle(hext);
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for generic external");
	TEST_ASSERT(type == externalvaluetype, "extBlob should report externalvaluetype");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for generic external");
	if (ok) {
		TEST_ASSERT(value.valuetype == externalvaluetype, "generic external should read as externalvaluetype");
		TEST_ASSERT(handle_ends_with_bytes(value.data.externalvalue, ext_blob, sizeof(ext_blob)), "generic external payload should round-trip");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.phase4table");
	value.valuetype = tablevaluetype;
	value.data.externalvalue = NULL;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should support tablevaluetype");
	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for table set via odbSetValue");
	TEST_ASSERT(type == tablevaluetype, "phase4table should report tablevaluetype");
	ok = odbCountItems(odb, bspath, &count);
	TEST_ASSERT(ok == 1, "odbCountItems should succeed for new table from set");
	TEST_ASSERT(count == 0, "new table from set should be empty");

	odb_string_from_c(bspath, "root.workspace.badScript");
	value.valuetype = scriptvaluetype;
	value.data.externalvalue = NULL;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 0, "odbSetValue should reject null script payload");

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

	odbRect rect = {10, 20, 30, 40};
	Handle hrect = make_handle_from_bytes(&rect, sizeof(rect));
	TEST_ASSERT(hrect != NULL, "rect handle should be created for save test");
	odb_string_from_c(bspath, "root.workspace.persistRect");
	value.valuetype = rectvaluetype;
	value.data.rectvalue = (odbRect **)hrect;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for persistRect");
	if (hrect) {
		disposehandle(hrect);
	}

	odbRGBColor rgb = {0x1111, 0x2222, 0x3333};
	Handle hrgb = make_handle_from_bytes(&rgb, sizeof(rgb));
	TEST_ASSERT(hrgb != NULL, "rgb handle should be created for save test");
	odb_string_from_c(bspath, "root.workspace.persistRgb");
	value.valuetype = rgbvaluetype;
	value.data.rgbvalue = (odbRGBColor **)hrgb;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for persistRgb");
	if (hrgb) {
		disposehandle(hrgb);
	}

	double precise = 6.283185307179586;
	Handle hdouble = make_handle_from_bytes(&precise, sizeof(precise));
	TEST_ASSERT(hdouble != NULL, "double handle should be created for save test");
	odb_string_from_c(bspath, "root.workspace.persistDouble");
	value.valuetype = doublevaluetype;
	value.data.doublevalue = (double **)hdouble;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for persistDouble");
	if (hdouble) {
		disposehandle(hdouble);
	}

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

	odb_string_from_c(bspath, "root.workspace.persistRect");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for saved rect");
	if (ok) {
		TEST_ASSERT(value.data.rectvalue && *value.data.rectvalue, "saved rect handle should be non-null");
		if (value.data.rectvalue && *value.data.rectvalue) {
			odbRect *r = *value.data.rectvalue;
			TEST_ASSERT(r->top == 10 && r->left == 20 && r->bottom == 30 && r->right == 40, "saved rect should round-trip");
		}
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.persistRgb");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for saved rgb");
	if (ok) {
		TEST_ASSERT(value.data.rgbvalue && *value.data.rgbvalue, "saved rgb handle should be non-null");
		if (value.data.rgbvalue && *value.data.rgbvalue) {
			odbRGBColor *c = *value.data.rgbvalue;
			TEST_ASSERT(c->red == 0x1111 && c->green == 0x2222 && c->blue == 0x3333, "saved rgb should round-trip");
		}
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.persistDouble");
	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for saved double");
	if (ok) {
		double d = (value.data.doublevalue && *value.data.doublevalue) ? **value.data.doublevalue : 0.0;
		TEST_ASSERT(d > 6.2831 && d < 6.2832, "saved double should round-trip");
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
	TEST_ASSERT(type == tablevaluetype, "root should be a table on new db");

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

static void test_moddate_updates(void) {
	const char *fixture = fixture_path();
	char path[512];
	snprintf(path, sizeof(path), "/tmp/odb_moddate_test.root");
	TEST_ASSERT(copy_file(fixture, path) == 1, "fixture copy should succeed");

	odbRef odb = NULL;
	odbBool ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for moddate test db");
	if (!ok) {
		return;
	}

	odbString bspath;
	uint32_t date1 = 0;
	uint32_t date2 = 0;
	uint32_t date3 = 0;
	odbValueRecord value;

	odb_string_from_c(bspath, "root.workspace");
	ok = odbGetModDate(odb, bspath, &date1);
	TEST_ASSERT(ok == 1, "odbGetModDate should succeed before update");

	test_sleep_ms(1100);

	odb_string_from_c(bspath, "root.workspace.age");
	value.valuetype = longvaluetype;
	value.data.longvalue = 45;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should succeed for moddate update");

	odb_string_from_c(bspath, "root.workspace");
	ok = odbSaveFile(odb);
	TEST_ASSERT(ok == 1, "odbSaveFile should succeed for moddate update");

	ok = odbGetModDate(odb, bspath, &date2);
	TEST_ASSERT(ok == 1, "odbGetModDate should succeed after update");
	TEST_ASSERT(date2 != date1, "moddate should change after update");

	odbCloseFile(odb);

	odb = NULL;
	ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "reopen should succeed for moddate test db");
	if (!ok) {
		return;
	}
	ok = odbGetModDate(odb, bspath, &date3);
	TEST_ASSERT(ok == 1, "odbGetModDate should succeed after reopen");
	TEST_ASSERT(date3 == date2, "moddate should persist after reopen");

	odbCloseFile(odb);
}

static void test_quoted_path_segments(void) {
	const char *fixture = fixture_path();
	char path[512];
	snprintf(path, sizeof(path), "/tmp/odb_quoted_path_test.root");
	TEST_ASSERT(copy_file(fixture, path) == 1, "fixture copy should succeed");

	odbRef odb = NULL;
	odbBool ok = odbOpenPath(path, &odb);
	TEST_ASSERT(ok == 1, "open should succeed for quoted path test db");
	if (!ok) {
		return;
	}

	odbString bspath;
	odbValueRecord value;
	OSType type = 0;

	odb_string_from_c(bspath, "root.workspace.[\"Sample Outline 1\"]");
	ok = odbNewTable(odb, bspath);
	TEST_ASSERT(ok == 1, "odbNewTable should accept quoted table name");

	odb_string_from_c(bspath, "root.workspace.[\"Sample Outline 1\"].[\"version.1\"]");
	value.valuetype = longvaluetype;
	value.data.longvalue = 7;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should accept quoted child name with dot");

	ok = odbGetType(odb, bspath, &type);
	TEST_ASSERT(ok == 1, "odbGetType should succeed for quoted child path");
	TEST_ASSERT(type == longvaluetype, "quoted child path should be long");

	ok = odbGetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbGetValue should succeed for quoted child path");
	if (ok) {
		TEST_ASSERT(value.data.longvalue == 7, "quoted child long value should round-trip");
		odbDisposeValue(odb, &value);
	}

	odb_string_from_c(bspath, "root.workspace.[\"He said \\\"hi\\\"\"]");
	value.valuetype = longvaluetype;
	value.data.longvalue = 11;
	ok = odbSetValue(odb, bspath, &value);
	TEST_ASSERT(ok == 1, "odbSetValue should accept escaped quotes in path segment");

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
	test_write_additional_scalar_struct_values();
	test_write_phase3_handle_types();
	test_write_phase4_external_types();
	test_save_persists_values();
	test_delete_persists();
	test_new_table_persists();
	test_new_file_creates_db();
	test_defined_paths();
	test_moddate_updates();
	test_quoted_path_segments();
	test_read_errors();
	if (tests_failed) {
		printf("%d tests failed\n", tests_failed);
		return 1;
	}
	printf("all tests passed\n");
	return 0;
}
