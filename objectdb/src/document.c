#include "document.h"
#include "value_format.h"

#include "odb_errors.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct ObjectDBDocument {
	odbRef odb;
	char *path;
};

extern Handle odbNewHandle(size_t size);
extern void odbDisposeHandle(Handle h);

static void objectdb_document_set_error(char **error_message) {
	if (!error_message) {
		return;
	}

	odbString error_text;
	odbGetError(error_text);

	if (error_text[0] > 0) {
		*error_message = g_strndup((const char *)(error_text + 1), error_text[0]);
		return;
	}

	*error_message = g_strdup_printf("ODB error %d", odbGetLastErrorCode());
}

static void objectdb_document_path_to_odb(const char *path, odbString out) {
	size_t len = 0;

	if (!path) {
		out[0] = 0;
		return;
	}

	len = strlen(path);
	if (len > 255) {
		len = 255;
	}

	out[0] = (unsigned char)len;
	if (len > 0) {
		memcpy(&out[1], path, len);
	}
}

static char *objectdb_document_pascal_to_c_string(const odbString input) {
	return g_strndup((const char *)(input + 1), input[0]);
}

static gboolean objectdb_document_is_identifier_segment(const char *name) {
	const unsigned char *p = (const unsigned char *)name;
	if (!p || !p[0]) {
		return FALSE;
	}
	if (!g_ascii_isalpha(*p)) {
		return FALSE;
	}
	for (p = p + 1; *p; ++p) {
		if (!g_ascii_isalnum(*p)) {
			return FALSE;
		}
	}
	return TRUE;
}

static char *objectdb_document_join_path(const char *parent_path, const char *name) {
	GString *quoted = NULL;
	const char *p = NULL;

	if (!parent_path || !parent_path[0]) {
		return g_strdup(name);
	}

	if (objectdb_document_is_identifier_segment(name)) {
		return g_strdup_printf("%s.%s", parent_path, name);
	}

	quoted = g_string_new(parent_path);
	g_string_append(quoted, ".[\"");
	for (p = name; *p; ++p) {
		if (*p == '"' || *p == '\\') {
			g_string_append_c(quoted, '\\');
		}
		g_string_append_c(quoted, *p);
	}
	g_string_append(quoted, "\"]");
	return g_string_free(quoted, FALSE);
}

static gboolean objectdb_document_parse_int32(const char *text, int32_t *out_value) {
	char *end = NULL;
	long value = 0;

	if (!text || !out_value) {
		return FALSE;
	}

	errno = 0;
	value = strtol(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0' || value < INT32_MIN || value > INT32_MAX) {
		return FALSE;
	}

	*out_value = (int32_t)value;
	return TRUE;
}

static gboolean objectdb_document_parse_uint32(const char *text, uint32_t *out_value) {
	char *end = NULL;
	unsigned long value = 0;

	if (!text || !out_value) {
		return FALSE;
	}

	errno = 0;
	value = strtoul(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0' || value > UINT32_MAX) {
		return FALSE;
	}

	*out_value = (uint32_t)value;
	return TRUE;
}

static gboolean objectdb_document_parse_double(const char *text, double *out_value) {
	char *end = NULL;
	double value = 0.0;

	if (!text || !out_value) {
		return FALSE;
	}

	errno = 0;
	value = g_ascii_strtod(text, &end);
	if (errno != 0 || end == text || *end != '\0') {
		return FALSE;
	}

	*out_value = value;
	return TRUE;
}

static gboolean objectdb_document_parse_fourcc(const char *text, OSType *out_type) {
	const unsigned char *bytes = (const unsigned char *)text;

	if (!text || !out_type || strlen(text) != 4) {
		return FALSE;
	}

	*out_type = ((OSType)bytes[0] << 24) | ((OSType)bytes[1] << 16) | ((OSType)bytes[2] << 8) | (OSType)bytes[3];
	return TRUE;
}

