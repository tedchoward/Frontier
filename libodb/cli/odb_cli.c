#include "odb_engine.h"
#include "odb_errors.h"
#include "odb_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

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
	printf("  odb_cli <dbpath> set <path> token <int16>\n");
	printf("  odb_cli <dbpath> set <path> address <value>\n");
	printf("  odb_cli <dbpath> set <path> binary <hex-bytes>\n");
	printf("  odb_cli <dbpath> set <path> objspec <hex-bytes>\n");
	printf("  odb_cli <dbpath> set <path> filespec <hex-bytes>\n");
	printf("  odb_cli <dbpath> set <path> alias <hex-bytes>\n");
	printf("  odb_cli <dbpath> set <path> list <hex-bytes>\n");
	printf("  odb_cli <dbpath> set <path> record <hex-bytes>\n");
	printf("  odb_cli <dbpath> set <path> fixed <int32>\n");
	printf("  odb_cli <dbpath> set <path> point <v> <h>\n");
	printf("  odb_cli <dbpath> set <path> rect <top> <left> <bottom> <right>\n");
	printf("  odb_cli <dbpath> set <path> pattern <16-hex-digits>\n");
	printf("  odb_cli <dbpath> set <path> rgb <red> <green> <blue>\n");
	printf("  odb_cli <dbpath> set <path> single <value>\n");
	printf("  odb_cli <dbpath> set <path> double <value>\n");
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

static int parse_i16(const char *text, int16_t *out) {
	char *end = NULL;
	long v = strtol(text, &end, 0);
	if (!end || *end != '\0' || v < SHRT_MIN || v > SHRT_MAX) {
		return 0;
	}
	*out = (int16_t)v;
	return 1;
}

static int parse_i32(const char *text, int32_t *out) {
	char *end = NULL;
	long v = strtol(text, &end, 0);
	if (!end || *end != '\0' || v < INT32_MIN || v > INT32_MAX) {
		return 0;
	}
	*out = (int32_t)v;
	return 1;
}

static int parse_u16(const char *text, uint16_t *out) {
	char *end = NULL;
	unsigned long v = strtoul(text, &end, 0);
	if (!end || *end != '\0' || v > UINT16_MAX) {
		return 0;
	}
	*out = (uint16_t)v;
	return 1;
}

static int parse_hex_pattern16(const char *text, unsigned char out[8]) {
	if (!text || strlen(text) != 16) {
		return 0;
	}
	for (int i = 0; i < 8; ++i) {
		char hi = text[i * 2];
		char lo = text[i * 2 + 1];
		if (!isxdigit((unsigned char)hi) || !isxdigit((unsigned char)lo)) {
			return 0;
		}
		unsigned int byte = 0;
		if (sscanf(&text[i * 2], "%2x", &byte) != 1) {
			return 0;
		}
		out[i] = (unsigned char)byte;
	}
	return 1;
}

static Handle parse_hex_bytes_handle(const char *text) {
	Handle h = NULL;
	size_t len = 0;
	if (!text) {
		return NULL;
	}
	len = strlen(text);
	if ((len % 2) != 0) {
		return NULL;
	}
	if (!newhandle((long)(len / 2), &h)) {
		return NULL;
	}
	for (size_t i = 0; i < len / 2; ++i) {
		unsigned int byte = 0;
		char hi = text[i * 2];
		char lo = text[i * 2 + 1];
		if (!isxdigit((unsigned char)hi) || !isxdigit((unsigned char)lo)) {
			disposehandle(h);
			return NULL;
		}
		if (sscanf(&text[i * 2], "%2x", &byte) != 1) {
			disposehandle(h);
			return NULL;
		}
		(*h)[i] = (unsigned char)byte;
	}
	return h;
}

