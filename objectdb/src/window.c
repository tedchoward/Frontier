#include "window.h"

#include <string.h>

enum {
	COL_NAME = 0,
	COL_VALUE,
	COL_KIND,
	COL_COUNT
};

struct ObjectDBWindow {
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *tree_view;
	GtkTreeStore *tree_store;
};

static void objectdb_window_destroy(gpointer data) {
	ObjectDBWindow *window = (ObjectDBWindow *)data;
	g_free(window);
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

static GtkWidget *objectdb_window_create_tree_view(ObjectDBWindow *window) {
	window->tree_store = gtk_tree_store_new(
		COL_COUNT,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING
	);

	GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(window->tree_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), TRUE);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
		"Name",
		renderer,
		"text", COL_NAME,
		NULL
	);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
		"Value",
		renderer,
		"text", COL_VALUE,
		NULL
	);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(
		"Kind",
		renderer,
		"text", COL_KIND,
		NULL
	);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

	return tree_view;
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
		objectdb_window_update_title(window, filename);
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

	return menu_bar;
}

ObjectDBWindow *objectdb_window_new(GtkApplication *app) {
	ObjectDBWindow *window = g_new0(ObjectDBWindow, 1);
	window->app = app;
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
