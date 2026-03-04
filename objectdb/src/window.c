#include "document.h"
#include "tree_model.h"
#include "value_format.h"
#include "window.h"

#include <stdint.h>

struct ObjectDBWindow {
	GtkApplication *app;
	ObjectDBDocument *document;
	GtkWidget *window;
	GtkWidget *tree_view;
	GtkTreeStore *tree_store;
};

static void objectdb_window_show_error(ObjectDBWindow *window, const char *message);

static char *objectdb_window_prompt_for_name(ObjectDBWindow *window) {
	GtkWidget *dialog = NULL;
	GtkWidget *content = NULL;
	GtkWidget *entry = NULL;
	char *name = NULL;

	if (!window) {
		return NULL;
	}

	dialog = gtk_dialog_new_with_buttons(
		"New Item",
		GTK_WINDOW(window->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_OK", GTK_RESPONSE_OK,
		NULL
	);

	content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	entry = gtk_entry_new();
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
	gtk_box_pack_start(GTK_BOX(content), entry, TRUE, TRUE, 8);
	gtk_widget_show_all(content);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
		if (text) {
			name = g_strdup(text);
			g_strstrip(name);
		}
	}

	gtk_widget_destroy(dialog);
	return name;
}

static void objectdb_window_update_table_count(ObjectDBWindow *window, GtkTreeIter *table_iter) {
	GtkTreeModel *model = NULL;
	int count = 0;
	char *count_text = NULL;

	if (!window || !window->tree_store || !table_iter) {
		return;
	}

	model = GTK_TREE_MODEL(window->tree_store);
	count = gtk_tree_model_iter_n_children(model, table_iter);
	if (count == 1) {
		count_text = g_strdup("1 item");
	} else {
		count_text = g_strdup_printf("%d items", count);
	}

	gtk_tree_store_set(window->tree_store, table_iter, OBJECTDB_MODEL_COL_VALUE, count_text, -1);
	g_free(count_text);
}

static void objectdb_window_table_new_scalar(GtkWidget *widget, gpointer user_data) {
	ObjectDBWindow *window = (ObjectDBWindow *)user_data;
	GtkTreeSelection *selection = NULL;
	GtkTreeModel *model = NULL;
	GtkTreeIter table_iter;
	gboolean has_selection = FALSE;
	gboolean is_table = FALSE;
	gboolean is_placeholder = FALSE;
	gboolean children_loaded = FALSE;
	char *table_path = NULL;
	char *new_name = NULL;
	char *new_full_path = NULL;
	char *error_message = NULL;
	odbValueType type = (odbValueType)(uintptr_t)g_object_get_data(G_OBJECT(widget), "odb-type");

	if (!window || !window->tree_view || !window->tree_store || !objectdb_document_is_open(window->document)) {
		return;
	}

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(window->tree_view));
	has_selection = gtk_tree_selection_get_selected(selection, &model, &table_iter);
	if (!has_selection) {
		objectdb_window_show_error(window, "Select a destination table first");
		return;
	}

	gtk_tree_model_get(
		model,
		&table_iter,
		OBJECTDB_MODEL_COL_PATH, &table_path,
		OBJECTDB_MODEL_COL_IS_TABLE, &is_table,
		OBJECTDB_MODEL_COL_IS_PLACEHOLDER, &is_placeholder,
		OBJECTDB_MODEL_COL_CHILDREN_LOADED, &children_loaded,
		-1
	);

	if (!is_table || is_placeholder || !table_path || !table_path[0]) {
		objectdb_window_show_error(window, "Select a table row before creating a new item");
		g_free(table_path);
		return;
	}

	new_name = objectdb_window_prompt_for_name(window);
	if (!new_name || !new_name[0]) {
		g_free(new_name);
		g_free(table_path);
		return;
	}

	if (!objectdb_document_create_scalar(window->document, table_path, new_name, type, &new_full_path, &error_message)) {
		objectdb_window_show_error(window, error_message ? error_message : "Failed to create value");
		g_free(error_message);
		g_free(new_name);
		g_free(table_path);
		return;
	}

	if (children_loaded) {
		GtkTreeIter child_iter;
		char *kind_text = objectdb_value_format_kind(type);
		gtk_tree_store_append(window->tree_store, &child_iter, &table_iter);
		gtk_tree_store_set(
			window->tree_store,
			&child_iter,
			OBJECTDB_MODEL_COL_NAME, new_name,
			OBJECTDB_MODEL_COL_VALUE, "",
			OBJECTDB_MODEL_COL_KIND, kind_text ? kind_text : "",
			OBJECTDB_MODEL_COL_PATH, new_full_path,
			OBJECTDB_MODEL_COL_IS_TABLE, FALSE,
			OBJECTDB_MODEL_COL_CHILDREN_LOADED, TRUE,
			OBJECTDB_MODEL_COL_IS_PLACEHOLDER, FALSE,
			-1
		);
		objectdb_window_update_table_count(window, &table_iter);
		g_free(kind_text);
	}

	g_free(new_full_path);
	g_free(new_name);
	g_free(table_path);
}

