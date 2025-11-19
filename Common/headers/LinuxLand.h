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
 * LinuxLand.h
 * 
 * Linux/GTK4 platform-specific definitions and declarations for Frontier
 */

#ifndef LINUXLAND_H
#define LINUXLAND_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>
#include <glib.h>

/* Window handle type - maps to GtkWidget pointer */
typedef GtkWidget* HWND;

/* Instance handle type - not really used on Linux but kept for compatibility */
typedef void* HINSTANCE;

/* Menu handle type */
typedef GMenu* HMENU;

/* Device context handle - maps to Cairo context */
typedef cairo_t* HDC;

/* Icon handle */
typedef GdkPixbuf* HICON;

/* Cursor handle */
typedef GdkCursor* HCURSOR;

/* Accelerator handle */
typedef GtkAccelGroup* HACCEL;

/* Process ID type */
typedef pid_t typrocessid;

/* Boolean type compatibility */
#ifndef BOOL
#define BOOL gboolean
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Window messages - GTK signal equivalents */
#define WM_CREATE           0x0001
#define WM_DESTROY          0x0002
#define WM_MOVE             0x0003
#define WM_SIZE             0x0005
#define WM_ACTIVATE         0x0006
#define WM_SETFOCUS         0x0007
#define WM_KILLFOCUS        0x0008
#define WM_PAINT            0x000F
#define WM_CLOSE            0x0010
#define WM_QUIT             0x0012
#define WM_KEYDOWN          0x0100
#define WM_KEYUP            0x0101
#define WM_CHAR             0x0102
#define WM_SYSKEYDOWN       0x0104
#define WM_SYSKEYUP         0x0105
#define WM_COMMAND          0x0111
#define WM_SYSCOMMAND       0x0112
#define WM_TIMER            0x0113
#define WM_HSCROLL          0x0114
#define WM_VSCROLL          0x0115
#define WM_INITMENU         0x0116
#define WM_MOUSEMOVE        0x0200
#define WM_LBUTTONDOWN      0x0201
#define WM_LBUTTONUP        0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN      0x0204
#define WM_RBUTTONUP        0x0205
#define WM_RBUTTONDBLCLK    0x0206
#define WM_MBUTTONDOWN      0x0207
#define WM_MBUTTONUP        0x0208
#define WM_MBUTTONDBLCLK    0x0209

/* Non-client (NC) messages */
#define WM_NCLBUTTONDOWN    0x00A1
#define WM_NCRBUTTONDOWN    0x00A4
#define WM_NCMBUTTONDOWN    0x00A7
#define WM_NCACTIVATE       0x0086

/* MDI messages */
#define WM_MDIACTIVATE      0x0222
#define WM_MDICREATE        0x0220
#define WM_MDIDESTROY       0x0221

/* Clipboard messages */
#define WM_DRAWCLIPBOARD    0x0308
#define WM_CHANGECBCHAIN    0x030D
#define WM_RENDERFORMAT     0x0305
#define WM_RENDERALLFORMATS 0x0306
#define WM_DESTROYCLIPBOARD 0x0307

/* Application-specific messages */
#define WM_USER             0x0400

/* Virtual key codes */
#define VK_LEFT             GDK_KEY_Left
#define VK_RIGHT            GDK_KEY_Right
#define VK_UP               GDK_KEY_Up
#define VK_DOWN             GDK_KEY_Down
#define VK_PRIOR            GDK_KEY_Page_Up
#define VK_NEXT             GDK_KEY_Page_Down
#define VK_HOME             GDK_KEY_Home
#define VK_END              GDK_KEY_End
#define VK_INSERT           GDK_KEY_Insert
#define VK_DELETE           GDK_KEY_Delete
#define VK_F1               GDK_KEY_F1
#define VK_F2               GDK_KEY_F2
#define VK_RETURN           GDK_KEY_Return
#define VK_ESCAPE           GDK_KEY_Escape
#define VK_TAB              GDK_KEY_Tab
#define VK_BACK             GDK_KEY_BackSpace

/* Window styles */
#define WS_OVERLAPPEDWINDOW 0x00CF0000
#define WS_CHILD            0x40000000
#define WS_VISIBLE          0x10000000
#define WS_CLIPCHILDREN     0x02000000
#define WS_VSCROLL          0x00200000
#define WS_HSCROLL          0x00100000

/* Show window commands */
#define SW_HIDE             0
#define SW_SHOW             1
#define SW_SHOWNOACTIVATE   2
#define SW_SHOWMINIMIZED    3
#define SW_SHOWMAXIMIZED    4

/* Hit test codes */
#define HTCLIENT            1
#define HTNOWHERE           0
#define HTERROR             (-2)
#define HTTRANSPARENT       (-1)

/* System commands */
#define SC_MAXIMIZE         0xF030
#define SC_MINIMIZE         0xF020
#define SC_RESTORE          0xF120
#define SC_CLOSE            0xF060

/* Message box types */
#define MB_OK               0x00000000
#define MB_OKCANCEL         0x00000001
#define MB_YESNO            0x00000004
#define MB_ICONEXCLAMATION  0x00000030
#define MB_ICONINFORMATION  0x00000040
#define MB_APPLMODAL        0x00000000

/* Message box return values */
#define IDOK                1
#define IDCANCEL            2
#define IDYES               6
#define IDNO                7

/* Color constants */
#define COLOR_WINDOW        5
#define COLOR_BTNFACE       15
#define COLOR_APPWORKSPACE  12

