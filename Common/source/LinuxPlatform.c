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

/*
 * LinuxPlatform.c
 * 
 * Linux/GTK4 platform abstraction layer implementation
 * Phase 2: Core window management and platform wrapper functions
 */

#include "frontier.h"
#include "standard.h"
#include "LinuxLand.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

/* Window registry for tracking GTK widgets and their associations */
#define MAX_WINDOWS 256

typedef struct {
    HWND hwnd;
    GtkWidget *widget;
    cairo_surface_t *surface;
    gboolean is_active;
} WindowEntry;

static WindowEntry window_registry[MAX_WINDOWS];
static int window_count = 0;
static guint tick_count_offset = 0;

/* Forward declarations for internal functions */
static WindowEntry* find_window_entry(HWND hwnd);
static WindowEntry* register_window(HWND hwnd);
static void unregister_window(HWND hwnd);

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : find_window_entry()                                       *
 *                                                                          *
 *  PURPOSE    : Find a window entry in the registry                       *
 *                                                                          *
 ****************************************************************************/
static WindowEntry* find_window_entry(HWND hwnd) {
    int i;
    
    for (i = 0; i < window_count; i++) {
        if (window_registry[i].hwnd == hwnd && window_registry[i].is_active) {
            return &window_registry[i];
        }
    }
    
    return NULL;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : register_window()                                         *
 *                                                                          *
 *  PURPOSE    : Register a new window in the registry                     *
 *                                                                          *
 ****************************************************************************/
static WindowEntry* register_window(HWND hwnd) {
    WindowEntry *entry = NULL;
    int i;
    
    /* Look for an inactive slot */
    for (i = 0; i < window_count; i++) {
        if (!window_registry[i].is_active) {
            entry = &window_registry[i];
            break;
        }
    }
    
    /* If no inactive slot found and we have room, use a new one */
    if (entry == NULL && window_count < MAX_WINDOWS) {
        entry = &window_registry[window_count++];
    }
    
    if (entry != NULL) {
        entry->hwnd = hwnd;
        entry->widget = hwnd;
        entry->surface = NULL;
        entry->is_active = TRUE;
    }
    
    return entry;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : unregister_window()                                       *
 *                                                                          *
 *  PURPOSE    : Remove a window from the registry                         *
 *                                                                          *
 ****************************************************************************/
static void unregister_window(HWND hwnd) {
    WindowEntry *entry = find_window_entry(hwnd);
    
    if (entry != NULL) {
        if (entry->surface != NULL) {
            cairo_surface_destroy(entry->surface);
            entry->surface = NULL;
        }
        entry->is_active = FALSE;
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : GetDC()                                                   *
 *                                                                          *
 *  PURPOSE    : Get a Cairo drawing context for a widget                  *
 *                                                                          *
 ****************************************************************************/
cairo_t* GetDC(HWND hwnd) {
    WindowEntry *entry;
    cairo_t *cr;
    
    if (hwnd == NULL) {
        return NULL;
    }
    
    entry = find_window_entry(hwnd);
    if (entry == NULL) {
        entry = register_window(hwnd);
        if (entry == NULL) {
            return NULL;
        }
    }
    
    /* Create a Cairo surface if we don't have one */
    if (entry->surface == NULL) {
        int width = gtk_widget_get_width(hwnd);
        int height = gtk_widget_get_height(hwnd);
        
        entry->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    }
    
    cr = cairo_create(entry->surface);
    return cr;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : ReleaseDC()                                               *
 *                                                                          *
 *  PURPOSE    : Release a Cairo drawing context                           *
 *                                                                          *
 ****************************************************************************/
void ReleaseDC(HWND hwnd, cairo_t *cr) {
    if (cr != NULL) {
        cairo_destroy(cr);
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : InvalidateRect()                                          *
 *                                                                          *
 *  PURPOSE    : Mark a region of a window for repainting                  *
 *                                                                          *
 ****************************************************************************/
BOOL InvalidateRect(HWND hwnd, const GdkRectangle *rect, BOOL erase) {
    if (hwnd == NULL) {
        return FALSE;
    }
    
    if (rect == NULL) {
        gtk_widget_queue_draw(hwnd);
    } else {
        gtk_widget_queue_draw_area(hwnd, rect->x, rect->y, rect->width, rect->height);
    }
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : UpdateWindow()                                            *
 *                                                                          *
 *  PURPOSE    : Force immediate update of a window                        *
 *                                                                          *
 ****************************************************************************/
BOOL UpdateWindow(HWND hwnd) {
    if (hwnd == NULL) {
        return FALSE;
    }
    
    /* Process pending draw events */
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : CreateWindow()                                            *
 *                                                                          *
 *  PURPOSE    : Create a new window (child window in notebook)            *
 *                                                                          *
 ****************************************************************************/
HWND CreateWindow(const char *class_name, const char *title,
                  guint32 style, int x, int y, int width, int height,
                  HWND parent, HMENU menu, HINSTANCE instance, gpointer param) {
    GtkWidget *widget;
    GtkWidget *label;
    
    /* For child windows, create a scrolled window in the notebook */
    if (style & WS_CHILD) {
        GtkWidget *scrolled;
        
        /* Create a scrolled window */
        scrolled = gtk_scrolled_window_new();
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                      (style & WS_HSCROLL) ? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER,
                                      (style & WS_VSCROLL) ? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER);
        
        /* Create the actual content widget - a drawing area for now */
        widget = gtk_drawing_area_new();
        gtk_widget_set_size_request(widget, width, height);
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), widget);
        
        /* Add to notebook if we have one */
        if (mdi_notebook != NULL) {
            label = gtk_label_new(title ? title : "Untitled");
            gtk_notebook_append_page(GTK_NOTEBOOK(mdi_notebook), scrolled, label);
            gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(mdi_notebook), scrolled, TRUE);
            gtk_widget_show(scrolled);
        }
        
        /* Register the window */
        register_window(widget);
        
        return widget;
    } else {
        /* For top-level windows, create a new application window */
        widget = gtk_window_new();
        
        if (title != NULL) {
            gtk_window_set_title(GTK_WINDOW(widget), title);
        }
        
        if (width > 0 && height > 0) {
            gtk_window_set_default_size(GTK_WINDOW(widget), width, height);
        }
        
        register_window(widget);
        
        return widget;
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DestroyWindow()                                           *
 *                                                                          *
 *  PURPOSE    : Destroy a window                                          *
 *                                                                          *
 ****************************************************************************/
BOOL DestroyWindow(HWND hwnd) {
    if (hwnd == NULL) {
        return FALSE;
    }
    
    unregister_window(hwnd);
    
    /* If this is a notebook page, remove it */
    if (mdi_notebook != NULL) {
        GtkWidget *parent = gtk_widget_get_parent(hwnd);
        if (parent != NULL && GTK_IS_SCROLLED_WINDOW(parent)) {
            int page_num = gtk_notebook_page_num(GTK_NOTEBOOK(mdi_notebook), parent);
            if (page_num >= 0) {
                gtk_notebook_remove_page(GTK_NOTEBOOK(mdi_notebook), page_num);
                return TRUE;
            }
        }
    }
    
    /* Otherwise just destroy the widget */
    gtk_window_destroy(GTK_WINDOW(hwnd));
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : ShowWindow()                                              *
 *                                                                          *
 *  PURPOSE    : Show or hide a window                                     *
 *                                                                          *
 ****************************************************************************/
BOOL ShowWindow(HWND hwnd, int cmd) {
    if (hwnd == NULL) {
        return FALSE;
    }
    
    switch (cmd) {
        case SW_HIDE:
            gtk_widget_set_visible(hwnd, FALSE);
            break;
            
        case SW_SHOW:
        case SW_SHOWNOACTIVATE:
            gtk_widget_set_visible(hwnd, TRUE);
            break;
            
        case SW_SHOWMINIMIZED:
            if (GTK_IS_WINDOW(hwnd)) {
                gtk_window_minimize(GTK_WINDOW(hwnd));
            }
            break;
            
        case SW_SHOWMAXIMIZED:
            if (GTK_IS_WINDOW(hwnd)) {
                gtk_window_maximize(GTK_WINDOW(hwnd));
            }
            break;
            
        default:
            break;
    }
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : SetWindowText()                                           *
 *                                                                          *
 *  PURPOSE    : Set window title                                          *
 *                                                                          *
 ****************************************************************************/
BOOL SetWindowText(HWND hwnd, const char *text) {
    GtkWidget *parent;
    int page_num;
    GtkWidget *label;
    
    if (hwnd == NULL || text == NULL) {
        return FALSE;
    }
    
    /* If this is a top-level window */
    if (GTK_IS_WINDOW(hwnd)) {
        gtk_window_set_title(GTK_WINDOW(hwnd), text);
        return TRUE;
    }
    
    /* If this is a notebook page, update the tab label */
    parent = gtk_widget_get_parent(hwnd);
    if (parent != NULL && GTK_IS_SCROLLED_WINDOW(parent) && mdi_notebook != NULL) {
        page_num = gtk_notebook_page_num(GTK_NOTEBOOK(mdi_notebook), parent);
        if (page_num >= 0) {
            label = gtk_label_new(text);
            gtk_notebook_set_tab_label(GTK_NOTEBOOK(mdi_notebook), parent, label);
            return TRUE;
        }
    }
    
    return FALSE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : GetWindowText()                                           *
 *                                                                          *
 *  PURPOSE    : Get window title                                          *
 *                                                                          *
 ****************************************************************************/
int GetWindowText(HWND hwnd, char *buffer, int maxcount) {
    const char *text = NULL;
    GtkWidget *parent;
    int page_num;
    GtkWidget *label;
    
    if (hwnd == NULL || buffer == NULL || maxcount <= 0) {
        return 0;
    }
    
    /* If this is a top-level window */
    if (GTK_IS_WINDOW(hwnd)) {
        text = gtk_window_get_title(GTK_WINDOW(hwnd));
    } else {
        /* If this is a notebook page, get the tab label */
        parent = gtk_widget_get_parent(hwnd);
        if (parent != NULL && GTK_IS_SCROLLED_WINDOW(parent) && mdi_notebook != NULL) {
            page_num = gtk_notebook_page_num(GTK_NOTEBOOK(mdi_notebook), parent);
            if (page_num >= 0) {
                label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(mdi_notebook), parent);
                if (label != NULL && GTK_IS_LABEL(label)) {
                    text = gtk_label_get_text(GTK_LABEL(label));
                }
            }
        }
    }
    
    if (text != NULL) {
        strncpy(buffer, text, maxcount - 1);
        buffer[maxcount - 1] = '\0';
        return strlen(buffer);
    }
    
    buffer[0] = '\0';
    return 0;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : MoveWindow()                                              *
 *                                                                          *
 *  PURPOSE    : Move and resize a window                                  *
 *                                                                          *
 ****************************************************************************/
BOOL MoveWindow(HWND hwnd, int x, int y, int width, int height, BOOL repaint) {
    if (hwnd == NULL) {
        return FALSE;
    }
    
    if (GTK_IS_WINDOW(hwnd)) {
        gtk_window_set_default_size(GTK_WINDOW(hwnd), width, height);
    } else {
        gtk_widget_set_size_request(hwnd, width, height);
    }
    
    if (repaint) {
        gtk_widget_queue_draw(hwnd);
    }
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : SetWindowPos()                                            *
 *                                                                          *
 *  PURPOSE    : Set window position and size                              *
 *                                                                          *
 ****************************************************************************/
BOOL SetWindowPos(HWND hwnd, HWND insert_after, int x, int y,
                  int width, int height, guint flags) {
    if (hwnd == NULL) {
        return FALSE;
    }
    
    if (width > 0 && height > 0) {
        MoveWindow(hwnd, x, y, width, height, TRUE);
    }
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : CreateMenu()                                              *
 *                                                                          *
 *  PURPOSE    : Create a new menu                                         *
 *                                                                          *
 ****************************************************************************/
HMENU CreateMenu(void) {
    return g_menu_new();
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : CreatePopupMenu()                                         *
 *                                                                          *
 *  PURPOSE    : Create a new popup menu                                   *
 *                                                                          *
 ****************************************************************************/
HMENU CreatePopupMenu(void) {
    return g_menu_new();
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : AppendMenu()                                              *
 *                                                                          *
 *  PURPOSE    : Append an item to a menu                                  *
 *                                                                          *
 ****************************************************************************/
BOOL AppendMenu(HMENU menu, guint flags, guint id, const char *label) {
    char action_name[64];
    
    if (menu == NULL || label == NULL) {
        return FALSE;
    }
    
    snprintf(action_name, sizeof(action_name), "app.item%u", id);
    g_menu_append(menu, label, action_name);
    
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DestroyMenu()                                             *
 *                                                                          *
 *  PURPOSE    : Destroy a menu                                            *
 *                                                                          *
 ****************************************************************************/
BOOL DestroyMenu(HMENU menu) {
    if (menu != NULL) {
        g_object_unref(menu);
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : MessageBox()                                              *
 *                                                                          *
 *  PURPOSE    : Display a message box dialog                              *
 *                                                                          *
 ****************************************************************************/
int MessageBox(HWND hwnd, const char *text, const char *caption, guint type) {
    GtkWidget *dialog;
    GtkMessageType msg_type = GTK_MESSAGE_INFO;
    GtkButtonsType buttons = GTK_BUTTONS_OK;
    int result;
    
    /* Determine message type */
    if (type & MB_ICONEXCLAMATION) {
        msg_type = GTK_MESSAGE_WARNING;
    } else if (type & MB_ICONINFORMATION) {
        msg_type = GTK_MESSAGE_INFO;
    }
    
    /* Determine button type */
    if (type & MB_OKCANCEL) {
        buttons = GTK_BUTTONS_OK_CANCEL;
    } else if (type & MB_YESNO) {
        buttons = GTK_BUTTONS_YES_NO;
    }
    
    /* Create and show dialog */
    dialog = gtk_message_dialog_new(hwnd ? GTK_WINDOW(hwnd) : GTK_WINDOW(main_window),
                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   msg_type,
                                   buttons,
                                   "%s",
                                   text);
    
    if (caption != NULL) {
        gtk_window_set_title(GTK_WINDOW(dialog), caption);
    }
    
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_window_destroy(GTK_WINDOW(dialog));
    
    /* Map GTK response to Windows message box result */
    switch (result) {
        case GTK_RESPONSE_OK:
        case GTK_RESPONSE_ACCEPT:
            return IDOK;
        case GTK_RESPONSE_CANCEL:
        case GTK_RESPONSE_REJECT:
            return IDCANCEL;
        case GTK_RESPONSE_YES:
            return IDYES;
        case GTK_RESPONSE_NO:
            return IDNO;
        default:
            return IDCANCEL;
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : OpenClipboard()                                           *
 *                                                                          *
 *  PURPOSE    : Open clipboard for access (no-op on GTK)                  *
 *                                                                          *
 ****************************************************************************/
BOOL OpenClipboard(HWND hwnd) {
    /* GTK clipboard doesn't require explicit opening */
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : CloseClipboard()                                          *
 *                                                                          *
 *  PURPOSE    : Close clipboard (no-op on GTK)                            *
 *                                                                          *
 ****************************************************************************/
BOOL CloseClipboard(void) {
    /* GTK clipboard doesn't require explicit closing */
    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : EmptyClipboard()                                          *
 *                                                                          *
 *  PURPOSE    : Clear clipboard contents                                  *
 *                                                                          *
 ****************************************************************************/
BOOL EmptyClipboard(void) {
    GdkClipboard *clipboard = gdk_display_get_clipboard(gdk_display_get_default());
    
    if (clipboard != NULL) {
        gdk_clipboard_set_text(clipboard, "");
        return TRUE;
    }
    
    return FALSE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : GetClipboardData()                                        *
 *                                                                          *
 *  PURPOSE    : Get data from clipboard                                   *
 *                                                                          *
 ****************************************************************************/
gpointer GetClipboardData(guint format) {
    GdkClipboard *clipboard = gdk_display_get_clipboard(gdk_display_get_default());
    char *text;
    
    if (clipboard == NULL) {
        return NULL;
    }
    
    /* For now, only handle text format */
    text = (char *)gdk_clipboard_read_text_finish(clipboard, NULL, NULL);
    
    return text;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : SetClipboardData()                                        *
 *                                                                          *
 *  PURPOSE    : Set clipboard data                                        *
 *                                                                          *
 ****************************************************************************/
gpointer SetClipboardData(guint format, gpointer data) {
    GdkClipboard *clipboard = gdk_display_get_clipboard(gdk_display_get_default());
    
    if (clipboard == NULL || data == NULL) {
        return NULL;
    }
    
    /* For now, only handle text format */
    gdk_clipboard_set_text(clipboard, (const char *)data);
    
    return data;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : SetTimer()                                                *
 *                                                                          *
 *  PURPOSE    : Create a timer                                            *
 *                                                                          *
 ****************************************************************************/
guint SetTimer(HWND hwnd, guint id, guint interval, GSourceFunc callback) {
    if (callback != NULL) {
        return g_timeout_add(interval, callback, hwnd);
    }
    
    return 0;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : KillTimer()                                               *
 *                                                                          *
 *  PURPOSE    : Destroy a timer                                           *
 *                                                                          *
 ****************************************************************************/
BOOL KillTimer(HWND hwnd, guint id) {
    if (id != 0) {
        g_source_remove(id);
        return TRUE;
    }
    
    return FALSE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : GetTickCount()                                            *
 *                                                                          *
 *  PURPOSE    : Get milliseconds since system start                       *
 *                                                                          *
 ****************************************************************************/
guint32 GetTickCount(void) {
    static guint32 start_time = 0;
    guint32 current_time;
    
    if (start_time == 0) {
        start_time = (guint32)(g_get_monotonic_time() / 1000);
    }
    
    current_time = (guint32)(g_get_monotonic_time() / 1000);
    
    return current_time - start_time;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : Sleep()                                                   *
 *                                                                          *
 *  PURPOSE    : Sleep for specified milliseconds                          *
 *                                                                          *
 ****************************************************************************/
void Sleep(guint milliseconds) {
    usleep(milliseconds * 1000);
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : GetCurrentProcessId()                                     *
 *                                                                          *
 *  PURPOSE    : Get current process ID                                    *
 *                                                                          *
 ****************************************************************************/
pid_t GetCurrentProcessId(void) {
    return getpid();
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : GetOpenFileName()                                         *
 *                                                                          *
 *  PURPOSE    : Show file open dialog                                     *
 *                                                                          *
 ****************************************************************************/
BOOL GetOpenFileName(GtkWindow *parent, char *filename, int maxlen) {
    GtkWidget *dialog;
    gint result;
    
    dialog = gtk_file_chooser_dialog_new("Open File",
                                        parent ? parent : GTK_WINDOW(main_window),
                                        GTK_FILE_CHOOSER_ACTION_OPEN,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Open", GTK_RESPONSE_ACCEPT,
                                        NULL);
    
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        char *selected_filename = gtk_file_chooser_get_filename(chooser);
        
        if (selected_filename != NULL) {
            strncpy(filename, selected_filename, maxlen - 1);
            filename[maxlen - 1] = '\0';
            g_free(selected_filename);
            gtk_window_destroy(GTK_WINDOW(dialog));
            return TRUE;
        }
    }
    
    gtk_window_destroy(GTK_WINDOW(dialog));
    return FALSE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : GetSaveFileName()                                         *
 *                                                                          *
 *  PURPOSE    : Show file save dialog                                     *
 *                                                                          *
 ****************************************************************************/
BOOL GetSaveFileName(GtkWindow *parent, char *filename, int maxlen) {
    GtkWidget *dialog;
    gint result;
    
    dialog = gtk_file_chooser_dialog_new("Save File",
                                        parent ? parent : GTK_WINDOW(main_window),
                                        GTK_FILE_CHOOSER_ACTION_SAVE,
                                        "_Cancel", GTK_RESPONSE_CANCEL,
                                        "_Save", GTK_RESPONSE_ACCEPT,
                                        NULL);
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        char *selected_filename = gtk_file_chooser_get_filename(chooser);
        
        if (selected_filename != NULL) {
            strncpy(filename, selected_filename, maxlen - 1);
            filename[maxlen - 1] = '\0';
            g_free(selected_filename);
            gtk_window_destroy(GTK_WINDOW(dialog));
            return TRUE;
        }
    }
    
    gtk_window_destroy(GTK_WINDOW(dialog));
    return FALSE;
}
