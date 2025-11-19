/*	$Id$    */

/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

#include "frontier.h"
#include "standard.h"

#include "LinuxLand.h"
#include "dialogs.h"
#include "file.h"
#include "font.h"
#include "kb.h"
#include "menu.h"
#include "mouse.h"
#include "quickdraw.h"
#include "scrap.h"
#include "strings.h"
#include "frontierwindows.h"
#include "cancoon.h"
#include "cancooninternal.h"
#include "shell.h"
#include "shellprivate.h"
#include "shellmenu.h"
#include "lang.h"
#include "launch.h"
#include "frontierstart.h"

/* Global variables */
GtkApplication *gtk_app = NULL;
GtkWidget *main_window = NULL;
GtkWidget *mdi_notebook = NULL;
GtkWidget *status_bar = NULL;
HINSTANCE shell_instance = NULL;

static gboolean fl_instance_initialized = FALSE;

/* Application ID */
#define APPLICATION_ID "org.userland.opml"

/* Forward declarations */
static void on_activate(GApplication *app, gpointer user_data);
static void on_startup(GApplication *app, gpointer user_data);
static void on_shutdown(GApplication *app, gpointer user_data);
static gboolean on_window_delete_event(GtkWidget *widget, gpointer data);
static void on_about_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void on_quit_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void on_new_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void on_open_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void on_save_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);

/* Action entries */
static GActionEntry app_entries[] = {
    { "new", on_new_action, NULL, NULL, NULL },
    { "open", on_open_action, NULL, NULL, NULL },
    { "save", on_save_action, NULL, NULL, NULL },
    { "about", on_about_action, NULL, NULL, NULL },
    { "quit", on_quit_action, NULL, NULL, NULL }
};

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : InitializeApplication()                                   *
 *                                                                          *
 *  PURPOSE    : Register application with GTK and set up basic structure  *
 *                                                                          *
 *  RETURNS    : TRUE if successful, FALSE otherwise                       *
 *                                                                          *
 ****************************************************************************/