static gboolean objectdb_document_parse_date(const char *text, uint32_t *out_mac_seconds) {
	static const uint32_t kMacEpochDelta = 2082844800U;
	struct tm tm_value;
	time_t unix_seconds = 0;
	uint32_t numeric_value = 0;
	int month = 0;
	int day = 0;
	int year = 0;
	int hour12 = 0;
	int minute = 0;
	int second = 0;
	char ampm[3];
	int parsed = 0;
	int hour24 = 0;

	if (!text || !out_mac_seconds) {
		return FALSE;
	}

	if (objectdb_document_parse_uint32(text, &numeric_value)) {
		*out_mac_seconds = numeric_value;
		return TRUE;
	}

	ampm[0] = '\0';
	ampm[1] = '\0';
	ampm[2] = '\0';
	parsed = sscanf(text, "%d/%d/%d; %d:%d:%d %2s", &month, &day, &year, &hour12, &minute, &second, ampm);
	if (parsed != 7) {
		parsed = sscanf(text, "%d/%d/%d %d:%d:%d %2s", &month, &day, &year, &hour12, &minute, &second, ampm);
	}
	if (parsed != 7) {
		return FALSE;
	}
	if (month < 1 || month > 12 || day < 1 || day > 31 || year < 1904 || hour12 < 1 || hour12 > 12 || minute < 0 || minute > 59 || second < 0 || second > 59) {
		return FALSE;
	}
	if (g_ascii_strcasecmp(ampm, "AM") != 0 && g_ascii_strcasecmp(ampm, "PM") != 0) {
		return FALSE;
	}

	hour24 = hour12 % 12;
	if (g_ascii_strcasecmp(ampm, "PM") == 0) {
		hour24 += 12;
	}

	memset(&tm_value, 0, sizeof(tm_value));
	tm_value.tm_year = year - 1900;
	tm_value.tm_mon = month - 1;
	tm_value.tm_mday = day;
	tm_value.tm_hour = hour24;
	tm_value.tm_min = minute;
	tm_value.tm_sec = second;

	tm_value.tm_isdst = -1;
	unix_seconds = mktime(&tm_value);
	if (unix_seconds < 0) {
		return FALSE;
	}

	*out_mac_seconds = (uint32_t)unix_seconds + kMacEpochDelta;
	return TRUE;
}

static gboolean objectdb_document_parse_direction(const char *text, odbDirection *out_dir) {
	int32_t numeric = 0;

	if (!text || !out_dir) {
		return FALSE;
	}

	if (g_ascii_strcasecmp(text, "none") == 0) {
		*out_dir = nodirection;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "up") == 0) {
		*out_dir = dir_up;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "down") == 0) {
		*out_dir = dir_down;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "left") == 0) {
		*out_dir = dir_left;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "right") == 0) {
		*out_dir = dir_right;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "flatup") == 0) {
		*out_dir = dir_flatup;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "flatdown") == 0) {
		*out_dir = dir_flatdown;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "sorted") == 0) {
		*out_dir = dir_sorted;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "pageup") == 0) {
		*out_dir = dir_pageup;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "pagedown") == 0) {
		*out_dir = dir_pagedown;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "pageleft") == 0) {
		*out_dir = dir_pageleft;
		return TRUE;
	}
	if (g_ascii_strcasecmp(text, "pageright") == 0) {
		*out_dir = dir_pageright;
		return TRUE;
	}

	if (objectdb_document_parse_int32(text, &numeric)) {
		*out_dir = (odbDirection)numeric;
		return TRUE;
	}

	return FALSE;
}

static gboolean objectdb_document_parse_int_list(const char *text, int expected_count, int *out_values) {
	char **parts = NULL;
	gboolean ok = TRUE;

	if (!text || !out_values || expected_count <= 0) {
		return FALSE;
	}

	parts = g_strsplit(text, ",", expected_count + 1);
	for (int i = 0; i < expected_count; ++i) {
		char *trimmed = NULL;
		int32_t value = 0;

		if (!parts[i]) {
			ok = FALSE;
			break;
		}
		trimmed = g_strstrip(parts[i]);
		if (!objectdb_document_parse_int32(trimmed, &value)) {
			ok = FALSE;
			break;
		}
		out_values[i] = value;
	}

	if (ok && parts[expected_count] != NULL) {
		ok = FALSE;
	}

	g_strfreev(parts);
	return ok;
}