/* Macros for compatibility */
#define LOWORD(l)           ((unsigned short)(l))
#define HIWORD(l)           ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))
#define MAKELONG(a,b)       ((long)(((unsigned short)(a)) | ((unsigned long)((unsigned short)(b))) << 16))

#define GET_WM_COMMAND_ID(wp, lp)       LOWORD(wp)
#define GET_WM_COMMAND_CMD(wp, lp)      HIWORD(wp)
#define GET_WM_COMMAND_HWND(wp, lp)     ((HWND)(lp))

/* Window class styles */
#define CS_DBLCLKS          0x0008

/* Menu position constants */
#define WINDOWMENU          3
#define SHORTMENU           2

/* Resource IDs - Icon resources */
#define ID_FRONTIER_ICON                1
#define ID_FATPAGE_ICON                 2
#define ID_ROOT_ICON                    3
#define ID_2CLK_ICON                    4
#define ID_CARD_ICON                    5
#define ID_LAND_ICON                    6
#define ID_MENU_ICON                    7
#define ID_OP_ICON                      8
#define ID_PICT_ICON                    9
#define ID_SCRIPT_ICON                  10
#define ID_TABLE_ICON                   11
#define ID_WP_ICON                      12
#define ID_NOTE_ICON                    13
#define ID_DESKTOPSCRIPT_ICON           14
#define ID_OPML_ICON                    15

/* Menu IDs */
#define IDM_FILENEW                     1001
#define IDM_FILEOPEN                    1002
#define IDM_FILESAVE                    1003
#define IDM_FILESAVEAS                  1004
#define IDM_FILEPRINT                   1005
#define IDM_FILEEXIT                    1006
#define IDM_FILEABOUT                   1007

#define IDM_EDITUNDO                    2001
#define IDM_EDITCUT                     2002
#define IDM_EDITCOPY                    2003
#define IDM_EDITPASTE                   2004
#define IDM_EDITCLEAR                   2005
#define IDM_EDITSELECT                  2006

#define IDM_WINDOWTILE                  4001
#define IDM_WINDOWCASCADE               4002
#define IDM_WINDOWCLOSEALL              4003
#define IDM_WINDOWCHILD                 4100

/* Dialog IDs */
#define IDD_ABOUT                       300

/* String IDs */
#define IDS_APPNAME                     18
#define IDS_UNTITLED                    17

/* Window extra bytes */
#define GWL_HWNDEDIT                    0
#define GWW_CHANGED                     4
#define CBWNDEXTRA                      12

/* Global variables */
extern GtkApplication *gtk_app;
extern GtkWidget *main_window;
extern GtkWidget *mdi_notebook;
extern HINSTANCE shell_instance;

/* Function declarations */
BOOL InitializeApplication(void);
BOOL InitializeInstance(int argc, char **argv);
void ShellMainEventLoop(void);
BOOL WinProcessMouseEvent(HWND hwnd, guint msg, guint button, gdouble x, gdouble y);
BOOL WinProcessKeyboardEvent(HWND hwnd, guint msg, guint keyval, guint state);
BOOL WinProcessPaintEvent(HWND hwnd);
BOOL WinProcessCommandEvent(HWND hwnd, guint msg, guint id);
BOOL WinProcessScrollEvent(HWND hwnd, guint msg, gdouble value, gboolean isvert);
BOOL WinProcessActivateEvent(HWND hwnd, guint msg, gboolean activate);

/* Cairo/GDI compatibility helpers */
cairo_t* GetDC(HWND hwnd);
void ReleaseDC(HWND hwnd, cairo_t *cr);
BOOL InvalidateRect(HWND hwnd, const GdkRectangle *rect, BOOL erase);
BOOL UpdateWindow(HWND hwnd);

/* Window management */
HWND CreateWindow(const char *class_name, const char *title, 
                  guint32 style, int x, int y, int width, int height,
                  HWND parent, HMENU menu, HINSTANCE instance, gpointer param);
BOOL DestroyWindow(HWND hwnd);
BOOL ShowWindow(HWND hwnd, int cmd);
BOOL SetWindowText(HWND hwnd, const char *text);
int GetWindowText(HWND hwnd, char *buffer, int maxcount);
BOOL MoveWindow(HWND hwnd, int x, int y, int width, int height, BOOL repaint);
BOOL SetWindowPos(HWND hwnd, HWND insert_after, int x, int y, 
                  int width, int height, guint flags);

/* Menu functions */
HMENU CreateMenu(void);
HMENU CreatePopupMenu(void);
BOOL AppendMenu(HMENU menu, guint flags, guint id, const char *label);
BOOL DestroyMenu(HMENU menu);

/* Message box */
int MessageBox(HWND hwnd, const char *text, const char *caption, guint type);

/* Clipboard */
BOOL OpenClipboard(HWND hwnd);
BOOL CloseClipboard(void);
BOOL EmptyClipboard(void);
gpointer GetClipboardData(guint format);
gpointer SetClipboardData(guint format, gpointer data);

/* Timer */
guint SetTimer(HWND hwnd, guint id, guint interval, GSourceFunc callback);
BOOL KillTimer(HWND hwnd, guint id);

/* Utility functions */
guint32 GetTickCount(void);
void Sleep(guint milliseconds);

/* Process functions */
pid_t GetCurrentProcessId(void);

/* File dialogs */
BOOL GetOpenFileName(GtkWindow *parent, char *filename, int maxlen);
BOOL GetSaveFileName(GtkWindow *parent, char *filename, int maxlen);

#endif /* LINUXLAND_H */
