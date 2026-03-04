#ifndef OBJECTDB_DOCUMENT_H
#define OBJECTDB_DOCUMENT_H

#include "odb_engine.h"

#include <glib.h>

typedef struct ObjectDBDocument ObjectDBDocument;

typedef struct ObjectDBEntry {
	char *name;
	char *full_path;
	char *value_text;
	char *kind_text;
	odbValueType value_type;
	gboolean is_table;
} ObjectDBEntry;

ObjectDBDocument *objectdb_document_new(void);
void objectdb_document_free(ObjectDBDocument *document);

gboolean objectdb_document_open(ObjectDBDocument *document, const char *path, char **error_message);
void objectdb_document_close(ObjectDBDocument *document);

gboolean objectdb_document_is_open(const ObjectDBDocument *document);
const char *objectdb_document_get_path(const ObjectDBDocument *document);

GPtrArray *objectdb_document_get_children(ObjectDBDocument *document, const char *parent_path, char **error_message);

void objectdb_entry_free(ObjectDBEntry *entry);

#endif