BOOL InitializeApplication(void) {
    /* Create GTK application */
    gtk_app = gtk_application_new(APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
    
    if (gtk_app == NULL) {
        g_printerr("Failed to create GTK application\n");
        return FALSE;
    }
    
    /* Connect signals */
    g_signal_connect(gtk_app, "startup", G_CALLBACK(on_startup), NULL);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(gtk_app, "shutdown", G_CALLBACK(on_shutdown), NULL);
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : InitializeInstance()                                      *
 *                                                                          *
 *  PURPOSE    : Create main window and initialize Frontier core           *
 *                                                                          *
 *  RETURNS    : TRUE if successful, FALSE otherwise                       *
 *                                                                          *
 ****************************************************************************/
BOOL InitializeInstance(int argc, char **argv) {
    /* Initialize Frontier shell */
    if (!shellinit()) {
        g_printerr("Failed to initialize shell\n");
        return FALSE;
    }
    
    grabthreadglobals();
    
    /* Start Frontier core */
    if (!frontierstart()) {
        shellshutdown();
        releasethreadglobals();
        return FALSE;
    }
    
    releasethreadglobals();
    fl_instance_initialized = TRUE;
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : on_startup()                                              *
 *                                                                          *
 *  PURPOSE    : Called when application starts - set up actions and menu  *
 *                                                                          *
 ****************************************************************************/
static void on_startup(GApplication *app, gpointer user_data) {
    GMenu *menubar;
    GMenu *file_menu;
    GMenu *help_menu;
    
    /* Add application actions */
    g_action_map_add_action_entries(G_ACTION_MAP(app),
                                   app_entries,
                                   G_N_ELEMENTS(app_entries),
                                   app);
    
    /* Create menubar */
    menubar = g_menu_new();
    
    /* File menu */
    file_menu = g_menu_new();
    g_menu_append(file_menu, "New", "app.new");
    g_menu_append(file_menu, "Open", "app.open");
    g_menu_append(file_menu, "Save", "app.save");
    g_menu_append(file_menu, "Quit", "app.quit");
    g_menu_append_submenu(menubar, "File", G_MENU_MODEL(file_menu));
    
    /* Help menu */
    help_menu = g_menu_new();
    g_menu_append(help_menu, "About", "app.about");
    g_menu_append_submenu(menubar, "Help", G_MENU_MODEL(help_menu));
    
    gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(menubar));
    
    g_object_unref(file_menu);
    g_object_unref(help_menu);
    g_object_unref(menubar);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : on_activate()                                             *
 *                                                                          *
 *  PURPOSE    : Called when application is activated - create main window *
 *                                                                          *
 ****************************************************************************/
static void on_activate(GApplication *app, gpointer user_data) {
    GtkWidget *vbox;
    GtkWidget *scrolled_window;
    
    /* Don't create multiple windows */
    if (main_window != NULL) {
        gtk_window_present(GTK_WINDOW(main_window));
        return;
    }
    
    /* Create main application window */
    main_window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(main_window), "OPML Editor");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
    
    /* Connect delete event */
    g_signal_connect(main_window, "close-request",
                    G_CALLBACK(on_window_delete_event), NULL);
    
    /* Create vertical box for layout */
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(main_window), vbox);
    
    /* Create notebook for MDI */
    mdi_notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(mdi_notebook), TRUE);
    gtk_widget_set_vexpand(mdi_notebook, TRUE);
    gtk_box_append(GTK_BOX(vbox), mdi_notebook);
    
    /* Create status bar */
    status_bar = gtk_statusbar_new();
    gtk_box_append(GTK_BOX(vbox), status_bar);
    
    /* Show window */
    gtk_widget_show(main_window);
    
    /* Initialize Frontier instance if not already done */
    if (!fl_instance_initialized) {
        char *argv_dummy[] = { "opml", NULL };
        if (!InitializeInstance(1, argv_dummy)) {
            g_printerr("Failed to initialize Frontier instance\n");
            gtk_window_destroy(GTK_WINDOW(main_window));
            main_window = NULL;
        }
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : on_shutdown()                                             *
 *                                                                          *
 *  PURPOSE    : Called when application shuts down - cleanup              *
 *                                                                          *
 ****************************************************************************/
static void on_shutdown(GApplication *app, gpointer user_data) {
    if (fl_instance_initialized) {
        grabthreadglobals();
        shellshutdown();
        releasethreadglobals();
        fl_instance_initialized = FALSE;
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : on_window_delete_event()                                  *
 *                                                                          *
 *  PURPOSE    : Handle window close request                               *
 *                                                                          *
 ****************************************************************************/
static gboolean on_window_delete_event(GtkWidget *widget, gpointer data) {
    gboolean can_close;
    
    grabthreadglobals();
    keyboardpeek(&keyboardstatus);
    can_close = shellquit();
    releasethreadglobals();
    
    return !can_close; /* Return TRUE to prevent closing */
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : Action callbacks                                          *
 *                                                                          *
 ****************************************************************************/
static void on_new_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    grabthreadglobals();
    /* TODO: Implement new file action */
    releasethreadglobals();
}

static void on_open_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWidget *dialog;
    GtkFileChooserAction file_action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    
    dialog = gtk_file_chooser_dialog_new("Open File",
                                        GTK_WINDOW(main_window),
                                        file_action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Open",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
    
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        
        filename = gtk_file_chooser_get_filename(chooser);
        
        grabthreadglobals();
        /* TODO: Open file using filename */
        releasethreadglobals();
        
        g_free(filename);
    }
    
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_save_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    grabthreadglobals();
    /* TODO: Implement save action */
    releasethreadglobals();
}

static void on_about_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWidget *about_dialog;
    
    about_dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "OPML Editor");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), "10.2.0");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog),
                                  "Copyright © 1992-2004 UserLand Software, Inc.");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),
                                 "High performance Web content management,\n"
                                 "object database, system-level and Internet\n"
                                 "scripting environment");
    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(about_dialog), GTK_LICENSE_GPL_2_0);
    
    gtk_window_set_transient_for(GTK_WINDOW(about_dialog), GTK_WINDOW(main_window));
    gtk_window_set_modal(GTK_WINDOW(about_dialog), TRUE);
    gtk_widget_show(about_dialog);
}