static void objectdb_window_destroy(gpointer data) {
	ObjectDBWindow *window = (ObjectDBWindow *)data;
	objectdb_document_free(window->document);
	g_free(window);
}

static void objectdb_window_show_error(ObjectDBWindow *window, const char *message) {
	GtkWidget *dialog = NULL;

	if (!window || !message) {
		return;
	}

	dialog = gtk_message_dialog_new(
		GTK_WINDOW(window->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		"%s",
		message
	);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

static void objectdb_window_update_title(ObjectDBWindow *window, const char *path) {
	if (!window) {
		return;
	}
	if (!path || !path[0]) {
		gtk_window_set_title(GTK_WINDOW(window->window), "ObjectDB");
		return;
	}
	char *basename = g_path_get_basename(path);
	char *title = g_strdup_printf("ObjectDB - %s", basename);
	gtk_window_set_title(GTK_WINDOW(window->window), title);
	g_free(title);
	g_free(basename);
}

static void objectdb_window_value_edited(GtkCellRendererText *renderer, gchar *path_string, gchar *new_text, gpointer user_data) {
	(void)renderer;

	ObjectDBWindow *window = (ObjectDBWindow *)user_data;
	GtkTreeModel *model = NULL;
	GtkTreePath *tree_path = NULL;
	GtkTreeIter iter;
	gboolean is_table = FALSE;
	gboolean is_placeholder = FALSE;
	char *full_path = NULL;
	char *updated_text = NULL;
	char *error_message = NULL;

	if (!window || !window->tree_store || !objectdb_document_is_open(window->document) || !path_string) {
		return;
	}

	model = GTK_TREE_MODEL(window->tree_store);
	tree_path = gtk_tree_path_new_from_string(path_string);
	if (!tree_path) {
		return;
	}
	if (!gtk_tree_model_get_iter(model, &iter, tree_path)) {
		gtk_tree_path_free(tree_path);
		return;
	}

	gtk_tree_model_get(
		model,
		&iter,
		OBJECTDB_MODEL_COL_PATH, &full_path,
		OBJECTDB_MODEL_COL_IS_TABLE, &is_table,
		OBJECTDB_MODEL_COL_IS_PLACEHOLDER, &is_placeholder,
		-1
	);

	if (!is_table && !is_placeholder && full_path && full_path[0]) {
		if (objectdb_document_set_value(window->document, full_path, new_text, &updated_text, &error_message)) {
			gtk_tree_store_set(window->tree_store, &iter, OBJECTDB_MODEL_COL_VALUE, updated_text ? updated_text : "", -1);
		} else {
			objectdb_window_show_error(window, error_message ? error_message : "Failed to update value");
		}
	}

	g_free(error_message);
	g_free(updated_text);
	g_free(full_path);
	gtk_tree_path_free(tree_path);
}

static GtkWidget *objectdb_window_create_tree_view(ObjectDBWindow *window) {
	window->tree_store = objectdb_tree_model_new();

	GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(window->tree_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), TRUE);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
		"Name",
		renderer,
		"text", OBJECTDB_MODEL_COL_NAME,
		NULL
	);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_min_width(column, 160);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(renderer, "edited", G_CALLBACK(objectdb_window_value_edited), window);
	column = gtk_tree_view_column_new_with_attributes(
		"Value",
		renderer,
		"text", OBJECTDB_MODEL_COL_VALUE,
		NULL
	);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_min_width(column, 220);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
		"Kind",
		renderer,
		"text", OBJECTDB_MODEL_COL_KIND,
		NULL
	);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_min_width(column, 100);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	return tree_view;
}

static gboolean objectdb_window_test_expand_row(GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path, gpointer user_data) {
	(void)tree_view;
	(void)path;

	ObjectDBWindow *window = (ObjectDBWindow *)user_data;
	char *error_message = NULL;

	if (!window || !objectdb_document_is_open(window->document)) {
		return FALSE;
	}

	if (!objectdb_tree_model_load_children(window->tree_store, iter, window->document, &error_message)) {
		objectdb_window_show_error(window, error_message ? error_message : "Failed to load table contents");
		g_free(error_message);
	}

	return FALSE;
}

