#include "document.h"

#include "odb_errors.h"

#include <stdlib.h>
#include <string.h>

struct ObjectDBDocument {
	odbRef odb;
	char *path;
};

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

static char *objectdb_document_join_path(const char *parent_path, const char *name) {
	if (!parent_path || !parent_path[0]) {
		return g_strdup(name);
	}

	return g_strdup_printf("%s.%s", parent_path, name);
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
		OSType type = 0;
		odbString child_path;

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

		g_ptr_array_add(entries, entry);
	}

	return entries;
}

void objectdb_entry_free(ObjectDBEntry *entry) {
	if (!entry) {
		return;
	}

	g_free(entry->name);
	g_free(entry->full_path);
	g_free(entry);
}
