#include "value_format.h"

#include <glib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

extern long gethandlesize(Handle h);

static char *objectdb_value_format_pascal_string(Handle string_handle) {
	long size = 0;
	unsigned char *data = NULL;

	if (!string_handle || !*string_handle) {
		return g_strdup("");
	}

	size = gethandlesize(string_handle);
	if (size <= 0) {
		return g_strdup("");
	}

	data = *string_handle;

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
			return g_strndup((const char *)(data + 4), len);
		}
	}

	if (data[0] <= (unsigned char)(size - 1)) {
		return g_strndup((const char *)(data + 1), data[0]);
	}

	return g_strndup((const char *)data, size);
}

static char *objectdb_value_format_fourcc(OSType type) {
	char text[5];

	text[0] = (char)((type >> 24) & 0xff);
	text[1] = (char)((type >> 16) & 0xff);
	text[2] = (char)((type >> 8) & 0xff);
	text[3] = (char)(type & 0xff);
	text[4] = '\0';

	return g_strdup(text);
}

static const char *objectdb_value_format_direction_name(odbDirection dir) {
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

static char *objectdb_value_format_rect(odbRect **rect_handle) {
	if (!rect_handle || !*rect_handle) {
		return g_strdup("<rect>");
	}
	odbRect *rect = *rect_handle;
	return g_strdup_printf("%d,%d,%d,%d", (int)rect->top, (int)rect->left, (int)rect->bottom, (int)rect->right);
}

static char *objectdb_value_format_rgb(odbRGBColor **rgb_handle) {
	if (!rgb_handle || !*rgb_handle) {
		return g_strdup("<rgb>");
	}
	odbRGBColor *rgb = *rgb_handle;
	return g_strdup_printf("%" PRIu16 ",%" PRIu16 ",%" PRIu16, rgb->red, rgb->green, rgb->blue);
}

static char *objectdb_value_format_pattern(odbPattern **pattern_handle) {
	GString *text = NULL;
	unsigned char *bytes = NULL;

	if (!pattern_handle || !*pattern_handle) {
		return g_strdup("<pattern>");
	}

	bytes = (unsigned char *)*pattern_handle;
	text = g_string_sized_new(16);
	for (int i = 0; i < 8; ++i) {
		g_string_append_printf(text, "%02x", bytes[i]);
	}
	return g_string_free(text, FALSE);
}

static char *objectdb_value_format_external(Handle external_handle) {
	long size = 0;
	if (!external_handle) {
		return g_strdup("<external>");
	}
	size = gethandlesize(external_handle);
	if (size < 0) {
		return g_strdup("<external>");
	}
	return g_strdup_printf("<external %ld bytes>", size);
}

static char *objectdb_value_format_date(uint32_t mac_seconds) {
	static const uint32_t kMacEpochDelta = 2082844800U;
	time_t unix_seconds = 0;
	struct tm local_tm;
	char formatted[64];
	char *hour_start = NULL;

	if (mac_seconds < kMacEpochDelta) {
		return g_strdup_printf("%u", mac_seconds);
	}

	unix_seconds = (time_t)(mac_seconds - kMacEpochDelta);
	if (!localtime_r(&unix_seconds, &local_tm)) {
		return g_strdup_printf("%u", mac_seconds);
	}

	if (strftime(formatted, sizeof(formatted), "%m/%d/%Y; %I:%M:%S %p", &local_tm) == 0) {
		return g_strdup_printf("%u", mac_seconds);
	}

	/* Match requested style with non-zero-padded hour (e.g. 2:12:12 PM). */
	hour_start = strstr(formatted, "; ");
	if (hour_start && hour_start[2] == '0') {
		memmove(hour_start + 2, hour_start + 3, strlen(hour_start + 3) + 1);
	}

	return g_strdup(formatted);
}

char *objectdb_value_format_kind(odbValueType type) {
	switch (type) {
		case tablevaluetype:
			return g_strdup("table");
		case stringvaluetype:
			return g_strdup("string");
		case longvaluetype:
			return g_strdup("long");
		case shortvaluetype:
			return g_strdup("short");
		case charvaluetype:
			return g_strdup("char");
		case booleanvaluetype:
			return g_strdup("bool");
		case datevaluetype:
			return g_strdup("date");
		case directionvaluetype:
			return g_strdup("direction");
		case string4valuetype:
			return g_strdup("string4");
		case tokenvaluetype:
			return g_strdup("token");
		case fixedvaluetype:
			return g_strdup("fixed");
		case pointvaluetype:
			return g_strdup("point");
		case rectvaluetype:
			return g_strdup("rect");
		case patternvaluetype:
			return g_strdup("pattern");
		case rgbvaluetype:
			return g_strdup("rgb");
		case singlevaluetype:
			return g_strdup("single");
		case enumeratorvaluetype:
			return g_strdup("enum");
		case binaryvaluetype:
			return g_strdup("binary");
		case externalvaluetype:
			return g_strdup("external");
		case scriptvaluetype:
			return g_strdup("script");
		case menubarvaluetype:
			return g_strdup("menubar");
		case picturevaluetype:
			return g_strdup("picture");
		case outlinevaluetype:
			return g_strdup("outline");
		case wptextvaluetype:
			return g_strdup("wptext");
		default:
			return g_strdup("unknown");
	}
}

char *objectdb_value_format_value(odbValueType type, const odbValueRecord *value) {
	const char *direction_name = NULL;

	if (type == tablevaluetype) {
		return g_strdup("on disk");
	}

	if (!value) {
		return g_strdup("");
	}

	switch (type) {
		case stringvaluetype:
			return objectdb_value_format_pascal_string(value->data.stringvalue);
		case longvaluetype:
			return g_strdup_printf("%d", value->data.longvalue);
		case shortvaluetype:
			return g_strdup_printf("%d", value->data.intvalue);
		case charvaluetype:
			if (g_ascii_isprint(value->data.chvalue)) {
				return g_strdup_printf("%c", value->data.chvalue);
			}
			return g_strdup_printf("\\x%02X", value->data.chvalue);
		case booleanvaluetype:
			return g_strdup(value->data.flvalue ? "true" : "false");
		case datevaluetype:
			return objectdb_value_format_date(value->data.datevalue);
		case directionvaluetype:
			direction_name = objectdb_value_format_direction_name(value->data.dirvalue);
			if (direction_name) {
				return g_strdup(direction_name);
			}
			return g_strdup_printf("%d", value->data.dirvalue);
		case string4valuetype:
			return objectdb_value_format_fourcc(value->data.ostypevalue);
		case tokenvaluetype:
			return g_strdup_printf("%d", (int)value->data.tokenvalue);
		case fixedvaluetype:
			return g_strdup_printf("%" PRId32, value->data.fixedvalue);
		case pointvaluetype:
			return g_strdup_printf("%d,%d", (int)value->data.pointvalue.v, (int)value->data.pointvalue.h);
		case rectvaluetype:
			return objectdb_value_format_rect(value->data.rectvalue);
		case patternvaluetype:
			return objectdb_value_format_pattern(value->data.patternvalue);
		case rgbvaluetype:
			return objectdb_value_format_rgb(value->data.rgbvalue);
		case singlevaluetype:
			return g_strdup_printf("%g", (double)value->data.singlevalue);
		case enumeratorvaluetype:
			return objectdb_value_format_fourcc(value->data.enumvalue);
		case binaryvaluetype:
			return g_strdup("<binary>");
		case externalvaluetype:
		case scriptvaluetype:
		case menubarvaluetype:
		case outlinevaluetype:
		case wptextvaluetype:
		case picturevaluetype:
			return objectdb_value_format_external(value->data.externalvalue);
		case doublevaluetype:
			if (value->data.doublevalue && *value->data.doublevalue) {
				return g_strdup_printf("%g", **value->data.doublevalue);
			}
			return g_strdup("0");
		default:
			return g_strdup("<unhandled>");
	}
}
