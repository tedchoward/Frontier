#include "odb_engine.h"
#include "odb_errors.h"
#include "odb_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <limits.h>

static void print_usage(void) {
	printf("usage:\n");
	printf("  odb_cli <dbpath> list <path>\n");
	printf("  odb_cli <dbpath> get <path>\n");
	printf("  odb_cli <dbpath> set <path> string <value>\n");
	printf("  odb_cli <dbpath> set <path> char <value>\n");
	printf("  odb_cli <dbpath> set <path> short <value>\n");
	printf("  odb_cli <dbpath> set <path> long <value>\n");
	printf("  odb_cli <dbpath> set <path> bool <true|false>\n");
	printf("  odb_cli <dbpath> set <path> date <uint32>\n");
	printf("  odb_cli <dbpath> set <path> direction <name|number>\n");
	printf("  odb_cli <dbpath> set <path> string4 <abcd>\n");
	printf("  odb_cli <dbpath> set <path> single <value>\n");
	printf("  odb_cli <dbpath> set <path> enum <abcd>\n");
	printf("  odb_cli <dbpath> newtable <path>\n");
	printf("  odb_cli <dbpath> newfile\n");
	printf("  odb_cli <dbpath> defined <path>\n");
	printf("  odb_cli <dbpath> moddate <path>\n");
	printf("  odb_cli <dbpath> delete <path>\n");
	printf("  odb_cli <dbpath> save\n");
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

static int cmd_list(odbRef odb, const char *path) {
	odbString bspath;
	odb_string_from_c(bspath, path);
	int32_t count = 0;
	if (!odbCountItems(odb, bspath, &count)) {
		return 1;
	}
	for (int32_t i = 1; i <= count; ++i) {
		odbString name;
		if (!odbGetNthItem(odb, bspath, i, name)) {
			return 1;
		}
		printf("%.*s\n", name[0], name + 1);
	}
	return 0;
}

extern long gethandlesize(Handle h);

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

static void print_ostype(OSType type) {
	unsigned char t[5];
	t[0] = (unsigned char)((type >> 24) & 0xff);
	t[1] = (unsigned char)((type >> 16) & 0xff);
	t[2] = (unsigned char)((type >> 8) & 0xff);
	t[3] = (unsigned char)(type & 0xff);
	t[4] = '\0';
	printf("%s", t);
}

static int parse_fourcc(const char *text, OSType *out) {
	if (!text || !out || strlen(text) != 4) {
		return 0;
	}
	*out = ODB_FOURCC(text[0], text[1], text[2], text[3]);
	return 1;
}

static int parse_direction(const char *text, odbDirection *out) {
	if (!text || !out) {
		return 0;
	}
	if (strcmp(text, "up") == 0) {
		*out = dir_up;
		return 1;
	}
	if (strcmp(text, "down") == 0) {
		*out = dir_down;
		return 1;
	}
	if (strcmp(text, "left") == 0) {
		*out = dir_left;
		return 1;
	}
	if (strcmp(text, "right") == 0) {
		*out = dir_right;
		return 1;
	}
	if (strcmp(text, "flatup") == 0) {
		*out = dir_flatup;
		return 1;
	}
	if (strcmp(text, "flatdown") == 0) {
		*out = dir_flatdown;
		return 1;
	}
	if (strcmp(text, "sorted") == 0) {
		*out = dir_sorted;
		return 1;
	}
	if (strcmp(text, "pageup") == 0) {
		*out = dir_pageup;
		return 1;
	}
	if (strcmp(text, "pagedown") == 0) {
		*out = dir_pagedown;
		return 1;
	}
	if (strcmp(text, "pageleft") == 0) {
		*out = dir_pageleft;
		return 1;
	}
	if (strcmp(text, "pageright") == 0) {
		*out = dir_pageright;
		return 1;
	}
	char *end = NULL;
	long raw = strtol(text, &end, 10);
	if (!end || *end != '\0') {
		return 0;
	}
	*out = (odbDirection)raw;
	return 1;
}

static const char *direction_name(odbDirection dir) {
	switch (dir) {
		case nodirection:
			return "none";
		case dir_up:
			return "up";
		case dir_down:
			return "down";
		case dir_left:
			return "left";
		case dir_right:
			return "right";
		case dir_flatup:
			return "flatup";
		case dir_flatdown:
			return "flatdown";
		case dir_sorted:
			return "sorted";
		case dir_pageup:
			return "pageup";
		case dir_pagedown:
			return "pagedown";
		case dir_pageleft:
			return "pageleft";
		case dir_pageright:
			return "pageright";
		default:
			return NULL;
	}
}

static int cmd_get(odbRef odb, const char *path) {
	odbString bspath;
	odb_string_from_c(bspath, path);
	odbValueRecord value;
	if (!odbGetValue(odb, bspath, &value)) {
		return 1;
	}
	printf("type=");
	print_ostype((OSType)value.valuetype);
	printf(" value=");
	switch (value.valuetype) {
		case stringvaluetype: {
			char buffer[1024];
			if (handle_to_c_string(value.data.stringvalue, buffer, sizeof(buffer))) {
				printf("%s", buffer);
			} else {
				printf("<string>");
			}
			break;
		}
		case longvaluetype:
			printf("%" PRId32, value.data.longvalue);
			break;
		case shortvaluetype:
			printf("%d", (int)value.data.intvalue);
			break;
		case booleanvaluetype:
			printf("%s", value.data.flvalue ? "true" : "false");
			break;
		case charvaluetype:
			printf("%c", value.data.chvalue ? value.data.chvalue : ' ');
			break;
		case datevaluetype:
			printf("%" PRIu32, value.data.datevalue);
			break;
		case directionvaluetype: {
			const char *name = direction_name(value.data.dirvalue);
			if (name) {
				printf("%s", name);
			} else {
				printf("%d", (int)value.data.dirvalue);
			}
			break;
		}
		case string4valuetype:
			print_ostype(value.data.ostypevalue);
			break;
		case singlevaluetype:
			printf("%f", (double)value.data.singlevalue);
			break;
		case enumeratorvaluetype:
			print_ostype(value.data.enumvalue);
			break;
		case doublevaluetype: {
			double d = (value.data.doublevalue && *value.data.doublevalue) ? **value.data.doublevalue : 0.0;
			printf("%f", d);
			break;
		}
		case binaryvaluetype: {
			long size = gethandlesize(value.data.binaryvalue);
			printf("<binary %ld bytes>", size);
			break;
		}
		case externalvaluetype: {
			long size = gethandlesize(value.data.externalvalue);
			printf("<external %ld bytes>", size);
			break;
		}
		case tablevaluetype: {
			int32_t count = 0;
			odbBool ok = odbCountItems(odb, bspath, &count);
			if (ok) {
				printf("<table %" PRId32 " items>", count);
			} else {
				printf("<table>");
			}
			break;
		}
		default:
			printf("<unhandled>");
			break;
	}
	printf("\n");
	odbDisposeValue(odb, &value);
	return 0;
}

static int cmd_set(odbRef odb, int argc, char **argv) {
	if (argc != 6) {
		return 1;
	}
	const char *path = argv[3];
	const char *type = argv[4];
	const char *value = argv[5];

	odbString bspath;
	odb_string_from_c(bspath, path);
	odbValueRecord rec;

	if (strcmp(type, "string") == 0) {
		size_t len = strlen(value);
		if (len > 255) {
			len = 255;
		}
		bigstring bs;
		bs[0] = (unsigned char)len;
		if (len > 0) {
			memcpy(&bs[1], value, len);
		}
		hdlstring hstring = NULL;
		if (!newheapstring(bs, &hstring)) {
			return 1;
		}
		rec.valuetype = stringvaluetype;
		rec.data.stringvalue = (Handle)hstring;
		int ok = odbSetValue(odb, bspath, &rec) ? 0 : 1;
		disposehandle((Handle)hstring);
		return ok;
	}

	if (strcmp(type, "char") == 0) {
		if (!value[0] || value[1] != '\0') {
			return 1;
		}
		rec.valuetype = charvaluetype;
		rec.data.chvalue = (unsigned char)value[0];
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "short") == 0) {
		char *end = NULL;
		long v = strtol(value, &end, 10);
		if (!end || *end != '\0' || v < SHRT_MIN || v > SHRT_MAX) {
			return 1;
		}
		rec.valuetype = shortvaluetype;
		rec.data.intvalue = (int16_t)v;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "long") == 0) {
		char *end = NULL;
		long v = strtol(value, &end, 10);
		if (!end || *end != '\0') {
			return 1;
		}
		rec.valuetype = longvaluetype;
		rec.data.longvalue = (int32_t)v;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "bool") == 0) {
		int v = 0;
		if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
			v = 1;
		} else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
			v = 0;
		} else {
			return 1;
		}
		rec.valuetype = booleanvaluetype;
		rec.data.flvalue = (odbBool)v;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "date") == 0) {
		char *end = NULL;
		unsigned long v = strtoul(value, &end, 10);
		if (!end || *end != '\0' || v > UINT32_MAX) {
			return 1;
		}
		rec.valuetype = datevaluetype;
		rec.data.datevalue = (uint32_t)v;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "direction") == 0) {
		odbDirection dir = nodirection;
		if (!parse_direction(value, &dir)) {
			return 1;
		}
		rec.valuetype = directionvaluetype;
		rec.data.dirvalue = dir;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "string4") == 0) {
		OSType code = 0;
		if (!parse_fourcc(value, &code)) {
			return 1;
		}
		rec.valuetype = string4valuetype;
		rec.data.ostypevalue = code;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "single") == 0) {
		char *end = NULL;
		float v = strtof(value, &end);
		if (!end || *end != '\0') {
			return 1;
		}
		rec.valuetype = singlevaluetype;
		rec.data.singlevalue = v;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "enum") == 0) {
		OSType code = 0;
		if (!parse_fourcc(value, &code)) {
			return 1;
		}
		rec.valuetype = enumeratorvaluetype;
		rec.data.enumvalue = code;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	return 1;
}