static gboolean objectdb_document_parse_pattern(const char *text, odbPattern *pattern) {
	char hex_only[17];
	size_t hex_len = 0;

	if (!text || !pattern) {
		return FALSE;
	}

	for (const char *p = text; *p; ++p) {
		if (g_ascii_isspace(*p)) {
			continue;
		}
		if (!g_ascii_isxdigit(*p) || hex_len >= 16) {
			return FALSE;
		}
		hex_only[hex_len++] = (char)*p;
	}

	if (hex_len != 16) {
		return FALSE;
	}
	hex_only[16] = '\0';

	for (int i = 0; i < 8; ++i) {
		char byte_text[3];
		char *end = NULL;
		unsigned long value = 0;

		byte_text[0] = hex_only[i * 2];
		byte_text[1] = hex_only[(i * 2) + 1];
		byte_text[2] = '\0';
		value = strtoul(byte_text, &end, 16);
		if (!end || *end != '\0' || value > 0xFF) {
			return FALSE;
		}
		pattern->bytes[i] = (unsigned char)value;
	}

	return TRUE;
}

static Handle objectdb_document_make_handle_from_bytes(const void *data, size_t size) {
	Handle h = NULL;

	if (!data || size == 0) {
		return NULL;
	}

	if (!h) {
		h = odbNewHandle(size);
	}
	if (!h) {
		return NULL;
	}

	memcpy(*h, data, size);
	return h;
}

static Handle objectdb_document_make_string_handle(const char *text) {
	size_t len = 0;
	Handle hstring = NULL;
	unsigned char *bytes = NULL;

	if (!text) {
		return NULL;
	}

	len = strlen(text);
	if (len > 255) {
		return NULL;
	}

	hstring = odbNewHandle(len + 1);
	if (!hstring || !*hstring) {
		return NULL;
	}
	bytes = *hstring;
	bytes[0] = (unsigned char)len;
	if (len > 0) {
		memcpy(bytes + 1, text, len);
	}

	return hstring;
}

static void objectdb_document_set_parse_error(char **error_message, const char *message) {
	if (!error_message) {
		return;
	}
	*error_message = g_strdup(message ? message : "Invalid value");
}