static void on_quit_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    gboolean can_quit;
    
    grabthreadglobals();
    keyboardpeek(&keyboardstatus);
    can_quit = shellquit();
    releasethreadglobals();
    
    if (can_quit) {
        g_application_quit(G_APPLICATION(gtk_app));
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : ShellMainEventLoop()                                      *
 *                                                                          *
 *  PURPOSE    : Main event loop - GTK handles this                        *
 *                                                                          *
 ****************************************************************************/
void ShellMainEventLoop(void) {
    /* GTK's main loop is run by g_application_run() */
    /* This function is here for compatibility with other platforms */
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WinProcessMouseEvent()                                    *
 *                                                                          *
 *  PURPOSE    : Process mouse events and forward to shell                 *
 *                                                                          *
 ****************************************************************************/
BOOL WinProcessMouseEvent(HWND hwnd, guint msg, guint button, gdouble x, gdouble y) {
    EventRecord ev;
    
    ev.hwnd = hwnd;
    ev.winmsg = msg;
    ev.what = mouseDown;
    ev.message = (long)hwnd;
    ev.when = g_get_monotonic_time() / 1000; /* Convert to milliseconds */
    ev.where.h = (short)x;
    ev.where.v = (short)y;
    ev.modifiers = 0; /* TODO: Get modifier keys from event */
    ev.part = HTCLIENT;
    
    return shellprocessevent(&ev);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WinProcessKeyboardEvent()                                 *
 *                                                                          *
 *  PURPOSE    : Process keyboard events and forward to shell              *
 *                                                                          *
 ****************************************************************************/
BOOL WinProcessKeyboardEvent(HWND hwnd, guint msg, guint keyval, guint state) {
    EventRecord ev;
    
    ev.hwnd = hwnd;
    ev.winmsg = msg;
    ev.what = keyDown;
    ev.message = keyval;
    ev.when = g_get_monotonic_time() / 1000;
    ev.where.h = 0;
    ev.where.v = 0;
    ev.modifiers = state;
    ev.part = HTCLIENT;
    
    return shellprocessevent(&ev);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WinProcessPaintEvent()                                    *
 *                                                                          *
 *  PURPOSE    : Process paint/draw events and forward to shell            *
 *                                                                          *
 ****************************************************************************/
BOOL WinProcessPaintEvent(HWND hwnd) {
    EventRecord ev;
    
    ev.hwnd = hwnd;
    ev.winmsg = WM_PAINT;
    ev.what = updateEvt;
    ev.message = (long)hwnd;
    ev.when = g_get_monotonic_time() / 1000;
    ev.where.h = 0;
    ev.where.v = 0;
    ev.modifiers = 0;
    ev.part = HTCLIENT;
    
    return shellprocessevent(&ev);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WinProcessCommandEvent()                                  *
 *                                                                          *
 *  PURPOSE    : Process command/menu events and forward to shell          *
 *                                                                          *
 ****************************************************************************/
BOOL WinProcessCommandEvent(HWND hwnd, guint msg, guint id) {
    EventRecord ev;
    
    ev.hwnd = hwnd;
    ev.winmsg = msg;
    ev.what = menuEvt;
    ev.message = (long)hwnd;
    ev.when = g_get_monotonic_time() / 1000;
    ev.where.h = 0;
    ev.where.v = 0;
    ev.modifiers = (((id / 100) * 100) << 16) + id;
    ev.part = HTCLIENT;
    
    keyboardpeek(&keyboardstatus);
    
    return shellprocessevent(&ev);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WinProcessScrollEvent()                                   *
 *                                                                          *
 *  PURPOSE    : Process scroll events and forward to shell                *
 *                                                                          *
 ****************************************************************************/
BOOL WinProcessScrollEvent(HWND hwnd, guint msg, gdouble value, gboolean isvert) {
    EventRecord ev;
    
    ev.hwnd = hwnd;
    ev.winmsg = msg;
    ev.what = scrollEvt;
    ev.message = (long)hwnd;
    ev.when = g_get_monotonic_time() / 1000;
    ev.where.h = (short)value;
    ev.where.v = 0;
    ev.modifiers = isvert ? 1 : 0;
    ev.part = HTCLIENT;
    
    keyboardpeek(&keyboardstatus);
    
    return shellprocessevent(&ev);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WinProcessActivateEvent()                                 *
 *                                                                          *
 *  PURPOSE    : Process window activation events and forward to shell     *
 *                                                                          *
 ****************************************************************************/
BOOL WinProcessActivateEvent(HWND hwnd, guint msg, gboolean activate) {
    EventRecord ev;
    
    ev.hwnd = hwnd;
    ev.winmsg = msg;
    ev.what = activateEvt;
    ev.message = (long)hwnd;
    ev.when = g_get_monotonic_time() / 1000;
    ev.where.h = 0;
    ev.where.v = 0;
    ev.modifiers = activate ? activeFlag : 0;
    ev.part = HTCLIENT;
    
    return shellprocessevent(&ev);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : main()                                                    *
 *                                                                          *
 *  PURPOSE    : Application entry point                                   *
 *                                                                          *
 ****************************************************************************/
int main(int argc, char **argv) {
    int status;
    
    /* Initialize application */
    if (!InitializeApplication()) {
        return 1;
    }
    
    /* Run the GTK application */
    status = g_application_run(G_APPLICATION(gtk_app), argc, argv);
    
    /* Cleanup */
    g_object_unref(gtk_app);
    
    return status;
}
