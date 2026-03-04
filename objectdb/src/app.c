#include "app.h"
#include "window.h"

#include <gtk/gtk.h>

static void objectdb_app_activate(GtkApplication *app, gpointer user_data) {
	(void)user_data;
	ObjectDBWindow *window = objectdb_window_new(app);
	objectdb_window_present(window);
}

int objectdb_app_run(int argc, char **argv) {
	GtkApplication *app = gtk_application_new("com.tedchoward.objectdb", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(objectdb_app_activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
