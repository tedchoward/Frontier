#include "tree_model.h"

static void objectdb_tree_model_append_placeholder(GtkTreeStore *tree_store, GtkTreeIter *parent_iter) {
	GtkTreeIter child_iter;

	gtk_tree_store_append(tree_store, &child_iter, parent_iter);
	gtk_tree_store_set(
		tree_store,
		&child_iter,
		OBJECTDB_MODEL_COL_NAME, "",
		OBJECTDB_MODEL_COL_VALUE, "",
		OBJECTDB_MODEL_COL_KIND, "",
		OBJECTDB_MODEL_COL_PATH, "",
		OBJECTDB_MODEL_COL_IS_TABLE, FALSE,
		OBJECTDB_MODEL_COL_CHILDREN_LOADED, TRUE,
		OBJECTDB_MODEL_COL_IS_PLACEHOLDER, TRUE,
		-1
	);
}

static void objectdb_tree_model_append_entry(GtkTreeStore *tree_store, GtkTreeIter *parent_iter, const ObjectDBEntry *entry) {
	GtkTreeIter iter;

	gtk_tree_store_append(tree_store, &iter, parent_iter);
	gtk_tree_store_set(
		tree_store,
		&iter,
		OBJECTDB_MODEL_COL_NAME, entry->name,
		OBJECTDB_MODEL_COL_VALUE, entry->value_text,
		OBJECTDB_MODEL_COL_KIND, entry->kind_text,
		OBJECTDB_MODEL_COL_PATH, entry->full_path,
		OBJECTDB_MODEL_COL_IS_TABLE, entry->is_table,
		OBJECTDB_MODEL_COL_CHILDREN_LOADED, FALSE,
		OBJECTDB_MODEL_COL_IS_PLACEHOLDER, FALSE,
		-1
	);

	if (entry->is_table) {
		objectdb_tree_model_append_placeholder(tree_store, &iter);
	}
}

static void objectdb_tree_model_remove_children(GtkTreeStore *tree_store, GtkTreeIter *parent_iter) {
	GtkTreeModel *model = GTK_TREE_MODEL(tree_store);
	GtkTreeIter child_iter;
	gboolean valid = gtk_tree_model_iter_children(model, &child_iter, parent_iter);

	while (valid) {
		valid = gtk_tree_store_remove(tree_store, &child_iter);
	}
}

static gboolean objectdb_tree_model_append_children(GtkTreeStore *tree_store, GtkTreeIter *parent_iter, ObjectDBDocument *document, const char *path, char **error_message) {
	GPtrArray *entries = objectdb_document_get_children(document, path, error_message);

	if (!entries) {
		return FALSE;
	}

	for (guint index = 0; index < entries->len; ++index) {
		const ObjectDBEntry *entry = g_ptr_array_index(entries, index);
		objectdb_tree_model_append_entry(tree_store, parent_iter, entry);
	}

	g_ptr_array_free(entries, TRUE);
	return TRUE;
}

GtkTreeStore *objectdb_tree_model_new(void) {
	return gtk_tree_store_new(
		OBJECTDB_MODEL_COL_COUNT,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN
	);
}

void objectdb_tree_model_clear(GtkTreeStore *tree_store) {
	if (!tree_store) {
		return;
	}

	gtk_tree_store_clear(tree_store);
}

gboolean objectdb_tree_model_populate_root(GtkTreeStore *tree_store, ObjectDBDocument *document, char **error_message) {
	if (!tree_store) {
		if (error_message) {
			*error_message = g_strdup("Missing tree model");
		}
		return FALSE;
	}

	objectdb_tree_model_clear(tree_store);
	return objectdb_tree_model_append_children(tree_store, NULL, document, "root", error_message);
}

gboolean objectdb_tree_model_load_children(GtkTreeStore *tree_store, GtkTreeIter *parent_iter, ObjectDBDocument *document, char **error_message) {
	GtkTreeModel *model = GTK_TREE_MODEL(tree_store);
	gboolean is_table = FALSE;
	gboolean children_loaded = FALSE;
	char *path = NULL;

	if (!tree_store || !parent_iter) {
		if (error_message) {
			*error_message = g_strdup("Invalid tree row");
		}
		return FALSE;
	}

	gtk_tree_model_get(
		model,
		parent_iter,
		OBJECTDB_MODEL_COL_IS_TABLE, &is_table,
		OBJECTDB_MODEL_COL_CHILDREN_LOADED, &children_loaded,
		OBJECTDB_MODEL_COL_PATH, &path,
		-1
	);

	if (!is_table || children_loaded) {
		g_free(path);
		return TRUE;
	}

	objectdb_tree_model_remove_children(tree_store, parent_iter);

	if (!objectdb_tree_model_append_children(tree_store, parent_iter, document, path, error_message)) {
		g_free(path);
		return FALSE;
	}

	gtk_tree_store_set(
		tree_store,
		parent_iter,
		OBJECTDB_MODEL_COL_CHILDREN_LOADED, TRUE,
		-1
	);

	g_free(path);
	return TRUE;
}
