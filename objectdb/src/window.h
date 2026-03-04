#ifndef OBJECTDB_WINDOW_H
#define OBJECTDB_WINDOW_H

#include <gtk/gtk.h>

typedef struct ObjectDBWindow ObjectDBWindow;

ObjectDBWindow *objectdb_window_new(GtkApplication *app);
void objectdb_window_present(ObjectDBWindow *window);

#endif