static gboolean objectdb_document_build_value(odbValueType type, const char *text, odbValueRecord *value, char **error_message) {
	int32_t i32 = 0;
	double f64 = 0.0;
	int parts[4];
	OSType code = 0;
	Handle h = NULL;

	memset(value, 0, sizeof(*value));
	value->valuetype = type;

	switch (type) {
		case stringvaluetype:
			h = objectdb_document_make_string_handle(text);
			if (!h) {
				objectdb_document_set_parse_error(error_message, "String is too long or invalid");
				return FALSE;
			}
			value->data.stringvalue = h;
			return TRUE;
		case longvaluetype:
			if (!objectdb_document_parse_int32(text, &i32)) {
				objectdb_document_set_parse_error(error_message, "Expected integer value");
				return FALSE;
			}
			value->data.longvalue = i32;
			return TRUE;
		case shortvaluetype:
			if (!objectdb_document_parse_int32(text, &i32) || i32 < INT16_MIN || i32 > INT16_MAX) {
				objectdb_document_set_parse_error(error_message, "Expected 16-bit integer value");
				return FALSE;
			}
			value->data.intvalue = (short)i32;
			return TRUE;
		case charvaluetype:
			if (g_str_has_prefix(text, "\\x") || g_str_has_prefix(text, "\\X")) {
				char *end = NULL;
				unsigned long ch = strtoul(text + 2, &end, 16);
				if (!end || *end != '\0' || ch > 0xFF) {
					objectdb_document_set_parse_error(error_message, "Expected single character or \\xNN");
					return FALSE;
				}
				value->data.chvalue = (char)ch;
				return TRUE;
			}
			if (strlen(text) != 1) {
				objectdb_document_set_parse_error(error_message, "Expected single character or \\xNN");
				return FALSE;
			}
			value->data.chvalue = text[0];
			return TRUE;
		case booleanvaluetype:
			if (g_ascii_strcasecmp(text, "true") == 0 || strcmp(text, "1") == 0) {
				value->data.flvalue = 1;
				return TRUE;
			}
			if (g_ascii_strcasecmp(text, "false") == 0 || strcmp(text, "0") == 0) {
				value->data.flvalue = 0;
				return TRUE;
			}
			objectdb_document_set_parse_error(error_message, "Expected true/false or 1/0");
			return FALSE;
		case datevaluetype:
			if (!objectdb_document_parse_date(text, &value->data.datevalue)) {
				objectdb_document_set_parse_error(error_message, "Expected date as MM/DD/YYYY; H:MM:SS AM/PM or raw integer");
				return FALSE;
			}
			return TRUE;
		case directionvaluetype:
			if (!objectdb_document_parse_direction(text, &value->data.dirvalue)) {
				objectdb_document_set_parse_error(error_message, "Expected direction name (up/down/...) or integer");
				return FALSE;
			}
			return TRUE;
		case string4valuetype:
			if (!objectdb_document_parse_fourcc(text, &code)) {
				objectdb_document_set_parse_error(error_message, "Expected exactly 4 characters");
				return FALSE;
			}
			value->data.ostypevalue = code;
			return TRUE;
		case tokenvaluetype:
			if (!objectdb_document_parse_int32(text, &i32)) {
				objectdb_document_set_parse_error(error_message, "Expected integer token");
				return FALSE;
			}
			value->data.tokenvalue = (short)i32;
			return TRUE;
		case fixedvaluetype:
			if (!objectdb_document_parse_int32(text, &i32)) {
				objectdb_document_set_parse_error(error_message, "Expected integer fixed value");
				return FALSE;
			}
			value->data.fixedvalue = i32;
			return TRUE;
		case pointvaluetype:
			if (!objectdb_document_parse_int_list(text, 2, parts)) {
				objectdb_document_set_parse_error(error_message, "Expected point as v,h");
				return FALSE;
			}
			value->data.pointvalue.v = (short)parts[0];
			value->data.pointvalue.h = (short)parts[1];
			return TRUE;
		case rectvaluetype: {
			odbRect rect;
			if (!objectdb_document_parse_int_list(text, 4, parts)) {
				objectdb_document_set_parse_error(error_message, "Expected rect as top,left,bottom,right");
				return FALSE;
			}
			rect.top = (short)parts[0];
			rect.left = (short)parts[1];
			rect.bottom = (short)parts[2];
			rect.right = (short)parts[3];
			h = objectdb_document_make_handle_from_bytes(&rect, sizeof(rect));
			if (!h) {
				objectdb_document_set_parse_error(error_message, "Failed to allocate rect value");
				return FALSE;
			}
			value->data.rectvalue = (odbRect **)h;
			return TRUE;
		}
		case patternvaluetype: {
			odbPattern pattern;
			if (!objectdb_document_parse_pattern(text, &pattern)) {
				objectdb_document_set_parse_error(error_message, "Expected 16 hex digits for pattern");
				return FALSE;
			}
			h = objectdb_document_make_handle_from_bytes(&pattern, sizeof(pattern));
			if (!h) {
				objectdb_document_set_parse_error(error_message, "Failed to allocate pattern value");
				return FALSE;
			}
			value->data.patternvalue = (odbPattern **)h;
			return TRUE;
		}
		case rgbvaluetype: {
			odbRGBColor rgb;
			if (!objectdb_document_parse_int_list(text, 3, parts)) {
				objectdb_document_set_parse_error(error_message, "Expected rgb as red,green,blue");
				return FALSE;
			}
			rgb.red = (uint16_t)parts[0];
			rgb.green = (uint16_t)parts[1];
			rgb.blue = (uint16_t)parts[2];
			h = objectdb_document_make_handle_from_bytes(&rgb, sizeof(rgb));
			if (!h) {
				objectdb_document_set_parse_error(error_message, "Failed to allocate rgb value");
				return FALSE;
			}
			value->data.rgbvalue = (odbRGBColor **)h;
			return TRUE;
		}
		case singlevaluetype:
			if (!objectdb_document_parse_double(text, &f64)) {
				objectdb_document_set_parse_error(error_message, "Expected floating-point value");
				return FALSE;
			}
			value->data.singlevalue = (float)f64;
			return TRUE;
		case enumeratorvaluetype:
			if (!objectdb_document_parse_fourcc(text, &code)) {
				objectdb_document_set_parse_error(error_message, "Expected exactly 4 characters for enum");
				return FALSE;
			}
			value->data.enumvalue = code;
			return TRUE;
		case doublevaluetype: {
			double stored = 0.0;
			if (!objectdb_document_parse_double(text, &f64)) {
				objectdb_document_set_parse_error(error_message, "Expected floating-point value");
				return FALSE;
			}
			stored = f64;
			h = objectdb_document_make_handle_from_bytes(&stored, sizeof(stored));
			if (!h) {
				objectdb_document_set_parse_error(error_message, "Failed to allocate double value");
				return FALSE;
			}
			value->data.doublevalue = (double **)h;
			return TRUE;
		}
		case tablevaluetype:
			objectdb_document_set_parse_error(error_message, "Tables are not directly editable");
			return FALSE;
		case binaryvaluetype:
		case externalvaluetype:
		case scriptvaluetype:
		case menubarvaluetype:
		case outlinevaluetype:
		case wptextvaluetype:
		case picturevaluetype:
		case addressvaluetype:
		case objspecvaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
		case listvaluetype:
		case recordvaluetype:
		default:
			objectdb_document_set_parse_error(error_message, "Editing is not supported for this value type yet");
			return FALSE;
	}
}