static char *format_hex_bytes(Handle h) {
	long size = 0;
	char *out = NULL;
	if (!h || !*h) {
		out = (char *)malloc(1);
		if (out) {
			out[0] = '\0';
		}
		return out;
	}
	size = gethandlesize(h);
	if (size <= 0) {
		out = (char *)malloc(1);
		if (out) {
			out[0] = '\0';
		}
		return out;
	}
	out = (char *)malloc((size_t)size * 2 + 1);
	if (!out) {
		return NULL;
	}
	for (long i = 0; i < size; ++i) {
		snprintf(out + (i * 2), 3, "%02x", (unsigned char)(*h)[i]);
	}
	out[size * 2] = '\0';
	return out;
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
		case tokenvaluetype:
			printf("%d", (int)value.data.tokenvalue);
			break;
		case addressvaluetype: {
			char buffer[1024];
			if (handle_to_c_string(value.data.addressvalue, buffer, sizeof(buffer))) {
				printf("%s", buffer);
			} else {
				printf("<address>");
			}
			break;
		}
		case binaryvaluetype: {
			char *hex = format_hex_bytes(value.data.binaryvalue);
			printf("%s", hex ? hex : "");
			free(hex);
			break;
		}
		case objspecvaluetype: {
			char *hex = format_hex_bytes(value.data.objspecvalue);
			printf("%s", hex ? hex : "");
			free(hex);
			break;
		}
		case filespecvaluetype: {
			char *hex = format_hex_bytes(value.data.filespecvalue);
			printf("%s", hex ? hex : "");
			free(hex);
			break;
		}
		case aliasvaluetype: {
			char *hex = format_hex_bytes(value.data.aliasvalue);
			printf("%s", hex ? hex : "");
			free(hex);
			break;
		}
		case listvaluetype: {
			char *hex = format_hex_bytes(value.data.listvalue);
			printf("%s", hex ? hex : "");
			free(hex);
			break;
		}
		case recordvaluetype: {
			char *hex = format_hex_bytes(value.data.recordvalue);
			printf("%s", hex ? hex : "");
			free(hex);
			break;
		}
		case fixedvaluetype:
			printf("%" PRId32, value.data.fixedvalue);
			break;
		case pointvaluetype:
			printf("%d,%d", (int)value.data.pointvalue.v, (int)value.data.pointvalue.h);
			break;
		case rectvaluetype:
			if (value.data.rectvalue && *value.data.rectvalue) {
				odbRect *r = *value.data.rectvalue;
				printf("%d,%d,%d,%d", (int)r->top, (int)r->left, (int)r->bottom, (int)r->right);
			} else {
				printf("<rect>");
			}
			break;
		case patternvaluetype:
			if (value.data.patternvalue && *value.data.patternvalue) {
				unsigned char *p = (unsigned char *)*value.data.patternvalue;
				for (int i = 0; i < 8; ++i) {
					printf("%02x", p[i]);
				}
			} else {
				printf("<pattern>");
			}
			break;
		case rgbvaluetype:
			if (value.data.rgbvalue && *value.data.rgbvalue) {
				odbRGBColor *c = *value.data.rgbvalue;
				printf("%" PRIu16 ",%" PRIu16 ",%" PRIu16, c->red, c->green, c->blue);
			} else {
				printf("<rgb>");
			}
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
	if (argc < 6) {
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
		if (argc != 6) {
			return 1;
		}
		if (!value[0] || value[1] != '\0') {
			return 1;
		}
		rec.valuetype = charvaluetype;
		rec.data.chvalue = (unsigned char)value[0];
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "short") == 0) {
		if (argc != 6) {
			return 1;
		}
		int16_t v = 0;
		if (!parse_i16(value, &v)) {
			return 1;
		}
		rec.valuetype = shortvaluetype;
		rec.data.intvalue = v;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "long") == 0) {
		if (argc != 6) {
			return 1;
		}
		int32_t v = 0;
		if (!parse_i32(value, &v)) {
			return 1;
		}
		rec.valuetype = longvaluetype;
		rec.data.longvalue = v;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "bool") == 0) {
		if (argc != 6) {
			return 1;
		}
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
		if (argc != 6) {
			return 1;
		}
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
		if (argc != 6) {
			return 1;
		}
		odbDirection dir = nodirection;
		if (!parse_direction(value, &dir)) {
			return 1;
		}
		rec.valuetype = directionvaluetype;
		rec.data.dirvalue = dir;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "string4") == 0) {
		if (argc != 6) {
			return 1;
		}
		OSType code = 0;
		if (!parse_fourcc(value, &code)) {
			return 1;
		}
		rec.valuetype = string4valuetype;
		rec.data.ostypevalue = code;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "token") == 0) {
		if (argc != 6) {
			return 1;
		}
		int16_t token = 0;
		if (!parse_i16(value, &token)) {
			return 1;
		}
		rec.valuetype = tokenvaluetype;
		rec.data.tokenvalue = token;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "address") == 0) {
		if (argc != 6) {
			return 1;
		}
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
		rec.valuetype = addressvaluetype;
		rec.data.addressvalue = (Handle)hstring;
		int ok = odbSetValue(odb, bspath, &rec) ? 0 : 1;
		disposehandle((Handle)hstring);
		return ok;
	}

	if (strcmp(type, "binary") == 0 || strcmp(type, "objspec") == 0 || strcmp(type, "filespec") == 0
		|| strcmp(type, "alias") == 0 || strcmp(type, "list") == 0 || strcmp(type, "record") == 0) {
		if (argc != 6) {
			return 1;
		}
		Handle h = parse_hex_bytes_handle(value);
		if (!h) {
			return 1;
		}
		if (strcmp(type, "binary") == 0) {
			rec.valuetype = binaryvaluetype;
			rec.data.binaryvalue = h;
		} else if (strcmp(type, "objspec") == 0) {
			rec.valuetype = objspecvaluetype;
			rec.data.objspecvalue = h;
		} else if (strcmp(type, "filespec") == 0) {
			rec.valuetype = filespecvaluetype;
			rec.data.filespecvalue = h;
		} else if (strcmp(type, "alias") == 0) {
			rec.valuetype = aliasvaluetype;
			rec.data.aliasvalue = h;
		} else if (strcmp(type, "list") == 0) {
			rec.valuetype = listvaluetype;
			rec.data.listvalue = h;
		} else if (strcmp(type, "record") == 0) {
			rec.valuetype = recordvaluetype;
			rec.data.recordvalue = h;
		} else {
			disposehandle(h);
			return 1;
		}
		int ok = odbSetValue(odb, bspath, &rec) ? 0 : 1;
		disposehandle(h);
		return ok;
	}

	if (strcmp(type, "fixed") == 0) {
		if (argc != 6) {
			return 1;
		}
		int32_t fixed = 0;
		if (!parse_i32(value, &fixed)) {
			return 1;
		}
		rec.valuetype = fixedvaluetype;
		rec.data.fixedvalue = fixed;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "point") == 0) {
		if (argc != 7) {
			return 1;
		}
		int16_t v = 0;
		int16_t h = 0;
		if (!parse_i16(argv[5], &v) || !parse_i16(argv[6], &h)) {
			return 1;
		}
		rec.valuetype = pointvaluetype;
		rec.data.pointvalue.v = v;
		rec.data.pointvalue.h = h;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "rect") == 0) {
		if (argc != 9) {
			return 1;
		}
		odbRect rect;
		if (!parse_i16(argv[5], &rect.top) || !parse_i16(argv[6], &rect.left) ||
			!parse_i16(argv[7], &rect.bottom) || !parse_i16(argv[8], &rect.right)) {
			return 1;
		}
		Handle hrect = make_handle_from_bytes(&rect, sizeof(rect));
		if (!hrect) {
			return 1;
		}
		rec.valuetype = rectvaluetype;
		rec.data.rectvalue = (odbRect **)hrect;
		int ok = odbSetValue(odb, bspath, &rec) ? 0 : 1;
		disposehandle(hrect);
		return ok;
	}

	if (strcmp(type, "pattern") == 0) {
		if (argc != 6) {
			return 1;
		}
		odbPattern pattern = {{0}};
		if (!parse_hex_pattern16(value, pattern.bytes)) {
			return 1;
		}
		Handle hpattern = make_handle_from_bytes(&pattern, sizeof(pattern));
		if (!hpattern) {
			return 1;
		}
		rec.valuetype = patternvaluetype;
		rec.data.patternvalue = (odbPattern **)hpattern;
		int ok = odbSetValue(odb, bspath, &rec) ? 0 : 1;
		disposehandle(hpattern);
		return ok;
	}

	if (strcmp(type, "rgb") == 0) {
		if (argc != 8) {
			return 1;
		}
		odbRGBColor color;
		if (!parse_u16(argv[5], &color.red) || !parse_u16(argv[6], &color.green) || !parse_u16(argv[7], &color.blue)) {
			return 1;
		}
		Handle hrgb = make_handle_from_bytes(&color, sizeof(color));
		if (!hrgb) {
			return 1;
		}
		rec.valuetype = rgbvaluetype;
		rec.data.rgbvalue = (odbRGBColor **)hrgb;
		int ok = odbSetValue(odb, bspath, &rec) ? 0 : 1;
		disposehandle(hrgb);
		return ok;
	}

	if (strcmp(type, "single") == 0) {
		if (argc != 6) {
			return 1;
		}
		char *end = NULL;
		float v = strtof(value, &end);
		if (!end || *end != '\0') {
			return 1;
		}
		rec.valuetype = singlevaluetype;
		rec.data.singlevalue = v;
		return odbSetValue(odb, bspath, &rec) ? 0 : 1;
	}

	if (strcmp(type, "double") == 0) {
		if (argc != 6) {
			return 1;
		}
		char *end = NULL;
		double d = strtod(value, &end);
		if (!end || *end != '\0') {
			return 1;
		}
		Handle hdouble = make_handle_from_bytes(&d, sizeof(d));
		if (!hdouble) {
			return 1;
		}
		rec.valuetype = doublevaluetype;
		rec.data.doublevalue = (double **)hdouble;
		int ok = odbSetValue(odb, bspath, &rec) ? 0 : 1;
		disposehandle(hdouble);
		return ok;
	}

	if (strcmp(type, "enum") == 0) {
		if (argc != 6) {
			return 1;
		}
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