static int cmd_newtable(odbRef odb, const char *path) {
	odbString bspath;
	odb_string_from_c(bspath, path);
	return odbNewTable(odb, bspath) ? 0 : 1;
}

static int cmd_delete(odbRef odb, const char *path) {
	odbString bspath;
	odb_string_from_c(bspath, path);
	return odbDelete(odb, bspath) ? 0 : 1;
}

static int cmd_defined(odbRef odb, const char *path) {
	odbString bspath;
	odb_string_from_c(bspath, path);
	odbBool ok = odbDefined(odb, bspath);
	printf("%s\n", ok ? "true" : "false");
	return ok ? 0 : 1;
}

static int cmd_moddate(odbRef odb, const char *path) {
	odbString bspath;
	odb_string_from_c(bspath, path);
	uint32_t date = 0;
	if (!odbGetModDate(odb, bspath, &date)) {
		return 1;
	}
	printf("%" PRIu32 "\n", date);
	return 0;
}

static int cmd_save(odbRef odb) {
	return odbSaveFile(odb) ? 0 : 1;
}

static int cmd_newfile(const char *path) {
	return odbNewPath(path) ? 0 : 1;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		print_usage();
		return 1;
	}
	const char *dbpath = argv[1];
	const char *command = argv[2];
	if (strcmp(command, "newfile") == 0) {
		if (argc != 3) {
			print_usage();
			return 1;
		}
		int rc = cmd_newfile(dbpath);
		if (rc != 0) {
			odbString err;
			odbGetError(err);
			printf("command failed (code=%d): %.*s\n", odbGetLastErrorCode(), err[0], err + 1);
		}
		return rc;
	}
	odbRef odb = NULL;
	if (!odbOpenPath(dbpath, &odb)) {
		odbString err;
		odbGetError(err);
		printf("odbOpenPath failed (code=%d): %.*s\n", odbGetLastErrorCode(), err[0], err + 1);
		return 1;
	}
	int rc = 0;
	if (strcmp(command, "list") == 0) {
		if (argc != 4) {
			rc = 1;
		} else {
			const char *path = argv[3];
			rc = cmd_list(odb, path);
		}
	} else if (strcmp(command, "get") == 0) {
		if (argc != 4) {
			rc = 1;
		} else {
			const char *path = argv[3];
			rc = cmd_get(odb, path);
		}
	} else if (strcmp(command, "set") == 0) {
		rc = cmd_set(odb, argc, argv);
	} else if (strcmp(command, "newtable") == 0) {
		if (argc != 4) {
			rc = 1;
		} else {
			const char *path = argv[3];
			rc = cmd_newtable(odb, path);
		}
	} else if (strcmp(command, "defined") == 0) {
		if (argc != 4) {
			rc = 1;
		} else {
			const char *path = argv[3];
			rc = cmd_defined(odb, path);
		}
	} else if (strcmp(command, "moddate") == 0) {
		if (argc != 4) {
			rc = 1;
		} else {
			const char *path = argv[3];
			rc = cmd_moddate(odb, path);
		}
	} else if (strcmp(command, "delete") == 0) {
		if (argc != 4) {
			rc = 1;
		} else {
			const char *path = argv[3];
			rc = cmd_delete(odb, path);
		}
	} else if (strcmp(command, "save") == 0) {
		if (argc != 3) {
			rc = 1;
		} else {
			rc = cmd_save(odb);
		}
	} else {
		print_usage();
		rc = 1;
	}
	if (rc != 0) {
		print_usage();
	}
	if (rc == 0 && (strcmp(command, "set") == 0 || strcmp(command, "delete") == 0 || strcmp(command, "newtable") == 0)) {
		if (!odbSaveFile(odb)) {
			rc = 1;
		}
	}
	if (rc != 0) {
		odbString err;
		odbGetError(err);
		printf("command failed (code=%d): %.*s\n", odbGetLastErrorCode(), err[0], err + 1);
	}
	odbCloseFile(odb);
	return rc;
}