static void objectdb_document_dispose_input_value(odbValueRecord *value) {
	if (!value) {
		return;
	}

	switch (value->valuetype) {
		case stringvaluetype:
			if (value->data.stringvalue) {
				odbDisposeHandle(value->data.stringvalue);
			}
			break;
		case rectvaluetype:
			if (value->data.rectvalue) {
				odbDisposeHandle((Handle)value->data.rectvalue);
			}
			break;
		case patternvaluetype:
			if (value->data.patternvalue) {
				odbDisposeHandle((Handle)value->data.patternvalue);
			}
			break;
		case rgbvaluetype:
			if (value->data.rgbvalue) {
				odbDisposeHandle((Handle)value->data.rgbvalue);
			}
			break;
		case doublevaluetype:
			if (value->data.doublevalue) {
				odbDisposeHandle((Handle)value->data.doublevalue);
			}
			break;
		default:
			break;
	}
}

ObjectDBDocument *objectdb_document_new(void) {
	return g_new0(ObjectDBDocument, 1);
}

void objectdb_document_close(ObjectDBDocument *document) {
	if (!document) {
		return;
	}

	if (document->odb) {
		odbCloseFile(document->odb);
		document->odb = NULL;
	}

	g_clear_pointer(&document->path, g_free);
}

void objectdb_document_free(ObjectDBDocument *document) {
	if (!document) {
		return;
	}

	objectdb_document_close(document);
	g_free(document);
}

gboolean objectdb_document_open(ObjectDBDocument *document, const char *path, char **error_message) {
	odbRef odb = NULL;

	if (error_message) {
		*error_message = NULL;
	}

	if (!document || !path || !path[0]) {
		if (error_message) {
			*error_message = g_strdup("Invalid file path");
		}
		return FALSE;
	}

	if (!odbOpenPath(path, &odb)) {
		objectdb_document_set_error(error_message);
		return FALSE;
	}

	objectdb_document_close(document);
	document->odb = odb;
	document->path = g_strdup(path);
	return TRUE;
}

gboolean objectdb_document_is_open(const ObjectDBDocument *document) {
	return document && document->odb;
}

const char *objectdb_document_get_path(const ObjectDBDocument *document) {
	if (!document) {
		return NULL;
	}

	return document->path;
}