static void objectdb_window_show_open_dialog(GtkWidget *widget, gpointer user_data) {
	(void)widget;
	ObjectDBWindow *window = (ObjectDBWindow *)user_data;
	if (!window) {
		return;
	}

	GtkWidget *dialog = gtk_file_chooser_dialog_new(
		"Open Root File",
		GTK_WINDOW(window->window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Open", GTK_RESPONSE_ACCEPT,
		NULL
	);

	GtkFileFilter *root_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(root_filter, "Frontier Root Files");
	gtk_file_filter_add_pattern(root_filter, "*.root");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), root_filter);

	GtkFileFilter *all_filter = gtk_file_filter_new();
	gtk_file_filter_set_name(all_filter, "All Files");
	gtk_file_filter_add_pattern(all_filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		char *error_message = NULL;

		if (!objectdb_document_open(window->document, filename, &error_message)) {
			objectdb_tree_model_clear(window->tree_store);
			objectdb_window_update_title(window, NULL);
			objectdb_window_show_error(window, error_message ? error_message : "Failed to open file");
			g_free(error_message);
		} else if (!objectdb_tree_model_populate_root(window->tree_store, window->document, &error_message)) {
			objectdb_document_close(window->document);
			objectdb_tree_model_clear(window->tree_store);
			objectdb_window_update_title(window, NULL);
			objectdb_window_show_error(window, error_message ? error_message : "Failed to read database contents");
			g_free(error_message);
		} else {
			objectdb_window_update_title(window, filename);
		}

		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

static void objectdb_window_quit(GtkWidget *widget, gpointer user_data) {
	(void)widget;
	ObjectDBWindow *window = (ObjectDBWindow *)user_data;
	if (!window) {
		return;
	}
	g_application_quit(G_APPLICATION(window->app));
}

static GtkWidget *objectdb_window_create_menu_bar(ObjectDBWindow *window) {
	GtkWidget *menu_bar = gtk_menu_bar_new();

	GtkWidget *file_item = gtk_menu_item_new_with_mnemonic("_File");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);

	GtkWidget *file_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

	GtkWidget *open_item = gtk_menu_item_new_with_mnemonic("_Open...");
	g_signal_connect(open_item, "activate", G_CALLBACK(objectdb_window_show_open_dialog), window);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);

	GtkWidget *separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), separator);

	GtkWidget *quit_item = gtk_menu_item_new_with_mnemonic("_Quit");
	g_signal_connect(quit_item, "activate", G_CALLBACK(objectdb_window_quit), window);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);

	GtkWidget *table_item = gtk_menu_item_new_with_mnemonic("_Table");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), table_item);

	GtkWidget *table_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(table_item), table_menu);

	GtkWidget *new_item = gtk_menu_item_new_with_mnemonic("_New");
	gtk_menu_shell_append(GTK_MENU_SHELL(table_menu), new_item);

	GtkWidget *new_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_item), new_menu);

	GtkWidget *scalar_item = gtk_menu_item_new_with_label("Scalar");
	gtk_menu_shell_append(GTK_MENU_SHELL(new_menu), scalar_item);

	GtkWidget *scalar_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(scalar_item), scalar_menu);

	struct {
		const char *label;
		odbValueType type;
	} scalar_items[] = {
		{"Address", addressvaluetype},
		{"Boolean", booleanvaluetype},
		{"Character", charvaluetype},
		{"Date", datevaluetype},
		{"Direction", directionvaluetype},
		{"Float", singlevaluetype},
		{"Long", longvaluetype},
		{"String", stringvaluetype}
	};

	for (guint i = 0; i < G_N_ELEMENTS(scalar_items); ++i) {
		GtkWidget *item = gtk_menu_item_new_with_label(scalar_items[i].label);
		g_object_set_data(G_OBJECT(item), "odb-type", (gpointer)(uintptr_t)scalar_items[i].type);
		g_signal_connect(item, "activate", G_CALLBACK(objectdb_window_table_new_scalar), window);
		gtk_menu_shell_append(GTK_MENU_SHELL(scalar_menu), item);
	}

	return menu_bar;
}

ObjectDBWindow *objectdb_window_new(GtkApplication *app) {
	ObjectDBWindow *window = g_new0(ObjectDBWindow, 1);
	window->app = app;
	window->document = objectdb_document_new();
	window->window = gtk_application_window_new(app);

	g_object_set_data_full(G_OBJECT(window->window), "objectdb-window", window, objectdb_window_destroy);

	gtk_window_set_default_size(GTK_WINDOW(window->window), 960, 640);
	objectdb_window_update_title(window, NULL);

	GtkWidget *root_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window->window), root_box);

	GtkWidget *menu_bar = objectdb_window_create_menu_bar(window);
	gtk_box_pack_start(GTK_BOX(root_box), menu_bar, FALSE, FALSE, 0);

	GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(root_box), scroller, TRUE, TRUE, 0);

	window->tree_view = objectdb_window_create_tree_view(window);
	g_signal_connect(window->tree_view, "test-expand-row", G_CALLBACK(objectdb_window_test_expand_row), window);
	gtk_container_add(GTK_CONTAINER(scroller), window->tree_view);

	return window;
}

void objectdb_window_present(ObjectDBWindow *window) {
	if (!window) {
		return;
	}
	gtk_widget_show_all(window->window);
	gtk_window_present(GTK_WINDOW(window->window));
}
