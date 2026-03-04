#ifndef OBJECTDB_TREE_MODEL_H
#define OBJECTDB_TREE_MODEL_H

#include "document.h"

#include <gtk/gtk.h>

enum {
	OBJECTDB_MODEL_COL_NAME = 0,
	OBJECTDB_MODEL_COL_VALUE,
	OBJECTDB_MODEL_COL_KIND,
	OBJECTDB_MODEL_COL_PATH,
	OBJECTDB_MODEL_COL_IS_TABLE,
	OBJECTDB_MODEL_COL_CHILDREN_LOADED,
	OBJECTDB_MODEL_COL_IS_PLACEHOLDER,
	OBJECTDB_MODEL_COL_COUNT
};

GtkTreeStore *objectdb_tree_model_new(void);
void objectdb_tree_model_clear(GtkTreeStore *tree_store);

gboolean objectdb_tree_model_populate_root(GtkTreeStore *tree_store, ObjectDBDocument *document, char **error_message);
gboolean objectdb_tree_model_load_children(GtkTreeStore *tree_store, GtkTreeIter *parent_iter, ObjectDBDocument *document, char **error_message);

#endif