GPtrArray *objectdb_document_get_children(ObjectDBDocument *document, const char *parent_path, char **error_message) {
	const char *path = parent_path;
	odbString bspath;
	int32_t count = 0;
	GPtrArray *entries = NULL;

	if (error_message) {
		*error_message = NULL;
	}

	if (!document || !document->odb) {
		if (error_message) {
			*error_message = g_strdup("No document is open");
		}
		return NULL;
	}

	if (!path || !path[0]) {
		path = "root";
	}

	objectdb_document_path_to_odb(path, bspath);

	if (!odbCountItems(document->odb, bspath, &count)) {
		objectdb_document_set_error(error_message);
		return NULL;
	}

	entries = g_ptr_array_new_with_free_func((GDestroyNotify)objectdb_entry_free);

	for (int32_t index = 1; index <= count; ++index) {
		odbString bsname;
		ObjectDBEntry *entry = NULL;
		char *name = NULL;
		char *full_path = NULL;
		char *value_text = NULL;
		char *kind_text = NULL;
		OSType type = 0;
		odbString child_path;
		odbValueRecord value;
		gboolean have_value = FALSE;

		if (!odbGetNthItem(document->odb, bspath, index, bsname)) {
			objectdb_document_set_error(error_message);
			g_ptr_array_free(entries, TRUE);
			return NULL;
		}

		name = objectdb_document_pascal_to_c_string(bsname);
		full_path = objectdb_document_join_path(path, name);
		objectdb_document_path_to_odb(full_path, child_path);

		if (!odbGetType(document->odb, child_path, &type)) {
			objectdb_document_set_error(error_message);
			g_free(name);
			g_free(full_path);
			g_ptr_array_free(entries, TRUE);
			return NULL;
		}

		entry = g_new0(ObjectDBEntry, 1);
		entry->name = name;
		entry->full_path = full_path;
		entry->value_type = (odbValueType)type;
		entry->is_table = (type == tablevaluetype);
		entry->kind_text = objectdb_value_format_kind(entry->value_type);

		if (entry->is_table) {
			entry->value_text = objectdb_value_format_value(entry->value_type, NULL);
		} else {
			if (!odbGetValue(document->odb, child_path, &value)) {
				objectdb_document_set_error(error_message);
				objectdb_entry_free(entry);
				g_ptr_array_free(entries, TRUE);
				return NULL;
			}
			have_value = TRUE;
			value_text = objectdb_value_format_value(entry->value_type, &value);
			entry->value_text = value_text;
		}

		g_ptr_array_add(entries, entry);

		if (have_value) {
			odbDisposeValue(document->odb, &value);
		}
	}

	return entries;
}

gboolean objectdb_document_set_value(ObjectDBDocument *document, const char *path, const char *text, char **updated_text, char **error_message) {
	odbString bspath;
	OSType type = 0;
	odbValueRecord value;
	odbValueRecord readback;
	gboolean ok = FALSE;
	gboolean read_ok = FALSE;
	char *trimmed = NULL;

	if (updated_text) {
		*updated_text = NULL;
	}
	if (error_message) {
		*error_message = NULL;
	}

	if (!document || !document->odb) {
		if (error_message) {
			*error_message = g_strdup("No document is open");
		}
		return FALSE;
	}
	if (!path || !path[0]) {
		if (error_message) {
			*error_message = g_strdup("Invalid path");
		}
		return FALSE;
	}

	trimmed = g_strdup(text ? text : "");
	g_strstrip(trimmed);

	objectdb_document_path_to_odb(path, bspath);
	if (!odbGetType(document->odb, bspath, &type)) {
		objectdb_document_set_error(error_message);
		g_free(trimmed);
		return FALSE;
	}

	if (!objectdb_document_build_value((odbValueType)type, trimmed, &value, error_message)) {
		g_free(trimmed);
		return FALSE;
	}
	g_free(trimmed);

	ok = odbSetValue(document->odb, bspath, &value);
	objectdb_document_dispose_input_value(&value);
	if (!ok) {
		objectdb_document_set_error(error_message);
		return FALSE;
	}

	if (!odbSaveFile(document->odb)) {
		objectdb_document_set_error(error_message);
		return FALSE;
	}

	read_ok = odbGetValue(document->odb, bspath, &readback);
	if (!read_ok) {
		objectdb_document_set_error(error_message);
		return FALSE;
	}

	if (updated_text) {
		*updated_text = objectdb_value_format_value((odbValueType)type, &readback);
	}
	odbDisposeValue(document->odb, &readback);
	return TRUE;
}

void objectdb_entry_free(ObjectDBEntry *entry) {
	if (!entry) {
		return;
	}

	g_free(entry->name);
	g_free(entry->full_path);
	g_free(entry->value_text);
	g_free(entry->kind_text);
	g_free(entry);
}
