
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

#ifdef PIKE
	#define _WIN32_IE 0x0500
#endif

#include "WinLand.h"
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
#include "WinSockNetEvents.h"
#include "FrontierWinMain.h"
#include "winregistry.h"
#include "langdll.h"
#include "htmlcontrol.h"  /*8/26/00 by RAB*/
#include "langinternal.h" /*7.0b31 PBS*/
#include "tablestructure.h" /*7.0b31 PBS*/
#include "dockmenu.h"

extern typrocessid langipcself;

HINSTANCE shellinstance;
HINSTANCE hInst;
static boolean flinstanceinitialized = false; //5.1.5b7 dmb

HWND shellframewindow;
HWND hwndMDIClient= NULL;
HWND hwndActive;
HWND hwndStatus = NULL;
HWND hwndHTMLControl = NULL;
long gstatuswindowparts = 1;
long gstatuswindowarray[50] = {0};
HICON hIconApp;

//#ifdef PIKE
	HICON hTrayIconApp;
	#define FWM_SYSTRAYICON	WM_USER + 19
//#endif

HANDLE hAccel;
static HWND hwndNextViewer = (HWND) -1;

LONG styleDefault = WS_OVERLAPPEDWINDOW;	//WS_MAXIMIZE; /* Default style bits for child windows  */

LONG CALLBACK FrontierOPWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LONG CALLBACK FrontierFrameWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef PIKE

#ifndef OPMLEDITOR
CHAR szFrame[] = "pikeframe";   /* Class name for "frame" window */

CHAR szChild1[] = "pikeopchild";   /* Class name for op MDI window     */
CHAR szChildFTop[] = "pikeoutlinechild";
CHAR szChildFTwp[] = "pikewpchild";
CHAR szChildFTtb[] = "piketablechild";
CHAR szChildFTmb[] = "pikemenuchild";
CHAR szChildFTsc[] = "pikescriptchild";
CHAR szChildRoot[] = "pikerootchild";

CHAR szChildHTML[] = "pikeHTMLchild";

CHAR sz_frontierstartreplace [] = "PikeStartReplace";
CHAR sz_frontierstartsearch [] = "PikeStartSearch";

#else  // OPMLEDITOR
CHAR szFrame[] = "opmlframe";   /* Class name for "frame" window */

CHAR szChild1[] = "opmlopchild";   /* Class name for op MDI window     */
CHAR szChildFTop[] = "opmloutlinechild";
CHAR szChildFTwp[] = "opmlwpchild";
CHAR szChildFTtb[] = "opmltablechild";
CHAR szChildFTmb[] = "opmlmenuchild";
CHAR szChildFTsc[] = "opmlscriptchild";
CHAR szChildRoot[] = "opmlerootchild";

CHAR szChildHTML[] = "opmlHTMLchild";

CHAR sz_frontierstartreplace [] = "OpmlStartReplace";
CHAR sz_frontierstartsearch [] = "OpmlStartSearch";
#endif // OPMLEDITOR

#else   // !PIKE
CHAR szFrame[] = "frontierframe";   /* Class name for "frame" window */

CHAR szChild1[] = "frontieropchild";   /* Class name for op MDI window     */
CHAR szChildFTop[] = "frontieroutlinechild";
CHAR szChildFTwp[] = "frontierwpchild";
CHAR szChildFTtb[] = "frontiertablechild";
CHAR szChildFTmb[] = "frontiermenuchild";
CHAR szChildFTsc[] = "frontierscriptchild";
CHAR szChildRoot[] = "frontierrootchild";

CHAR szChildHTML[] = "frontierHTMLchild";

CHAR sz_frontierstartreplace [] = "FrontierStartReplace";
CHAR sz_frontierstartsearch [] = "FrontierStartSearch";
#endif  // !PIKE

UINT wm_startreplace;
UINT wm_startsearch;

CHAR *pCmdLine;

static UINT wm_frontieropenfile;
static HANDLE frontiermutex;
static UINT wm_findmsgstring;		/*Used for find/replace dialogs */

//RAB: 1/20/98 this was 8 changed to 12 for status bar control window handle
#define OPWindowExtraByteCount 12

//-----------------------RABCC Message Tracking Code-----------------------------------------------
// August 20, 1994
// This code provides a logging of messages received by a window.  This is useful for tracking 
// re-entrency bugs.
//
// 
#define MESSAGETRACKSYSTEM 0

#if (MESSAGETRACKSYSTEM == 1)
#pragma message ("***************************************MESSAGETRACKING IS ON!!***************************************")
FILE * errfile = NULL;
#define MSGDEFAULT	3
#define MSGOTHER    4


UINT MESSAGESTACK[100];
short MESSAGESTACKPTR = 0;

void PUSHMESSAGE (UINT msg)
	{
	MESSAGESTACK[MESSAGESTACKPTR] = msg;
	++MESSAGESTACKPTR;
	}

UINT POPMESSAGE ()
	{
	--MESSAGESTACKPTR;
	return (MESSAGESTACK[MESSAGESTACKPTR]);
	}


void GETMESSAGETEXT (UINT msg, char * msgname)
	{
	switch (msg)
		{
		case WM_NULL:
			strcpy (msgname, "WM_NULL");
			break;

        case WM_CREATE                       :   //001
 			strcpy (msgname, "WM_CREATE");
			break;

        case WM_DESTROY                      :   //0x0002
			strcpy (msgname, "WM_DESTROY");
			break;

        case WM_MOVE                         :   //0x0003
			strcpy (msgname, "WM_MOVE");
			break;

        case WM_SIZE                         :   //0x0005
			strcpy (msgname, "WM_SIZE");
			break;

        case WM_ACTIVATE                     :   //0x0006
			strcpy (msgname, "WM_ACTIVATE");
			break;

        case WM_SETFOCUS                     :   //0x0007
			strcpy (msgname, "WM_SETFOCUS");
			break;

        case WM_KILLFOCUS                    :   //0x0008
			strcpy (msgname, "WM_KILLFOCUS");
			break;

        case WM_ENABLE                       :   //0x000A
			strcpy (msgname, "WM_ENABLE");
			break;

        case WM_SETREDRAW                    :   //0x000B
			strcpy (msgname, "WM_SETREDRAW");
			break;

        case WM_SETTEXT                      :   //0x000C
			strcpy (msgname, "WM_SETTEXT");
			break;

        case WM_GETTEXT                      :   //0x000D
			strcpy (msgname, "WM_GETTEXT");
			break;

        case WM_GETTEXTLENGTH                :   //0x000E
			strcpy (msgname, "WM_GETTEXTLENGTH");
			break;

        case WM_PAINT                        :   //0x000F
			strcpy (msgname, "WM_PAINT");
			break;

        case WM_CLOSE                        :   //0x0010
			strcpy (msgname, "WM_CLOSE");
			break;

        case WM_QUERYENDSESSION              :   //0x0011
			strcpy (msgname, "WM_QUERYENDSESSION");
			break;

        case WM_QUIT                         :   //0x0012
			strcpy (msgname, "WM_QUIT");
			break;

        case WM_QUERYOPEN                    :   //0x0013
			strcpy (msgname, "WM_QUERYOPEN");
			break;

        case WM_ERASEBKGND                   :   //0x0014
			strcpy (msgname, "WM_ERASEBKGND");
			break;

        case WM_SYSCOLORCHANGE               :   //0x0015
			strcpy (msgname, "WM_SYSCOLORCHANGE");
			break;

        case WM_ENDSESSION                   :   //0x0016
			strcpy (msgname, "WM_ENDSESSION");
			break;

        case WM_SHOWWINDOW                   :   //0x0018
			strcpy (msgname, "WM_SHOWWINDOW");
			break;

        case WM_WININICHANGE                 :   //0x001A
			strcpy (msgname, "WM_WININICHANGE");
			break;

        case WM_DEVMODECHANGE                :   //0x001B
			strcpy (msgname, "WM_DEVMODECHANGE");
			break;

        case WM_ACTIVATEAPP                  :   //0x001C
			strcpy (msgname, "WM_ACTIVATEAPP");
			break;

        case WM_FONTCHANGE                   :   //0x001D
			strcpy (msgname, "WM_FONTCHANGE");
			break;

        case WM_TIMECHANGE                   :   //0x001E
			strcpy (msgname, "WM_TIMECHANGE");
			break;

        case WM_CANCELMODE                   :   //0x001F
			strcpy (msgname, "WM_CANCELMODE");
			break;

        case WM_SETCURSOR                    :   //0x0020
			strcpy (msgname, "WM_SETCURSOR");
			break;

        case WM_MOUSEACTIVATE                :   //0x0021
			strcpy (msgname, "WM_MOUSEACTIVATE");
			break;

        case WM_CHILDACTIVATE                :   //0x0022
			strcpy (msgname, "WM_CHILDACTIVATE");
			break;

        case WM_QUEUESYNC                    :   //0x0023
			strcpy (msgname, "WM_QUEUESYNC");
			break;


        case WM_GETMINMAXINFO                :   //0x0024
			strcpy (msgname, "WM_GETMINMAXINFO");
			break;

        case WM_PAINTICON                    :   //0x0026
			strcpy (msgname, "WM_PAINTICON");
			break;

        case WM_ICONERASEBKGND               :   //0x0027
			strcpy (msgname, "WM_ICONERASEBKGND");
			break;

        case WM_NEXTDLGCTL                   :   //0x0028
			strcpy (msgname, "WM_NEXTDLGCTL");
			break;

        case WM_SPOOLERSTATUS                :   //0x002A
			strcpy (msgname, "WM_SPOOLERSTATUS");
			break;

        case WM_DRAWITEM                     :   //0x002B
			strcpy (msgname, "WM_DRAWITEM");
			break;

        case WM_MEASUREITEM                  :   //0x002C
			strcpy (msgname, "WM_MEASUREITEM");
			break;

        case WM_DELETEITEM                   :   //0x002D
			strcpy (msgname, "WM_DELETEITEM");
			break;

        case WM_VKEYTOITEM                   :   //0x002E
			strcpy (msgname, "WM_VKEYTOITEM");
			break;

        case WM_CHARTOITEM                   :   //0x002F
			strcpy (msgname, "WM_CHARTOITEM");
			break;

        case WM_SETFONT                      :   //0x0030
			strcpy (msgname, "WM_SETFONT");
			break;

        case WM_GETFONT                      :   //0x0031
			strcpy (msgname, "WM_GETFONT");
			break;

        case WM_SETHOTKEY                    :   //0x0032
			strcpy (msgname, "WM_SETHOTKEY");
			break;

        case WM_GETHOTKEY                    :   //0x0033
			strcpy (msgname, "WM_GETHOTKEY");
			break;

        case WM_QUERYDRAGICON                :   //0x0037
			strcpy (msgname, "WM_QUERYDRAGICON");
			break;

        case WM_COMPAREITEM                  :   //0x0039
			strcpy (msgname, "WM_COMPAREITEM");
			break;

        case WM_COMPACTING                   :   //0x0041
			strcpy (msgname, "WM_COMPACTING");
			break;

        case WM_COMMNOTIFY                   :   //0x0044  /* no longer suported */
			strcpy (msgname, "WM_COMMNOTIFY");
			break;

        case WM_WINDOWPOSCHANGING            :   //0x0046
			strcpy (msgname, "WM_WINDOWPOSCHANGING");
			break;

        case WM_WINDOWPOSCHANGED             :   //0x0047
			strcpy (msgname, "WM_WINDOWPOSCHANGED");
			break;

        case WM_POWER                        :   //0x0048
			strcpy (msgname, "WM_POWER");
			break;

        case WM_COPYDATA                     :   //0x004A
			strcpy (msgname, "WM_COPYDATA");
			break;

        case WM_CANCELJOURNAL                :   //0x004B
			strcpy (msgname, "WM_CANCELJOURNAL");
			break;

        case WM_NOTIFY                       :   //0x004E
			strcpy (msgname, "WM_NOTIFY");
			break;

        case WM_INPUTLANGCHANGEREQUEST       :   //0x0050
			strcpy (msgname, "WM_INPUTLANGCHANGEREQUEST");
			break;

        case WM_INPUTLANGCHANGE              :   //0x0051
			strcpy (msgname, "WM_INPUTLANGCHANGE");
			break;

        case WM_TCARD                        :   //0x0052
			strcpy (msgname, "WM_TCARD");
			break;

        case WM_HELP                         :   //0x0053
			strcpy (msgname, "WM_HELP");
			break;

        case WM_USERCHANGED                  :   //0x0054
			strcpy (msgname, "WM_USERCHANGED");
			break;

        case WM_NOTIFYFORMAT                 :   //0x0055
			strcpy (msgname, "WM_NOTIFYFORMAT");
			break;

        case WM_CONTEXTMENU                  :   //0x007B
			strcpy (msgname, "WM_CONTEXTMENU");
			break;

        case WM_STYLECHANGING                :   //0x007C
			strcpy (msgname, "WM_STYLECHANGING");
			break;

        case WM_STYLECHANGED                 :   //0x007D
			strcpy (msgname, "WM_STYLECHANGED");
			break;

        case WM_DISPLAYCHANGE                :   //0x007E
			strcpy (msgname, "WM_DISPLAYCHANGE");
			break;

        case WM_GETICON                      :   //0x007F
			strcpy (msgname, "WM_GETICON");
			break;

        case WM_SETICON                      :   //0x0080
			strcpy (msgname, "WM_SETICON");
			break;

		case WM_NCCREATE                     :   //0x0081
			strcpy (msgname, "WM_NCCREATE");
			break;

        case WM_NCDESTROY                    :   //0x0082
			strcpy (msgname, "WM_NCDESTROY");
			break;

        case WM_NCCALCSIZE                   :   //0x0083
			strcpy (msgname, "WM_NCCALCSIZE");
			break;

        case WM_NCHITTEST                    :   //0x0084
			strcpy (msgname, "WM_NCHITTEST");
			break;

        case WM_NCPAINT                      :   //0x0085
			strcpy (msgname, "WM_NCPAINT");
			break;

        case WM_NCACTIVATE                   :   //0x0086
			strcpy (msgname, "WM_NCACTIVATE");
			break;

        case WM_GETDLGCODE                   :   //0x0087
			strcpy (msgname, "WM_GETDLGCODE");
			break;

        case WM_NCMOUSEMOVE                  :   //0x00A0
			strcpy (msgname, "WM_NCMOUSEMOVE");
			break;

        case WM_NCLBUTTONDOWN                :   //0x00A1
			strcpy (msgname, "WM_NCLBUTTONDOWN");
			break;

        case WM_NCLBUTTONUP                  :   //0x00A2
			strcpy (msgname, "WM_NCLBUTTONUP");
			break;

        case WM_NCLBUTTONDBLCLK              :   //0x00A3
			strcpy (msgname, "WM_NCLBUTTONDBLCLK");
			break;

        case WM_NCRBUTTONDOWN                :   //0x00A4
			strcpy (msgname, "WM_NCRBUTTONDOWN");
			break;

        case WM_NCRBUTTONUP                  :   //0x00A5
			strcpy (msgname, "WM_NCRBUTTONUP");
			break;

        case WM_NCRBUTTONDBLCLK              :   //0x00A6
			strcpy (msgname, "WM_NCRBUTTONDBLCLK");
			break;

        case WM_NCMBUTTONDOWN                :   //0x00A7
			strcpy (msgname, "WM_NCMBUTTONDOWN");
			break;

        case WM_NCMBUTTONUP                  :   //0x00A8
			strcpy (msgname, "WM_NCMBUTTONUP");
			break;

        case WM_NCMBUTTONDBLCLK              :   //0x00A9
			strcpy (msgname, "WM_NCMBUTTONDBLCLK");
			break;

        case WM_KEYDOWN                      :   //0x0100
			strcpy (msgname, "WM_KEYDOWN");
			break;

        case WM_KEYUP                        :   //0x0101
			strcpy (msgname, "WM_KEYUP");
			break;

        case WM_CHAR                         :   //0x0102
			strcpy (msgname, "WM_CHAR");
			break;

        case WM_DEADCHAR                     :   //0x0103
			strcpy (msgname, "WM_DEADCHAR");
			break;

        case WM_SYSKEYDOWN                   :   //0x0104
			strcpy (msgname, "WM_SYSKEYDOWN");
			break;

        case WM_SYSKEYUP                     :   //0x0105
			strcpy (msgname, "WM_SYSKEYUP");
			break;

        case WM_SYSCHAR                      :   //0x0106
			strcpy (msgname, "WM_SYSCHAR");
			break;

        case WM_SYSDEADCHAR                  :   //0x0107
			strcpy (msgname, "WM_SYSDEADCHAR");
			break;

        case WM_KEYLAST                      :   //0x0108
			strcpy (msgname, "WM_KEYLAST");
			break;

        case WM_IME_STARTCOMPOSITION         :   //0x010D
			strcpy (msgname, "WM_IME_STARTCOMPOSITION");
			break;

        case WM_IME_ENDCOMPOSITION           :   //0x010E
			strcpy (msgname, "WM_IME_ENDCOMPOSITION");
			break;

        case WM_IME_COMPOSITION              :   //0x010F
			strcpy (msgname, "WM_IME_COMPOSITION");
			break;

        case WM_INITDIALOG                   :   //0x0110
			strcpy (msgname, "WM_INITDIALOG");
			break;

        case WM_COMMAND                      :   //0x0111
			strcpy (msgname, "WM_COMMAND");
			break;

        case WM_SYSCOMMAND                   :   //0x0112
			strcpy (msgname, "WM_SYSCOMMAND");
			break;

        case WM_TIMER                        :   //0x0113
			strcpy (msgname, "WM_TIMER");
			break;

        case WM_HSCROLL                      :   //0x0114
			strcpy (msgname, "WM_HSCROLL");
			break;

        case WM_VSCROLL                      :   //0x0115
			strcpy (msgname, "WM_VSCROLL");
			break;

        case WM_INITMENU                     :   //0x0116
			strcpy (msgname, "WM_INITMENU");
			break;

        case WM_INITMENUPOPUP                :   //0x0117
			strcpy (msgname, "WM_INITMENUPOPUP");
			break;

        case WM_MENUSELECT                   :   //0x011F
			strcpy (msgname, "WM_MENUSELECT");
			break;

        case WM_MENUCHAR                     :   //0x0120
			strcpy (msgname, "WM_MENUCHAR");
			break;

        case WM_ENTERIDLE                    :   //0x0121
			strcpy (msgname, "WM_ENTERIDLE");
			break;


        case WM_CTLCOLORMSGBOX               :   //0x0132
			strcpy (msgname, "WM_CTLCOLORMSGBOX");
			break;

        case WM_CTLCOLOREDIT                 :   //0x0133
			strcpy (msgname, "WM_CTLCOLOREDIT");
			break;

        case WM_CTLCOLORLISTBOX              :   //0x0134
			strcpy (msgname, "WM_CTLCOLORLISTBOX");
			break;

        case WM_CTLCOLORBTN                  :   //0x0135
			strcpy (msgname, "WM_CTLCOLORBTN");
			break;

        case WM_CTLCOLORDLG                  :   //0x0136
			strcpy (msgname, "WM_CTLCOLORDLG");
			break;

        case WM_CTLCOLORSCROLLBAR            :   //0x0137
			strcpy (msgname, "WM_CTLCOLORSCROLLBAR");
			break;

        case WM_CTLCOLORSTATIC               :   //0x0138
			strcpy (msgname, "WM_CTLCOLORSTATIC");
			break;

        case WM_MOUSEMOVE                    :   //0x0200
			strcpy (msgname, "WM_MOUSEMOVE");
			break;

        case WM_LBUTTONDOWN                  :   //0x0201
			strcpy (msgname, "WM_LBUTTONDOWN");
			break;

        case WM_LBUTTONUP                    :   //0x0202
			strcpy (msgname, "WM_LBUTTONUP");
			break;

        case WM_LBUTTONDBLCLK                :   //0x0203
			strcpy (msgname, "WM_LBUTTONDBLCLK");
			break;

        case WM_RBUTTONDOWN                  :   //0x0204
			strcpy (msgname, "WM_RBUTTONDOWN");
			break;

        case WM_RBUTTONUP                    :   //0x0205
			strcpy (msgname, "WM_RBUTTONUP");
			break;

        case WM_RBUTTONDBLCLK                :   //0x0206
			strcpy (msgname, "WM_RBUTTONDBLCLK");
			break;

        case WM_MBUTTONDOWN                  :   //0x0207
			strcpy (msgname, "WM_MBUTTONDOWN");
			break;

        case WM_MBUTTONUP                    :   //0x0208
			strcpy (msgname, "WM_MBUTTONUP");
			break;

        case WM_MBUTTONDBLCLK                :   //0x0209
			strcpy (msgname, "WM_MBUTTONDBLCLK");
			break;

//        case WM_MOUSEWHEEL                   :   //0x020A
//			strcpy (msgname, "WM_MOUSEWHEEL");
//			break;

        case WM_PARENTNOTIFY                 :   //0x0210
			strcpy (msgname, "WM_PARENTNOTIFY");
			break;

        case WM_ENTERMENULOOP                :   //0x0211
			strcpy (msgname, "WM_ENTERMENULOOP");
			break;

        case WM_EXITMENULOOP                 :   //0x0212
			strcpy (msgname, "WM_EXITMENULOOP");
			break;

        case WM_NEXTMENU                     :   //0x0213
			strcpy (msgname, "WM_NEXTMENU");
			break;

        case WM_SIZING                       :   //0x0214
			strcpy (msgname, "WM_SIZING");
			break;

        case WM_CAPTURECHANGED               :   //0x0215
			strcpy (msgname, "WM_CAPTURECHANGED");
			break;

        case WM_MOVING                       :   //0x0216
			strcpy (msgname, "WM_MOVING");
			break;

        case WM_POWERBROADCAST               :   //0x0218
			strcpy (msgname, "WM_POWERBROADCAST");
			break;

        case WM_DEVICECHANGE                 :   //0x0219
			strcpy (msgname, "WM_DEVICECHANGE");
			break;


        case WM_IME_SETCONTEXT               :   //0x0281
			strcpy (msgname, "WM_IME_SETCONTEXT");
			break;

        case WM_IME_NOTIFY                   :   //0x0282
			strcpy (msgname, "WM_IME_NOTIFY");
			break;

        case WM_IME_CONTROL                  :   //0x0283
			strcpy (msgname, "WM_IME_CONTROL");
			break;

        case WM_IME_COMPOSITIONFULL          :   //0x0284
			strcpy (msgname, "WM_IME_COMPOSITIONFULL");
			break;

        case WM_IME_SELECT                   :   //0x0285
			strcpy (msgname, "WM_IME_SELECT");
			break;

        case WM_IME_CHAR                     :   //0x0286
			strcpy (msgname, "WM_IME_CHAR");
			break;

        case WM_IME_KEYDOWN                  :   //0x0290
			strcpy (msgname, "WM_IME_KEYDOWN");
			break;

        case WM_IME_KEYUP                    :   //0x0291
			strcpy (msgname, "WM_IME_KEYUP");
			break;

        case WM_MDICREATE                    :   //0x0220
			strcpy (msgname, "WM_MDICREATE");
			break;

        case WM_MDIDESTROY                   :   //0x0221
			strcpy (msgname, "WM_MDIDESTROY");
			break;

        case WM_MDIACTIVATE                  :   //0x0222
			strcpy (msgname, "WM_MDIACTIVATE");
			break;

        case WM_MDIRESTORE                   :   //0x0223
			strcpy (msgname, "WM_MDIRESTORE");
			break;

        case WM_MDINEXT                      :   //0x0224
			strcpy (msgname, "WM_MDINEXT");
			break;

        case WM_MDIMAXIMIZE                  :   //0x0225
			strcpy (msgname, "WM_MDIMAXIMIZE");
			break;

        case WM_MDITILE                      :   //0x0226
			strcpy (msgname, "WM_MDITILE");
			break;

        case WM_MDICASCADE                   :   //0x0227
			strcpy (msgname, "WM_MDICASCADE");
			break;

        case WM_MDIICONARRANGE               :   //0x0228
			strcpy (msgname, "WM_MDIICONARRANGE");
			break;

        case WM_MDIGETACTIVE                 :   //0x0229
			strcpy (msgname, "WM_MDIGETACTIVE");
			break;

        case WM_MDISETMENU                   :   //0x0230
			strcpy (msgname, "WM_MDISETMENU");
			break;

        case WM_ENTERSIZEMOVE                :   //0x0231
			strcpy (msgname, "WM_ENTERSIZEMOVE");
			break;

        case WM_EXITSIZEMOVE                 :   //0x0232
			strcpy (msgname, "WM_EXITSIZEMOVE");
			break;

        case WM_DROPFILES                    :   //0x0233
			strcpy (msgname, "WM_DROPFILES");
			break;

        case WM_MDIREFRESHMENU               :   //0x0234
			strcpy (msgname, "WM_MDIREFRESHMENU");
			break;

//        case WM_MOUSEHOVER                   :   //0x02A1
//			strcpy (msgname, "WM_MOUSEHOVER");
//			break;

//        case WM_MOUSELEAVE                   :   //0x02A3
//			strcpy (msgname, "WM_MOUSELEAVE");
//			break;

        case WM_CUT                          :   //0x0300
			strcpy (msgname, "WM_CUT");
			break;

        case WM_COPY                         :   //0x0301
			strcpy (msgname, "WM_COPY");
			break;

        case WM_PASTE                        :   //0x0302
			strcpy (msgname, "WM_PASTE");
			break;

        case WM_CLEAR                        :   //0x0303
			strcpy (msgname, "WM_CLEAR");
			break;

        case WM_UNDO                         :   //0x0304
			strcpy (msgname, "WM_UNDO");
			break;

        case WM_RENDERFORMAT                 :   //0x0305
			strcpy (msgname, "WM_RENDERFORMAT");
			break;

        case WM_RENDERALLFORMATS             :   //0x0306
			strcpy (msgname, "WM_RENDERALLFORMATS");
			break;

        case WM_DESTROYCLIPBOARD             :   //0x0307
			strcpy (msgname, "WM_DESTROYCLIPBOARD");
			break;

        case WM_DRAWCLIPBOARD                :   //0x0308
			strcpy (msgname, "WM_DRAWCLIPBOARD");
			break;

        case WM_PAINTCLIPBOARD               :   //0x0309
			strcpy (msgname, "WM_PAINTCLIPBOARD");
			break;

        case WM_VSCROLLCLIPBOARD             :   //0x030A
			strcpy (msgname, "WM_VSCROLLCLIPBOARD");
			break;

        case WM_SIZECLIPBOARD                :   //0x030B
			strcpy (msgname, "WM_SIZECLIPBOARD");
			break;

        case WM_ASKCBFORMATNAME              :   //0x030C
			strcpy (msgname, "WM_ASKCBFORMATNAME");
			break;

        case WM_CHANGECBCHAIN                :   //0x030D
			strcpy (msgname, "WM_CHANGECBCHAIN");
			break;

        case WM_HSCROLLCLIPBOARD             :   //0x030E
			strcpy (msgname, "WM_HSCROLLCLIPBOARD");
			break;

        case WM_QUERYNEWPALETTE              :   //0x030F
			strcpy (msgname, "WM_QUERYNEWPALETTE");
			break;

        case WM_PALETTEISCHANGING            :   //0x0310
			strcpy (msgname, "WM_PALETTEISCHANGING");
			break;

        case WM_PALETTECHANGED               :   //0x0311
			strcpy (msgname, "WM_PALETTECHANGED");
			break;

        case WM_HOTKEY                       :   //0x0312
			strcpy (msgname, "WM_HOTKEY");
			break;

        case WM_PRINT                        :   //0x0317
			strcpy (msgname, "WM_PRINT");
			break;

        case WM_PRINTCLIENT                  :   //0x0318
			strcpy (msgname, "WM_PRINTCLIENT");
			break;

        case WM_HANDHELDFIRST                :   //0x0358
			strcpy (msgname, "WM_HANDHELDFIRST");
			break;

        case WM_HANDHELDLAST                 :   //0x035F
			strcpy (msgname, "WM_HANDHELDLAST");
			break;

        case WM_AFXFIRST                     :   //0x0360
			strcpy (msgname, "WM_AFXFIRST");
			break;

        case WM_AFXLAST                      :   //0x037F
			strcpy (msgname, "WM_AFXLAST");
			break;

        case WM_PENWINFIRST                  :   //0x0380
			strcpy (msgname, "WM_PENWINFIRST");
			break;

		case WM_PENWINLAST                   :   //0x038F
			strcpy (msgname, "WM_PENWINLAST");
			break;

		default:
			strcpy (msgname, "Unknown Message");
			break;
		}
	}


void MESSAGEENDER (short x)
	{
	char msgname[100];
	char msgname2[100];
	UINT msg;

	switch (x)
		{
		case true:
			strcpy (msgname, "Exit Message:  TRUE");
			break;

		case false:
			strcpy (msgname, "Exit Message:  FALSE");
			break;

		case MSGDEFAULT:
			strcpy (msgname, "Exit Message:  Default");
			break;

		case MSGOTHER:
			strcpy (msgname, "Exit Message:  Other");
			break;

		default:
			strcpy (msgname, "Exit Message:  UNKNOWN");
			break;
		}

	msg = POPMESSAGE();

	GETMESSAGETEXT (msg, msgname2);

	fprintf (errfile, "%s\t%s\n", msgname, msgname2);
	}

void MESSAGETRACKER (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	char msgname[100];

	if (errfile == NULL)
		{
		errfile = fopen ("errfile.txt", "w+");
		}

	GETMESSAGETEXT (msg, msgname);

	fprintf (errfile, "Message: %-30s  (%ld)\t\t Window: %lX\twParam: %lX\tlParam: %lX\n",
						msgname, msg, hwnd, wParam, lParam);

	PUSHMESSAGE (msg);
	}
#else
#define MESSAGETRACKER(a,b,c,d)
#define MESSAGEENDER(x)
#endif


static boolean ccerrormessage (bigstring bs) {
	
	char s [256];
	short itemnumber;

	copyptocstring (bs, s);

	itemnumber = MessageBox (hwndMDIClient, s, "Error Info", MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
	
	return (true);
	} /*ccerrormessage*/


static boolean shellpushcancoonglobals (void) {
	
	short ix;
	
	if (!shellfindcallbacks (idcancoonconfig, &ix))
		return (false);
	
	if (!shellpushglobals (nil)) /*save off old state*/
		return (false);
		
	shellglobals = globalsarray [ix];
	
	config = shellglobals.config;
	
	return (true);
	} /*shellpushcancoonglobals*/
	

extern long getstatusbarheight () {
	/* 2005-01-30 SMD - made extern */
	RECT rectStatus;

	if (hwndStatus != NULL) {
		GetWindowRect(hwndStatus, &rectStatus);

		return (rectStatus.bottom - rectStatus.top);
		}

	return (0);
	} /*getstatusbarheight*/

static long getstatusbarwidth () {
	RECT rectStatus;

	if (hwndStatus != NULL) {
		GetWindowRect(hwndStatus, &rectStatus);

		return (rectStatus.right - rectStatus.left);
		}

	return (0);
	} /*getstatusbarwidth*/

static boolean setstatusbartext (char * szText, long partNumber) {
	if (hwndStatus != NULL) {
		return ((boolean) SendMessage(hwndStatus, SB_SETTEXT, (WPARAM) partNumber, (LPARAM) (LPSTR) szText));
		}
	return (false);
	} /*setstatusbartext*/

boolean setstatusbarstring (bigstring bs, long partNumber) {
	boolean res;

	nullterminate (bs);

	releasethreadglobals();

	res = setstatusbartext (stringbaseaddress(bs), partNumber);

	grabthreadglobals();

	return (res);
	} /*setstatusbarstring*/


boolean getstatusbarstring (bigstring bs, long partNumber) {
	boolean res;
	unsigned long longres;

	res = true;

	releasethreadglobals();

	if (hwndStatus != NULL) {
		longres = SendMessage(hwndStatus, SB_GETTEXT, (WPARAM) partNumber, (LPARAM) (LPSTR) stringbaseaddress(bs));
		}

	grabthreadglobals();

	setstringlength (bs, LOWORD(longres));

	return (res);
	} /*getstatusbarstring*/


static boolean setDefinedStatusBarParts (long width) {
	int aWidths[50];
	int dec, i;
	int borders[4];

	if (hwndStatus != NULL) {
		SendMessage(hwndStatus, SB_GETBORDERS, 0, (LPARAM) borders);

//		width = width - borders[1] - borders[1]; /*Account for vertical width*/

		aWidths[gstatuswindowparts-1] = -1;
		dec = gstatuswindowarray[gstatuswindowparts-1] + 13 + (borders[1]*2);

		for (i = gstatuswindowparts-2; i >= 0; i--) {
			aWidths[i] = width - dec;
			dec = dec + gstatuswindowarray[i];
			}

		return ((boolean)SendMessage (hwndStatus, SB_SETPARTS, (WPARAM) gstatuswindowparts, (LPARAM)aWidths));
		}

	return (false);
	}

boolean setstatusbarparts (long count, long * array) {
	int i;
	boolean res;

	for (i = 0; i < count; i++) {
		gstatuswindowarray[i] = array[i];
		}

	gstatuswindowparts = count;

	releasethreadglobals();

	res = setDefinedStatusBarParts (getstatusbarwidth());

	grabthreadglobals();

	return (res);
	} /*setstatusbarparts*/


long getstatusbarparts (long * array) {
	int i;
	long res;
	int aWidths[50];
	RECT r;
	long prior;

	releasethreadglobals();

	res = SendMessage (hwndStatus, SB_GETPARTS, (WPARAM) 50, (LPARAM)aWidths);

	if (res > 0) {
		SendMessage (hwndStatus, SB_GETRECT, (WPARAM) res - 1, (LPARAM) &r);
		}

	grabthreadglobals();


	prior = 0;

	for (i = 0; i < res; i++) {
		if (aWidths[i] != -1)
			array[i] = aWidths[i] - prior;
		else
			array[i] = r.right-r.left;

		prior = aWidths[i];
		}

	return (res);
	} /*getstatusbarparts*/






WNDPROC wndprocMDIClient;

LONG CALLBACK FrontierMDIWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	
    switch (msg){
		case WM_WINDOWPOSCHANGING:
			{
			WINDOWPOS * wp;

			if (hwndMDIClient != NULL) {

				if (hwnd == hwndMDIClient) {

				wp = (WINDOWPOS *) lParam;

				wp->cy = wp->cy - getstatusbarheight();
					}
				}

			return (CallWindowProc (wndprocMDIClient, hwnd, msg, wParam, lParam));

			}

#if (FRONTIERWEB == 1)
		case WM_NCCALCSIZE:
			{
			LPNCCALCSIZE_PARAMS lp;
			long res;
			HWND tophwnd;
			int flNoMaxedWindow;

			res = CallWindowProc (wndprocMDIClient, hwnd, msg, wParam, lParam);

			//RAB 2/21/2001: If there is a maxed window do not size HTML control
			flNoMaxedWindow = true;

			tophwnd = GetTopWindow (hwndMDIClient);

			if (IsZoomed (tophwnd)) {
				flNoMaxedWindow = false;
				}

			if (wParam && flNoMaxedWindow) {
				lp = (LPNCCALCSIZE_PARAMS) lParam;

				SetWindowPos (hwndHTMLControl, HWND_BOTTOM, 0, 0, lp->rgrc[0].right, lp->rgrc[0].bottom, 0);
				}

			return (res);
			}

		case WM_SIZE:
			{
			HWND tophwnd;

			if (hwndHTMLControl != NULL) {
				//RAB 2/21/2001: If there is a maxed window do not size HTML control
				tophwnd = GetTopWindow (hwndMDIClient);

				if (! IsZoomed (tophwnd)) {
					SetWindowPos (hwndHTMLControl, HWND_BOTTOM, 0, 0, LOWORD(lParam), HIWORD (lParam), 0);
					}
				}
			break;
			}

		case WM_VSCROLL:
		case WM_HSCROLL:
			{
			RECT r;

			if (hwndHTMLControl != NULL) {
			
//				GetClientRect (shellframewindow, &r);
				GetClientRect (hwndMDIClient, &r);

				SetWindowPos (hwndHTMLControl, HWND_BOTTOM, 0, 0, r.right, r.bottom, SWP_NOSIZE | SWP_NOZORDER);
				}
			break;
			}
#endif
		}
	return (CallWindowProc (wndprocMDIClient, hwnd, msg, wParam, lParam));

	 }




	
static boolean registerChildWindowClass (short idicon, CHAR *classname) {
	
    WNDCLASS wc;
    wc.style         = CS_DBLCLKS;
//  wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC) FrontierOPWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = OPWindowExtraByteCount;
    wc.hInstance	 = hInst;
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(idicon));
    wc.hCursor       = NULL; // 5.0a8 dmb - was: LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = classname;

    return (RegisterClass(&wc) != 0);
	} /*registerChildWindowClass*/

static boolean registerHTMLChildWindowClass () {
	
    WNDCLASS wc;
    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = (WNDPROC) htmlControlWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = OPWindowExtraByteCount;
    wc.hInstance	 = hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL; 
    wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szChildHTML;

    return (RegisterClass(&wc) != 0);
	} /*registerHTMLChildWindowClass*/


BOOL InitializeApplication() {
	
    WNDCLASS wc;
	char bsfatfile [] = "\x0c" "Fatpage_File";
	char bsfrontierdoc [] = "\x18" "Frontier Document [FATP]";
	tyfilespec fs;
	bigstring bsopen;

	getapplicationfilespec (NULL, &fs);

	copystring (fsname (&fs), bsopen);

	pushstring ("\x04 %1\0", bsopen);

#ifdef PIKE /*7.0b24 PBS -- register OPML file type and icon.*/
	
#ifndef OPMLEDITOR	
	registerFileType ("\x05" ".opml", "\x04" "OPML", "\x0b" "text/x-opml", "\x1c" "Radio UserLand OPML Document", fsname(&fs), -1 * ID_OPML_ICON, bsopen);
#else
	registerFileType ("\x05" ".opml", "\x04" "OPML", "\x0b" "text/x-opml", "\x0d" "OPML Document", fsname(&fs), -1 * ID_OPML_ICON, bsopen);
#endif

#else
	registerFileType ("\x04" ".fat", bsfatfile, "\x09" "text/fatp", bsfrontierdoc, fsname(&fs), -1 * ID_FATPAGE_ICON, bsopen);
	registerFileType ("\x05" ".fatp", bsfatfile, "\x09" "text/fatp", NULL, NULL, -1 * ID_FATPAGE_ICON, NULL);
	registerFileType ("\x05" ".root", "\x0c" "FrontierRoot", NULL, "\x18" "Frontier Object Database", fsname(&fs), -1 * ID_ROOT_ICON, bsopen);
	registerFileType ("\x05" ".2clk", "\x0c" "Frontier2Clk", NULL, "\x17" "Frontier Desktop Script", fsname(&fs), -1 * ID_2CLK_ICON, bsopen);

	registerFileType ("\x05" ".FTwp", "\x0a" "FrontierWP", NULL, "\x14" "Frontier WP Document", fsname(&fs), -1 * ID_WP_ICON, bsopen);
	registerFileType ("\x05" ".FTop", "\x0f" "FrontierOutline", NULL, "\x10" "Frontier Outline", fsname(&fs), -1 * ID_OP_ICON, bsopen);
	registerFileType ("\x05" ".FTsc", "\x0e" "FrontierScript", NULL, "\x0f" "Frontier Script", fsname(&fs), -1 * ID_SCRIPT_ICON, bsopen);
	registerFileType ("\x05" ".FTmb", "\x0c" "FrontierMenu", NULL, "\x0d" "Frontier Menu", fsname(&fs), -1 * ID_MENU_ICON, bsopen);
	registerFileType ("\x05" ".FTtb", "\x0d" "FrontierTable", NULL, "\x0e" "Frontier Table", fsname(&fs), -1 * ID_TABLE_ICON, bsopen);
	registerFileType ("\x05" ".FTds", "\x15" "FrontierDesktopScript", NULL, "\x17" "Frontier Desktop Script", fsname(&fs), -1 * ID_DESKTOPSCRIPT_ICON, bsopen);
#endif

	/* 9/24/01 RAB */
    hIconApp = LoadIcon(hInst,MAKEINTRESOURCE(ID_FRONTIER_ICON));

//#ifdef PIKE
    hTrayIconApp = LoadIcon(hInst,MAKEINTRESOURCE(ID_FRONTIERTRAY_ICON));
//#endif


    /* Register the frame class */
    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = (WNDPROC) FrontierFrameWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 4;
    wc.hInstance    = hInst;
    wc.hIcon         = hIconApp;
    wc.hCursor       = NULL; // 5.0a8 dmb - was: LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE+1);
    wc.lpszMenuName  = IDMULTIPAD;
    wc.lpszClassName = szFrame;

    if (!RegisterClass (&wc) )
		return (FALSE);

    /* Register the MDI child classes */ /*7.0fc1 PBS: use Frontier icon*/
	if (!registerChildWindowClass (/*ID_NOTE_ICON*/ ID_FRONTIER_ICON, szChild1)) /*Class for generic windows (About, QuickScript).*/
		return (false);

	if (!registerChildWindowClass (ID_OP_ICON, szChildFTop))
		return (false);

	if (!registerChildWindowClass (ID_WP_ICON, szChildFTwp))
		return (false);

	if (!registerChildWindowClass (ID_TABLE_ICON, szChildFTtb))
		return (false);

	if (!registerChildWindowClass (ID_MENU_ICON, szChildFTmb))
		return (false);

	if (!registerChildWindowClass (ID_SCRIPT_ICON, szChildFTsc))
		return (false);

	if (!registerChildWindowClass (ID_ROOT_ICON, szChildRoot))
		return (false);
	
	registerHTMLChildWindowClass ();

    return TRUE;
	} /*InitializeApplication*/

/*
static void showenv (void) {

	LPTSTR lpszVariable;  
	LPVOID lpvEnv; 
 
	// Get a pointer to the environment block. 
 
	lpvEnv = GetEnvironmentStrings(); 
 
	// Variable strings are separated by NULL byte, and the block is 
	// terminated by a NULL byte. 
 
	for (lpszVariable = (LPTSTR) lpvEnv; *lpszVariable; lpszVariable++) 
	{ 
	   while (*lpszVariable) 
		  putchar(*lpszVariable++); 
	   putchar('\n'); 
	} 
	
	FreeEnvironmentStrings (lpvEnv); //2003-05-18 AR
}*/ /*showenv*/


/****************************************************************************
 *                                                                          *
 *  FUNCTION   : InitializeInstance ()                                      *
 *                                                                          *
 *  PURPOSE    : Performs a per-instance initialization of Frontier. It     *
 *               also creates the frame and an MDI window.                  *
 *                                                                          *
 *  RETURNS    : TRUE  - If initialization was successful.                  *
 *               FALSE - otherwise.                                         *
 *                                                                          *
 ****************************************************************************/
BOOL InitializeInstance(LPSTR lpCmdLine, INT nCmdShow)
	{
    extern HWND  hwndMDIClient;
    CHAR         sz[80];
    HDC          hdc;
    HMENU        hmenu;
	unsigned long x, y, nWidth, nHeight;
	DWORD style;

#if FRONTIERWEB == 1
	bigstring bsresult;
	char * initialURL;
	RECT r;
#endif

    /* Get the base window title */
    LoadString (hInst, IDS_APPNAME, sz, sizeof(sz));

	//showenv ();	 /*2003-05-18 AR: this is debugging code, commented out*/
	
	if (! getProfileLong ("\x06" "FrameX", &x))
		x = CW_USEDEFAULT;

	if (! getProfileLong ("\x06" "FrameY", &y))
		x = CW_USEDEFAULT;		//yes X; y will be ignored

	if (! getProfileLong ("\x06" "FrameW", &nWidth))
		nWidth = CW_USEDEFAULT;

	if (! getProfileLong ("\x06" "FrameH", &nHeight))
		nWidth = CW_USEDEFAULT;		//yes nWidth; nHeight will be ignored

	if (getProfileLong ("\x0a" "FrameStyle", &style))
		nCmdShow = style;
	else
		style = 0;

//#ifdef PIKE

	#ifndef OPMLEDITOR           /*2005-04-17 dluebbert*/

		// nCmdShow = SW_HIDE;		/* 9/24/01 RAB */

	#endif // OPMLEDITOR

//#endif

	style = style | WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

    /* If the command line string is empty, nullify the pointer to it
    ** else copy command line into our data segment
    */
    if ( lpCmdLine && !(*lpCmdLine))
	     pCmdLine = NULL;
    else {
		pCmdLine = (CHAR *) LocalAlloc(LPTR, lstrlen(lpCmdLine) + 1);
		if (pCmdLine)
			lstrcpy(pCmdLine, lpCmdLine);
	    }
	
    /* Create the frame */
    shellframewindow = CreateWindow (szFrame,
			      sz,
			      style,
			      x,
			      y,
			      nWidth,
			      nHeight,
			      NULL,
			      NULL,
			      hInst,
			      NULL);

    if ((!shellframewindow) || (!hwndMDIClient))
		return FALSE;

	DeleteMenu (GetMenu (shellframewindow), 0, MF_BYPOSITION); // destroy the dummy menu
	
    /* Load main menu accelerators */
    if (!(hAccel = LoadAccelerators (hInst, IDMULTIPAD)))
		return FALSE;
	
	// Do the Frontier/Shell startup
	if (!shellinit ())
		return (false);
	
	langipcself = getcurrentprocessid();

	grabthreadglobals ();
	
	if (!frontierstart ()) {
		
		shellshutdown ();
		
		return (false);
		}

	findreplacewindow = NULL;  /*just ensure that this is NULL */


	wndprocMDIClient = (WNDPROC) SetWindowLong (hwndMDIClient, GWL_WNDPROC, (long)FrontierMDIWndProc);

	#if (FRONTIERWEB==1)
		GetClientRect (shellframewindow, &r);

		hwndHTMLControl = CreateMDIWindow (szChildHTML, NULL, WS_CHILD , 0, 0,
			r.right, r.bottom, hwndMDIClient, hInst, 0);

		if (langrunstringnoerror ("\x26" "system.callbacks.htmlControlStartup ()", bsresult)) { /*7.0b39 PBS: call system.callbacks.htmlControlStartup*/
			nullterminate (bsresult);
			initialURL = bsresult+1;
			}
		else {
			initialURL = "about:blank";
			}

/*
hwndStatus = GetDlgItem(m_hwnd, ID_STATUSBAR);
GetWindowRect(hwndStatus, &rectStatus);
statusHeight = rectStatus.bottom - rectStatus.top;
MoveWindow( hwndStatus, 
            0,
            height - statusHeight,
            width,
            statusHeight,
            TRUE);

if(m_pContainer)
   m_pContainer->setLocation(0, 0, width, height-statusHeight);
*/

		if ((*(gwebappInfo.webappSetup))(hInst, hwndHTMLControl, hwndStatus, initialURL))
			gwebappInfo.flactive = true;

	#endif

    /* Display the frame window */
    ShowWindow (shellframewindow, nCmdShow);
    UpdateWindow (shellframewindow);

 //   ShowWindow (hwndHTMLControl, nCmdShow);
	ShowWindow (hwndHTMLControl, SW_SHOWNOACTIVATE); /*7.0 RAB: fix for minimized HTML control window*/

    UpdateWindow (hwndHTMLControl);

    /* if we allocated a buffer then free it */
    if (pCmdLine) {

		LocalFree((LOCALHANDLE) pCmdLine);

		pCmdLine = NULL;
		}

	releasethreadglobals ();

	flinstanceinitialized = true;

    return TRUE;
		UNREFERENCED_PARAMETER(hmenu);
		UNREFERENCED_PARAMETER(hdc);
	} /*InitializeInstance*/



BOOL WinProcessMouseEvent (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	EventRecord ev;
	POINT winpt;

	ev.hwnd = hwnd;
	ev.winmsg = msg;
	ev.wparam = wParam;
	ev.lparam = lParam;
	ev.part = HTCLIENT;

	ev.message = (long) hwnd;
	ev.when = gettickcount ();

	switch (msg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			ev.what = mouseDown;
			ev.where.h = LOWORD(lParam);
			ev.where.v = HIWORD(lParam);
			ev.modifiers = wParam;
			break;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			ev.what = rmouseDown;
			ev.where.h = LOWORD(lParam);
			ev.where.v = HIWORD(lParam);
			ev.modifiers = wParam;
			break;

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			ev.what = cmouseDown;
			ev.where.h = LOWORD(lParam);
			ev.where.v = HIWORD(lParam);
			ev.modifiers = wParam;
			break;

		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONDBLCLK:
			ev.what = mouseDown;
			ev.part = wParam;

			winpt.x = LOWORD(lParam);
			winpt.y = HIWORD(lParam);

			ScreenToClient (hwnd, &winpt);

			ev.where.h = (short) winpt.x;
			ev.where.v = (short) winpt.y;

			ev.modifiers = 0;
			break;

		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONDBLCLK:
			ev.what = rmouseDown;
			ev.part = wParam;

			winpt.x = LOWORD(lParam);
			winpt.y = HIWORD(lParam);

			ScreenToClient (hwnd, &winpt);

			ev.where.h = (short) winpt.x;
			ev.where.v = (short) winpt.y;

			ev.modifiers = 0;
			break;

		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONDBLCLK:
			ev.what = cmouseDown;
			ev.part = wParam;

			winpt.x = LOWORD(lParam);
			winpt.y = HIWORD(lParam);

			ScreenToClient (hwnd, &winpt);

			ev.where.h = (short) winpt.x;
			ev.where.v = (short) winpt.y;

			ev.modifiers = 0;
			break;

		default:
			ev.what = mouseDown;
			ev.where.h = LOWORD(lParam);
			ev.where.v = HIWORD(lParam);
			ev.modifiers = wParam;
			break;
		}

	return (shellprocessevent (&ev));
	}

BOOL WinProcessKeyboardEvent (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	EventRecord ev;

	ev.hwnd = hwnd;
	ev.winmsg = msg;
	ev.wparam = wParam;
	ev.lparam = lParam;
	ev.part = HTCLIENT;

	ev.what = keyDown;
	ev.message = wParam;
	ev.when = gettickcount ();
	ev.where.h = 0;
	ev.where.v = 0;
	ev.modifiers = lParam;
	return (shellprocessevent (&ev));
	}

BOOL WinProcessPaintEvent (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	EventRecord ev;

	ev.hwnd = hwnd;
	ev.winmsg = msg;
	ev.wparam = wParam;
	ev.lparam = lParam;
	ev.part = HTCLIENT;

	ev.what = updateEvt;
	ev.message = (long) hwnd;
	ev.when = gettickcount ();
	ev.where.h = 0;
	ev.where.v = 0;
	ev.modifiers = wParam;
	return (shellprocessevent (&ev));
	}

BOOL WinProcessCommandEvent (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	EventRecord ev;

	ev.hwnd = hwnd;
	ev.winmsg = msg;
	ev.wparam = wParam;
	ev.lparam = lParam;
	ev.part = HTCLIENT;

	ev.what = menuEvt;
	ev.message = (long) hwnd;
	ev.when = gettickcount ();
	ev.where.h = 0;
	ev.where.v = 0;
	ev.modifiers = (((LOWORD(wParam) / 100) * 100) << 16) + (LOWORD(wParam));
	
	keyboardpeek (&keyboardstatus);
	
	return (shellprocessevent (&ev));
	}

BOOL WinProcessScrollEvent (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, boolean isvert)
	{
	EventRecord ev;

	ev.hwnd = hwnd;
	ev.winmsg = msg;
	ev.wparam = wParam;
	ev.lparam = lParam;
	ev.part = HTCLIENT;

	ev.what = scrollEvt;
	ev.message = (long) hwnd;
	ev.when = gettickcount ();
	ev.where.h = HIWORD (wParam);
	ev.where.v = LOWORD (wParam);
	ev.modifiers = isvert;
	
	keyboardpeek (&keyboardstatus);
	
	return (shellprocessevent (&ev));
	}

BOOL WinProcessActivateEvent (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	EventRecord ev;

	ev.hwnd = hwnd;
	ev.winmsg = msg;
	ev.wparam = wParam;
	ev.lparam = lParam;
	ev.part = HTCLIENT;

	ev.what = activateEvt;
	ev.message = (long) hwnd;
	ev.when = gettickcount ();
	ev.where.h = 0;
	ev.where.v = 0;
	if (GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wParam, lParam))
		ev.modifiers = activeFlag;
	else
		ev.modifiers = 0;

	return (shellprocessevent (&ev));
	}

BOOL WinProcessNCActivateEvent (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	EventRecord ev;

	ev.hwnd = hwnd;
	ev.winmsg = msg;
	ev.wparam = wParam;
	ev.lparam = lParam;
	ev.part = HTCLIENT;

	ev.what = activateEvt;
	ev.message = (long) hwnd;
	ev.when = gettickcount ();
	ev.where.h = 0;
	ev.where.v = 0;
	if (wParam)
		ev.modifiers = activeFlag;
	else
		ev.modifiers = 0;

	return (shellprocessevent (&ev));
	} /*WinProcessNCActivateEvent*/


BOOL WinProcessActivateAppEvent (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	/*
	5.0a22 dmb: added
	
	2006-04-17 aradke: Removed tyjugglermessage for endianness-agnostic code.
		See shellhandlejugglerevent in shelljuggler.c.
	*/

	EventRecord ev;

	ev.hwnd = hwnd;
	ev.winmsg = msg;
	ev.wparam = wParam;
	ev.lparam = lParam;
	
	ev.message = 0;
	ev.message |= (suspendResumeMessage << 24);
	ev.message |= ((wParam != 0) ? resumeFlag : 0);

	ev.what = jugglerEvt;
	ev.when = gettickcount ();
	ev.where.h = 0;
	ev.where.v = 0;

	return (shellprocessevent (&ev));
	} /*WinProcessActivateAppEvent*/


static BOOL WinMapSpecialKey (WPARAM *wParam) {

	switch (*wParam) {
	
		case VK_LEFT:
			*wParam = chleftarrow;
			break; 
	
		case VK_RIGHT:
			*wParam = chrightarrow;
			break; 
	
		case VK_UP:
			*wParam = chuparrow;
			break; 
	
		case VK_DOWN:
			*wParam = chdownarrow;
			break; 
	
		case VK_PRIOR:
			*wParam = chpageup;
			break;
		
		case VK_NEXT:
			*wParam = chpagedown;
			break;
	
		case VK_HOME:
			*wParam = chhome;
			break; 
	
		case VK_END:
			*wParam = chend;
			break; 
	
		case VK_INSERT:
			*wParam = chleftarrow;
			break; 
	
		case VK_DELETE:
			*wParam = chdelete;
			break; 
	
		case VK_F1:
			*wParam = chhelp;
			break; 
	
		case VK_F2:
			*wParam = chenter;
			break; 
		
		default:
			return FALSE;
		}
	
	return TRUE;
	} /*WinMapSpecialKey*/


/****************************************************************************
 *                                                                          *
 *  FUNCTION   : QueryCloseAllChildren()                                    *
 *                                                                          *
 *  PURPOSE    : Asks the child windows if it is ok to close up app. Nothing*
 *               is destroyed at this point. The z-order is not changed.    *
 *                                                                          *
 *  RETURNS    : TRUE - If all children agree to the query.                 *
 *               FALSE- If any one of them disagrees.                       *
 *                                                                          *
 ****************************************************************************/

BOOL QueryCloseAllChildren() {
	boolean fl;
	
	grabthreadglobals ();
	
	keyboardpeek (&keyboardstatus);
	
	fl = shellquit ();
	
	releasethreadglobals ();

	return (fl);
	}


/****************************************************************************
 *                                                                          *
 *  FUNCTION   : QueryCloseChild (hwnd)                                     *
 *                                                                          *
 *  PURPOSE    : If the child MDI is unsaved, allow the user to save, not   *
 *               save, or cancel the close operation.                       *
 *                                                                          *
 *  RETURNS    : TRUE  - if user chooses save or not save, or if the file   *
 *                       has not changed.                                   *
 *               FALSE - otherwise.                                         *
 *                                                                          *
 ****************************************************************************/

BOOL QueryCloseChild(register HWND hwnd)
	{
	boolean fl;
	
	grabthreadglobals ();
	
	keyboardpeek (&keyboardstatus);
	
	if (keyboardstatus.floptionkey)
		fl = shellcloseall (getfrontwindow (), true);
	else
		fl = shellclose (getfrontwindow (), true);
	
	releasethreadglobals ();

	return (fl);
	}




LPSTR GetCmdLine (void) {

    LPSTR lpCmdLine, lpT;

#if defined(_MAC)
	lpCmdLine =  NULL;
#else
    lpCmdLine = GetCommandLine();
#endif

    // on Win32, lpCmdLine's first string includes its own name, remove this
    // to make it exactly like the windows command line.

    if (*lpCmdLine) {
        lpT = strchr(lpCmdLine, ' ');   // skip self name
        if (lpT) {
            lpCmdLine = lpT;
            while (*lpCmdLine == ' ') {
                lpCmdLine++;            // skip spaces to end or first cmd
				}
			}
		else {
            lpCmdLine += strlen(lpCmdLine);   // point to NULL
			}
		}
    return(lpCmdLine);
	} /*GetCmdLine*/


boolean openwindowsparamlinefiles (void) {

	/*
	7.0b26 PBS: updated with two fixes from Bob:

	1. Strip leading and trailing quotes in the path -- fixes Win2K bug.

	2. Get full path name of file instead of 8.3 name -- fixes Win98/NT4 bug.

	8.0b46: Deal with folder "" not found errors when person double-clicks on the
	app and it's already running.
	*/

	bigstring bspath;
	bigstring bsnewfile;
	tyfilespec fs;
	short len;
	LPTSTR lpFilePart;
	char buffer[1001];
	DWORD newLen;
	WIN32_FIND_DATA info;
	HANDLE hinfo;

	if (pCmdLine) {

		if (strcmpi (pCmdLine, "-Embedding") == 0)
			return (false);
		
		copyctopstring (pCmdLine, bspath);

		/*7.0b26: Delete leading and trailing quotes.*/
		
		if ((lastchar(bspath) == '"') && (getstringcharacter (bspath, 0) == '"')) {

			setstringlength (bspath, stringlength(bspath) - 1); /*trailing "*/

			deletestring (bspath, 1, 1); /*leading "*/
			}

		len = stringlength (bspath);

		if (len == 0) { /*8.0b46: user has double-clicked on the app icon*/
				
				bigstring bsresult;
	
				grabthreadglobals ();

				langrunstringnoerror ("\x28" "system.callbacks.systemTrayIcon2Click ()", bsresult);

				releasethreadglobals ();
				
				#ifndef PIKE
					ShowWindow (shellframewindow, SW_SHOW); /*Show the window.*/

					activateapplication (NULL); /*Bring to front*/
				#endif

				return (true);
			} /*if*/

		nullterminate (bspath);
		
		/*7.0b26: Get full path name, not 8.3 name.*/

		newLen = GetFullPathName (stringbaseaddress(bspath), 1000, buffer, &lpFilePart);

		if ((newLen > 0) && (newLen < 1000)) {
			hinfo = FindFirstFile (buffer, &info);

			if (hinfo != INVALID_HANDLE_VALUE) {
				*lpFilePart = 0;
				copyctopstring (buffer, bspath);

				copyctopstring (info.cFileName, bsnewfile);

				pushstring (bsnewfile, bspath);

				FindClose (hinfo);
				}
			}
			
		pathtofilespec (bspath, &fs);
		
		return (shellopenfile (&fs, false, nil));
		}

	return (false);
	} /*openwindowsparamlinefiles*/


/*boolean openwindowsparamlinefiles (void) {

	bigstring bspath;
	tyfilespec fs;
	short len;
	boolean flquoteatbeginning = false;
	boolean flquoteatend = false;

	if (pCmdLine) {

		if (strcmpi (pCmdLine, "-Embedding") == 0)
			return (false);
		
		copyctopstring (pCmdLine, bspath);

		len = stringlength (bspath);

		pathtofilespec (bspath, &fs);
		
		return (shellopenfile (&fs, false, nil));
		}

	return (false);
	} /*openwindowsparamlinefiles*/

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : FrontierFrameWndProc (hwnd, msg, wParam, lParam )                *
 *                                                                          *
 *  PURPOSE    : The window function for the "frame" window, which controls *
 *               the menu and encompasses all the MDI child windows. Does   *
 *               the major part of the message processing. Specifically, in *
 *               response to:                                               *
 *                                                                          *
 *                   WM_CREATE          : Creates and displays the "frame". *
 *                                                                          *
 *                   WM_INITMENU        : Sets up the state of the menu.    *
 *                                                                          *
 *                   WM_WININICHANGE &  : If default printer characteristics*
 *                   WM_DEVMODECHANGE     have been changed, reinitialises  *
 *                                        printer DC.                       *
 *                                                                          *
 *                   WM_COMMAND         : Passes control to a command-      *
 *                                        handling function.                *
 *                                                                          *
 *                   WM_CLOSE           : Quits the app. if all the child   *
 *                                        windows agree.                    *
 *                                                                          *
 *                   WM_QUERYENDSESSION : Checks that all child windows     *
 *                                        agree to quit.                    *
 *                                                                          *
 *                   WM_DESTROY         : Destroys frame window and quits   *
 *                                        app.                              *
 *                                                                          *
 ****************************************************************************/
LONG CALLBACK FrontierFrameWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	
	/*
	5.0.2 dmb/rab: grabthreadglobals for frontieropenfile code

	5.0.2b20 dmb: don't create new find/replace window if one is already open

	5.1.5b7 dmb: don't accept frontieropenfile if we're not done initializing
	*/
	
	if (msg == wm_frontieropenfile) {
		
		if (lParam) {
			bigstring inputFile;
			int n;
			boolean flinitted = flinstanceinitialized;

			getProfileString ("\x0b" "DeskTopFile", inputFile);

			n = stringlength (inputFile);
			
			pCmdLine = (CHAR *) LocalAlloc(LPTR, n + 1);

			if (pCmdLine) {
				
				if (flinitted)
					grabthreadglobals ();
				
				copyptocstring (inputFile, pCmdLine);
				
				activateapplication (NULL);
				
				if (flinitted)
					openwindowsparamlinefiles ();
				
				LocalFree((LOCALHANDLE) pCmdLine);
				
				pCmdLine = NULL;
				
				if (flinitted)
					releasethreadglobals ();
				}
			}
		
		return 0;
		}

	if (msg == wm_startsearch) {

		if (findreplacewindow == NULL)
			findreplacewindow = FindText ((FINDREPLACE *) lParam);
		else
			windowbringtofront (findreplacewindow);

		return (0);
		}

	if (msg == wm_startreplace) {

		if (findreplacewindow == NULL)
			findreplacewindow = ReplaceText ((FINDREPLACE *) lParam);
		else
			windowbringtofront (findreplacewindow);

		return (0);
		}


	if (msg == wm_findmsgstring) {

		shellprocessfindmessage ((FINDREPLACE *) lParam);

		return (0);
		}

	
    switch (msg){
/*		case WM_WINDOWPOSCHANGING:
			{
			WINDOWPOS * wp;

			if (hwndMDIClient != NULL) {

				if (hwnd == hwndMDIClient) {

				wp = (WINDOWPOS *) lParam;

				wp->flags |= SWP_NOZORDER;
					}
				}

			#if (FRONTIERWEB == 1)
				doweb (hwnd, msg, wParam, lParam);
			#endif

            return DefFrameProc (hwnd,hwndMDIClient,msg,wParam,lParam);
			}
*/

		case 0x020A: { /*WM_MOUSEWHEEL:*/

			short delta;
			WindowPtr w = getfrontwindow ();
			tydirection dir = down;

			if (w != nil) {
				
				delta = (short) HIWORD (wParam);

				delta = delta / 120; /*WHEEL_DELTA == 120*/

				if (delta < 0)
					dir = up;

				grabthreadglobals ();
	
				shellpushglobals (w);

				(*shellglobals.scrollroutine) (dir, false, abs (delta));

				shellpopglobals ();

				releasethreadglobals ();
				} /*if*/

			break;
			} 

//#ifdef PIKE
		case FWM_SYSTRAYICON:
			{
		//	RECT r;
	
			if (lParam == WM_LBUTTONDBLCLK) {

				bigstring bsresult;
	
				grabthreadglobals ();

				langrunstringnoerror ("\x28" "system.callbacks.systemTrayIcon2Click ()", bsresult);

				releasethreadglobals ();
				
				#ifndef PIKE
					ShowWindow (shellframewindow, SW_SHOW); /*Show the window.*/

					activateapplication (NULL); /*Bring to front*/
				#endif
				}

			if (lParam == WM_RBUTTONUP) {

				grabthreadglobals ();

				rundockmenu ();

				releasethreadglobals ();
				}
				
			return DefFrameProc (hwnd,hwndMDIClient,msg,wParam,lParam);
			}
//#endif


		case WM_SIZE:
			{
			long statusHeight;
			unsigned short width;
			unsigned short height;

			width = LOWORD(lParam);

			height = HIWORD(lParam);

			statusHeight = getstatusbarheight();

			if (hwndStatus != NULL) {
				MoveWindow( hwndStatus, 
					0,
					height - statusHeight,
					width,
					statusHeight,
					TRUE);

				setDefinedStatusBarParts (width);
				}

			if (hwndMDIClient != NULL) {
			
				SetWindowPos (hwndMDIClient, HWND_BOTTOM, 0, 0, width, height - statusHeight, SWP_NOZORDER);
				}

			return DefFrameProc (hwnd,hwndMDIClient,msg,wParam,lParam);
			}

        case WM_CREATE:
			{
            CLIENTCREATESTRUCT ccs;
            HDC hdc;

            /* Find window menu where children will be listed */
            ccs.hWindowMenu = GetSubMenu (GetMenu(hwnd),WINDOWMENU);
            ccs.idFirstChild = IDM_WINDOWCHILD;

			hwndStatus = CreateStatusWindow( WS_CHILD | 
                                    WS_VISIBLE | 
                                    WS_CLIPSIBLINGS | 
                                    CCS_BOTTOM | 
                                    SBARS_SIZEGRIP ,
									NULL,
									hwnd,
									1001);

            /* Create the MDI client filling the client area */
            hwndMDIClient = CreateWindow ("mdiclient",
                                          NULL,
                                          WS_CHILD | WS_CLIPCHILDREN |
                                          /*WS_VSCROLL | WS_HSCROLL | */MDIS_ALLCHILDSTYLES,
                                          0,
                                          0,
                                          0,
                                          0,
                                          hwnd,
                                          (HMENU)0xCAC,
                                          hInst,
                                          (LPSTR)&ccs);


            ShowWindow (hwndMDIClient,SW_SHOW);

            /* Check if printer can be initialized */
            if (hdc = GetPrinterDC (TRUE)){
                DeleteDC (hdc);
				}
			
		    hwndNextViewer = SetClipboardViewer (hwnd);
			
			if (hwndNextViewer == NULL)
				if (GetLastError() != ERROR_SUCCESS)
					hwndNextViewer = (HWND) -1;
			
			#if (FRONTIERWEB == 1)
//				doweb (hwnd, msg, wParam, lParam);
			#endif

			break;
			}

		//  case WM_INITMENU:
				/* Set up the menu state */
		//      InitializeMenu ((HMENU)wParam);
		//		shellinitmenus();
		//      break;

        case WM_WININICHANGE:
		case WM_DEVMODECHANGE:
			{
				/*  If control panel changes default printer characteristics,
				 *  reinitialize our printer information...
				 */
				HDC hdc;
	
				if (hdc = GetPrinterDC (TRUE))
				{
					DeleteDC (hdc);
				}
				break;
			}
			break;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint (hwnd, &ps);
			EndPaint (hwnd, &ps);
			}
			break;

        case WM_COMMAND:
            /* Direct all menu selection or accelerator commands to another
             * function
             */
	 	//	setport(GetDC(hwndActive));
			WinProcessCommandEvent (hwndActive, msg, wParam, lParam);
		//	ReleaseDC(hwndActive, getport());
		//	setport (NULL);

			#if (FRONTIERWEB == 1)
//				doweb (hwnd, msg, wParam, lParam);
			#endif

			return DefFrameProc (hwnd,hwndMDIClient,msg,wParam,lParam);
            break;

        case WM_CLOSE: 

//#ifdef PIKE
			ShowWindow (hwnd, SW_HIDE);
			break;
//#else



/*			grabthreadglobals ();
			
			keyboardpeek (&keyboardstatus);

			#ifdef PIKE

				if (pikequit ())

					shellquit ();

			#else
			
				shellquit();

			#endif
			
			releasethreadglobals ();

			#if (FRONTIERWEB == 1)
//				doweb (hwnd, msg, wParam, lParam);
			#endif

            break;
//#endif*/

        case WM_QUERYENDSESSION:
            /*  Before session ends, check that all files are saved */
            return QueryCloseAllChildren ();

        case WM_DESTROY: {
			WINDOWPLACEMENT w;

			//Save the Frame position
			w.length = sizeof(WINDOWPLACEMENT);

			if (GetWindowPlacement (hwnd, &w)) {
				setProfileLong ("\x06" "FrameX", w.rcNormalPosition.left);
				setProfileLong ("\x06" "FrameY", w.rcNormalPosition.top);
				setProfileLong ("\x06" "FrameW", w.rcNormalPosition.right - w.rcNormalPosition.left);
				setProfileLong ("\x06" "FrameH", w.rcNormalPosition.bottom - w.rcNormalPosition.top);
				setProfileLong ("\x0a" "FrameStyle", w.showCmd);
				}

			if (hwndNextViewer != (HWND) -1)
				ChangeClipboardChain(hwnd, hwndNextViewer);

			#if (FRONTIERWEB == 1)
//				doweb (hwnd, msg, wParam, lParam);
			#endif

			PostQuitMessage (0);
			}
            break;
		
		case WM_QUIT:
			// we never get here; the thread exits before sending it to this proc.
			// seems weird, but consistent with the docs
			break;
		
		case WM_CHANGECBCHAIN:  

			// If the next window is closing, repair the chain.
			if ((HWND) wParam == hwndNextViewer) 
				hwndNextViewer = (HWND) lParam; 

			// Otherwise, pass the message to the next link. 
			else if (hwndNextViewer != NULL)
				SendMessage(hwndNextViewer, msg, wParam, lParam); 

			break;

		case WM_DRAWCLIPBOARD:
			handlescrapdisposed (); //it needs to be refreshed

			if ((HWND) wParam == hwndNextViewer) 
				hwndNextViewer = (HWND) lParam;

			if ((hwndNextViewer != (HWND) NULL) && (lParam != (LPARAM) NULL))
				SendMessage(hwndNextViewer, msg, wParam, lParam); 

			break;

		case WM_RENDERFORMAT:
			shellexportscrap (win2shellscraptype (wParam));
			break;
		
		case WM_RENDERALLFORMATS:
			shellwritescrap (allscraptypes);
			break;

        case WM_DESTROYCLIPBOARD:
			shelldisposescrap ();
			break;

		case wm_destroycaret:
			DestroyCaret ();
			break;

		case wm_processAccept:
			fwsNetEventAcceptSocket (wParam, lParam);
			break;

		case WM_ACTIVATEAPP:
			WinProcessActivateAppEvent (hwnd, msg, wParam, lParam);
			return (0);

        default:
            /*  use DefFrameProc() instead of DefWindowProc() since there
             *  are things that have to be handled differently because of MDI
             */


			#if (FRONTIERWEB == 1)
//				doweb (hwnd, msg, wParam, lParam);
			#endif

            return DefFrameProc (hwnd,hwndMDIClient,msg,wParam,lParam);
    }
    return 0;
}


static boolean GetFrontierWindowInfo (HWND hwnd, hdlwindowinfo *hinfo) {

	if (!getwindowinfo (hwnd, hinfo))
		return (false);
	
	if (**hinfo == NULL) // dmb: I don't understand why, but it can be so 
		return (false);

	return ((***hinfo).hdata != NULL);
	} /*GetFrontierWindowInfo*/


LONG CALLBACK FrontierOPWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	hdlwindowinfo hinfo;
	LONG retval;

	MESSAGETRACKER(hwnd, msg, wParam, lParam);

    switch (msg){
        case WM_CREATE:	{
			
			ShowWindow (hwnd, SW_HIDE);
			/*
			EventRecord ev;

            SetFocus (hwnd);
				
			ev.message = hwnd;
			ev.what = activateEvt;
			ev.where.h = 0;
			ev.where.v = 0;
			if (wParam)
				ev.modifiers = activeFlag;
			else
				ev.modifiers = 0;

			shellprocessevent (&ev);
			*/  
			}
			break;

		case WM_SIZE:
			if (GetFrontierWindowInfo (hwnd, &hinfo)) {
				RECT rr;
				HDC oldone;
				
				#ifdef USINGSTATUSBAR
					//RAB: 1/20/98 - Added code to keep the status bar where it should be
					int cxParent = LOWORD (lParam);
					int cyParent = HIWORD (lParam);
					int x, y, cx, cy;
					RECT rWindow;
					HWND hwndStatusBar;

					hwndStatusBar = (HWND) GetWindowLong (hwnd, 8);

					// Keep Status Window Height the same
					GetWindowRect (hwndStatusBar, &rWindow);

					cy = rWindow.bottom - rWindow.top;

					x = 0;

					y = cyParent - cy;

					cx = cxParent;

					MoveWindow (hwndStatusBar, x, y, cx, cy, TRUE);
					//RAB: 1/20/98 end of changes
				#endif

				rr = (**hinfo).contentrect;

				if ((rr.right != LOWORD(lParam)) || (rr.bottom != HIWORD(lParam)))  {
					
					oldone = getport();
	 			//	winpushport(GetDC(hwnd));
					grabthreadglobals ();
					shellpushglobals (hwnd);

					shelladjustaftergrow (hwnd);
					
					shellpopglobals ();
					releasethreadglobals ();
				//	ReleaseDC(hwnd, getport());
					//setport (oldone);
				//	winpopport();
					}
				}
			
			retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
			MESSAGEENDER (MSGDEFAULT);
			return (retval);	
			
		case WM_SYSCOMMAND: {

			/*#ifdef PIKE /*7.0b26 PBS: standard Windows maximize behavior.*/

				hdlhashtable hprefs;
				boolean flkernelhandledmaximize = false;

#if (FRONTIERWEB == 1)
				//RAB 2/21/2001: If a window was maxed we must be sure to resize the HTML control.
				if ((wParam == SC_RESTORE) || (wParam == SC_MINIMIZE)) {
					HWND tophwnd;

					tophwnd = GetTopWindow (hwndMDIClient);

					if (IsZoomed (tophwnd)) {
						RECT r;

						if (hwndHTMLControl != NULL) {
						
							GetClientRect (hwndMDIClient, &r);

							SetWindowPos (hwndHTMLControl, HWND_BOTTOM, 0, 0, r.right, r.bottom, SWP_NOACTIVATE | SWP_NOZORDER);
							}

						}
					}
#endif

				if (wParam == SC_MAXIMIZE) { /*handle maximize*/
					grabthreadglobals ();
					shellpushglobals (hwnd);

					if (langfastaddresstotable (roottable, "\x0a" "user.prefs", &hprefs)) {

						hdlhashnode hnode;
						tyvaluerecord val;

						pushhashtable (hprefs);

						if (hashlookup ("\x15" "flwinstandardmaximize", &val, &hnode)) {

							if (copyvaluerecord (val, &val) && coercetoboolean (&val))
								
								if ((!val.data.flvalue) && (shellwindowinfo != NULL)) {

									(*shellglobals.zoomwindowroutine) (shellwindowinfo, true);
						
									flkernelhandledmaximize = true;
									} /*if*/
							} /*if*/

						pophashtable ();
						} /*if*/
						
					if ((shellwindowinfo != NULL) && (!flkernelhandledmaximize)) {
							
						keyboardpeek (&keyboardstatus); //6.0a13 dmb

						if ((**shellwindowinfo).configresnum == idaboutconfig) {
													
							(*shellglobals.zoomwindowroutine) (shellwindowinfo, true);

							flkernelhandledmaximize = true;
							} /*if*/
						} /*if*/
						
					shellpopglobals ();
					releasethreadglobals ();

					if (flkernelhandledmaximize) {
						retval = 0;
						break;
						} /*if*/
					} /*if*/

		/*	#else

				if (wParam == SC_MAXIMIZE) { /*handle maximize*/
		/*				grabthreadglobals ();
						shellpushglobals (hwnd);
						
						if (shellwindowinfo != NULL) {
							
							keyboardpeek (&keyboardstatus); //6.0a13 dmb
													
							(*shellglobals.zoomwindowroutine) (shellwindowinfo, true);
							}
						
						shellpopglobals ();
						releasethreadglobals ();
					retval = 0;
					break;
					}

			#endif*/

			retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
			MESSAGEENDER (MSGDEFAULT);
			return (retval);
			}

        case WM_MDIACTIVATE:
            /* If we're activating this child, remember it */
            if (GET_WM_MDIACTIVATE_FACTIVATE(hwnd, wParam, lParam)){
                hwndActive     = hwnd;
				}
            else{
                hwndActive     = NULL;
				}

			if (GetFrontierWindowInfo (hwnd, &hinfo)) {

	 		//	winpushport(GetDC(hwnd));

				WinProcessActivateEvent (hwnd, msg, wParam, lParam);
				
			//	ReleaseDC(hwnd, getport());
			//	winpopport ();
				}

			break;

        case WM_NCACTIVATE:
			if (wParam && GetFrontierWindowInfo (hwnd, &hinfo)) {

	 		//	winpushport(GetDC(hwnd));
				
				WinProcessNCActivateEvent (hwnd, msg, wParam, lParam);
				
			//	ReleaseDC(hwnd, getport());
			//	winpopport ();
				}

			retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
			MESSAGEENDER (MSGDEFAULT);
			return (retval);			

		case WM_ACTIVATEAPP:
			WinProcessActivateAppEvent (hwnd, msg, wParam, lParam);
			return (0);

        case WM_QUERYENDSESSION:
            /* Prompt to save the child */
            retval = !QueryCloseChild (hwnd);
			MESSAGEENDER (MSGOTHER);
			return (retval);
        
		case WM_CLOSE:
            /* If its OK to close the child, do so, else ignore */
            if (QueryCloseChild (hwnd))
				{
				retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
				MESSAGEENDER (MSGDEFAULT);
				return (retval);			
				}
            else
                break;

        case WM_SETFOCUS:
			/*  See of editor needs focus  **/
			retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
			MESSAGEENDER (MSGDEFAULT);
			return (retval);			

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		//	setport(GetDC(hwnd));

			setmousedoubleclickstatus(false);
			WinProcessMouseEvent (hwnd, msg, wParam, lParam);
		//	ReleaseDC(hwnd, getport());
		//	setport (NULL);
			break;

		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		//	setport(GetDC(hwnd));
			setmousedoubleclickstatus(true);
			WinProcessMouseEvent (hwnd, msg, wParam, lParam);
		//	ReleaseDC(hwnd, getport());
		//	setport (NULL);
			break;
		
		// For the normal left mouse button do not trap or otherwise interfere
		// with the normal NC behavior.
		//case WM_NCLBUTTONDOWN:
		//case WM_NCLBUTTONDBLCLK:

	
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			if ((wParam == HTERROR) || (wParam == HTNOWHERE) || (wParam == HTTRANSPARENT)) {
				retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
				MESSAGEENDER (MSGDEFAULT);
				return (retval);			
				}

			WinProcessMouseEvent (hwnd, msg, wParam, lParam);
			break;

		case WM_NCRBUTTONDBLCLK:
		case WM_NCMBUTTONDBLCLK:
			if ((wParam == HTERROR) || (wParam == HTNOWHERE) || (wParam == HTTRANSPARENT)) {
				retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
				MESSAGEENDER (MSGDEFAULT);
				return (retval);			
				}

			setmousedoubleclickstatus(true);
			WinProcessMouseEvent (hwnd, msg, wParam, lParam);
			break;

		case WM_KEYDOWN:
			if (!WinMapSpecialKey (&wParam))
				{
				retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
				MESSAGEENDER (MSGDEFAULT);
				return (retval);			
				}
			// fall through to WM_CHAR case with updated wParam

		case WM_CHAR:
// 5.0a24 rab: why are we trapping this - blocks Alt-space for menu etc.		case WM_SYSCHAR:
		//	setport(GetDC(hwnd));
			WinProcessKeyboardEvent (hwnd, msg, wParam, lParam);
		//	ReleaseDC(hwnd, getport());
		//	setport (NULL);
			break;

		case WM_PAINT:
			if (GetFrontierWindowInfo (hwnd, &hinfo)) { // window is ready for updating
			//	PAINTSTRUCT ps;
			//	setport(BeginPaint (hwnd, &ps));
				WinProcessPaintEvent (hwnd, msg, wParam, lParam);
			//	ValidateRect (hwnd, NULL);
			//	EndPaint (hwnd, &ps);
			//	setport (NULL);
			//	ValidateRect (hwnd, NULL);
				break;
				}

			retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
			MESSAGEENDER (MSGDEFAULT);
			return (retval);			

        case WM_COMMAND:
		//	setport(GetDC(hwnd));
			WinProcessCommandEvent (hwnd, msg, wParam, lParam);
		//	ReleaseDC(hwnd, getport());
		//	setport (NULL);
			break;

        case WM_HSCROLL:
		//	setport(GetDC(hwnd));
			WinProcessScrollEvent (hwnd, msg, wParam, lParam, false);
		//	ReleaseDC(hwnd, getport());
		//	setport (NULL);
			break;

        case WM_VSCROLL:
		//	setport(GetDC(hwnd));
			WinProcessScrollEvent (hwnd, msg, wParam, lParam, true);
		//	ReleaseDC(hwnd, getport());
		//	setport (NULL);
			break;

        default:
            /* Again, since the MDI default behaviour is a little different,
             * call DefMDIChildProc instead of DefWindowProc()
             */
			retval = DefMDIChildProc (hwnd, msg, wParam, lParam);
			MESSAGEENDER (MSGDEFAULT);
			return (retval);			
    }

	MESSAGEENDER (FALSE);
	return FALSE;
}

#if (MEMTRACKER == 1)
extern void displaymemtrack ();
#endif


#if (FRONTIERCOM == 1)
typedef int (WINAPI * tycominit) (HINSTANCE hInstance, 
                              HINSTANCE hPrevInstance,
                              LPSTR lpCmdLine, 
                              int nCmdShow, XDLLProcTable * calltable);

typedef void (WINAPI * tycomclear) ();

//extern int WINAPI comInit(HINSTANCE hInstance, 
//                              HINSTANCE hPrevInstance,
//                              LPSTR lpCmdLine, 
//                              int nCmdShow, XDLLProcTable * calltable);
//extern int comClear ();
typedef struct tyComServerInfo {
	tycominit cominit;
	tycomclear comclear;
	XDLLProcTable  calltable;
	HANDLE hCOMModule;
	HINSTANCE   hInstance;
	HINSTANCE   hPrevInstance;
	LPSTR       lpCmdLine;
	int         nCmdShow;
	} tyComServerInfo;

tyComServerInfo gcomServerInfo;

Handle COMSYSModule() {
	return (gcomServerInfo.hCOMModule);
	} /*COMSYSModule*/


boolean initCOM ()
{
	if ( !flcominitialized )
	{
		LPVOID reserved = NULL;
		HRESULT err;
	
		err = CoInitialize( reserved );
	
		if ( SUCCEEDED( err ) )
		{
			flcominitialized = true;
			return (true);
		}
		else
			return (false);
	}

	return (true);
}

void shutdownCOM ()
{
	CoUninitialize();
	flcominitialized = false;
}


Handle COMStartup () {

	initCOM();

	gcomServerInfo.hCOMModule = LoadLibrary ("COMDLL.DLL");
	
	if (gcomServerInfo.hCOMModule == NULL)
		gcomServerInfo.hCOMModule = LoadLibrary ("DLLS\\COMDLL.DLL");
		
	if (gcomServerInfo.hCOMModule == NULL)
		gcomServerInfo.hCOMModule = LoadLibrary ("..\\DLLS\\COMDLL.DLL");

	if (gcomServerInfo.hCOMModule != NULL) {
		gcomServerInfo.cominit = (tycominit) GetProcAddress (gcomServerInfo.hCOMModule, "comInit");
		gcomServerInfo.comclear = (tycominit) GetProcAddress (gcomServerInfo.hCOMModule, "comClear");

		if ((gcomServerInfo.cominit != NULL) && (gcomServerInfo.comclear != NULL)) {

			fillcalltable(&gcomServerInfo.calltable);
			
			(*(gcomServerInfo.cominit)) (gcomServerInfo.hInstance, gcomServerInfo.hPrevInstance, gcomServerInfo.lpCmdLine, gcomServerInfo.nCmdShow, &gcomServerInfo.calltable);
			}
		}

	return (gcomServerInfo.hCOMModule);
	} /*COMStartup*/

boolean COMShutdown () {
	shutdownCOM();

	if ((gcomServerInfo.cominit != NULL) && (gcomServerInfo.comclear != NULL))
		(*(gcomServerInfo.comclear)) ();

	if (gcomServerInfo.hCOMModule != NULL)
		FreeLibrary (gcomServerInfo.hCOMModule);

	gcomServerInfo.cominit = NULL;
	gcomServerInfo.comclear = NULL;
	gcomServerInfo.hCOMModule = NULL;

	return (true);
	} /*COMShutdown*/

#endif

//#ifdef PIKE

	/* 9/24/01 RAB */
	#define PACKVERSION(major,minor) MAKELONG(minor,major)

	DWORD GetDllVersion(LPCTSTR lpszDllName)
		{

		HINSTANCE hinstDll;
		DWORD dwVersion = 0;

		hinstDll = LoadLibrary(lpszDllName);
		
		if(hinstDll)
			{
			DLLGETVERSIONPROC pDllGetVersion;

			pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

	/*Because some DLLs might not implement this function, you
	  must test for it explicitly. Depending on the particular 
	  DLL, the lack of a DllGetVersion function can be a useful
	  indicator of the version.
	*/
			if(pDllGetVersion)
				{
				DLLVERSIONINFO dvi;
				HRESULT hr;

				ZeroMemory(&dvi, sizeof(dvi));
				dvi.cbSize = sizeof(dvi);

				hr = (*pDllGetVersion)(&dvi);

				if(SUCCEEDED(hr))
					{
					dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
					}
				}
        
			FreeLibrary(hinstDll);
			}
		return dwVersion;
		}

//#endif

int CALLBACK WinMain (
		HINSTANCE   hInstance,
		HINSTANCE   hPrevInstance,
		LPSTR       lpCmdLine,
		int         nCmdShow
		)
	{

//#ifdef PIKE

	NOTIFYICONDATA statusIconData;

//#endif

#if (FRONTIERCOM == 1)
		gcomServerInfo.hInstance = hInstance;
		gcomServerInfo.hPrevInstance = hPrevInstance;
		gcomServerInfo.lpCmdLine = lpCmdLine;
		gcomServerInfo.nCmdShow = nCmdShow;

		COMStartup();
	#endif

	#if (FRONTIERWEB == 1)
		webappStartup ();

		OleInitialize(NULL);

		InitCommonControls();
	#endif

	#ifdef PIKE // 7.0b26 PBS

		#if defined ( IUDADESKTOP ) || defined ( OPMLEDITOR )

			#ifdef IUDADESKTOP  // 2007-10-22 creedon

				wm_frontieropenfile = RegisterWindowMessage ("IudaDesktopOpenFile");

			#endif // IUDADESKTOP

			#ifdef OPMLEDITOR  // 2005-04-06 dluebbert

				wm_frontieropenfile = RegisterWindowMessage ("OpmlOpenFile");

			#endif // OPMLEDITOR

		#else

			wm_frontieropenfile = RegisterWindowMessage ("PikeOpenFile");

		#endif

	#else

		wm_frontieropenfile = RegisterWindowMessage ("FrontierOpenFile");

	#endif // PIKE

	wm_findmsgstring = RegisterWindowMessage (FINDMSGSTRING);
	
	wm_startreplace = RegisterWindowMessage (sz_frontierstartreplace);
	
	wm_startsearch = RegisterWindowMessage (sz_frontierstartsearch);

	#ifdef PIKE	

		#if defined ( IUDADESKTOP ) || defined ( OPMLEDITOR )

			#ifdef IUDADESKTOP  // 2007-10-22 creedon

				frontiermutex = CreateMutex (NULL, true, "IudaDesktopInstance");

			#endif // IUDADESKTOP

			#ifdef OPMLEDITOR  // 2005-04-06 dluebbert

				frontiermutex = CreateMutex (NULL, true, "OpmlInstance");

			#endif // OPMLEDITOR

		#else

			frontiermutex = CreateMutex (NULL, true, "PikeInstance");

		#endif

	#else

		frontiermutex = CreateMutex (NULL, true, "FrontierInstance");

	#endif

	if (GetLastError () == ERROR_ALREADY_EXISTS) {
		bigstring inputFile;

		//5.0b9 rab: It is not allowed to share memory directly between processes
		//under NT so instead we just write the command line to the registry then
		//read it back when processing the openfile message.

		copyctopstring (lpCmdLine, inputFile);

		setProfileString ("\x0b" "DeskTopFile", inputFile);

//		MessageBox (NULL, lpCmdLine, "Frontier Exists", MB_OK);

		SendMessage (HWND_BROADCAST, wm_frontieropenfile, 0, (LPARAM) true);
		
		return 0;
		}
	
    hInst = hInstance;
	shellinstance = hInstance;

		/* If this is the first instance of the app. register window classes */
    if (!hPrevInstance) {
        if (!InitializeApplication ())
            return 0;
		}
	
    /* Create the frame and do other initialization */		
    if (!InitializeInstance (lpCmdLine, nCmdShow))
	    return 0;

//#ifdef PIKE
	
	if(GetDllVersion(TEXT("shell32.dll")) >= PACKVERSION(5,0))
		{
		statusIconData.cbSize = sizeof(NOTIFYICONDATA);
		}
	else
		{
		statusIconData.cbSize = sizeof(NOTIFYICONDATA);//NOTIFYICONDATA_V1_SIZE;
		}

	statusIconData.hIcon = hTrayIconApp;
	statusIconData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	statusIconData.uID = 5335;
	statusIconData.hWnd = shellframewindow;
	statusIconData.uCallbackMessage = FWM_SYSTRAYICON;

	#ifdef PIKE

		#if defined ( IUDADESKTOP ) || defined ( OPMLEDITOR )

			#ifdef IUDADESKTOP  // 2007-10-22 creedon

				strcpy ( statusIconData.szTip, "IUDA Desktop" );

			#endif // IUDADESKTOP

			#ifdef OPMLEDITOR  // 2005-04-06 dluebbert

				strcpy (statusIconData.szTip, "OPML");

			#endif // OPMLEDITOR

		#else

			strcpy (statusIconData.szTip, "Radio UserLand");

		#endif

	#else

		strcpy (statusIconData.szTip, "Frontier"); // 2005-04-02 creedon: removed UserLand

	#endif

	Shell_NotifyIcon (NIM_ADD, &statusIconData);

//#endif

    /* Enter main message loop */
	shellmaineventloop ();

//#ifdef PIKE

	Shell_NotifyIcon (NIM_DELETE, &statusIconData);

//#endif

	#if (MEMTRACKER == 1)
		displaymemtrack ();
	#endif

	fwsNetEventShutDown();

	#if (FRONTIERWEB==1)
		OleUninitialize();

		webappShutdown ();

	#endif

	#if (FRONTIERCOM == 1)
		COMShutdown ();
	#endif

	#if (MESSAGETRACKSYSTEM == 1)
		fclose (errfile);
	#endif
	
	if (frontiermutex != NULL)
		verify (CloseHandle (frontiermutex));

    return 0;
}  /*WinMain*/



BOOL fAbort;            /* TRUE if the user has aborted the print job    */
HWND hwndPDlg;          /* Handle to the cancel print dialog             */
CHAR szDevice[160];     /* Contains the device, the driver, and the port */
PSTR szDriver;          /* Pointer to the driver name                    */
PSTR szPort;            /* Port, ie, LPT1                                */
PSTR szTitle;           /* Global pointer to job title                   */
INT iPrinter = 0;       /* level of available printer support.           */
                        /* 0 - no printer available                      */
                        /* 1 - printer available                         */
                        /* 2 - driver supports 3.0 device initialization */
HANDLE hInitData=NULL;  /* handle to initialization data                 */

CHAR szExtDeviceMode[] = "EXTDEVICEMODE";

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : GetPrinterDC ()                                            *
 *                                                                          *
 *  PURPOSE    : Creates a printer display context for the default device.  *
 *               As a side effect, it sets the szDevice and szPort variables*
 *               It also sets iPrinter to the supported level of printing.  *
 *                                                                          *
 *  RETURNS    : HDC   - A handle to printer DC.                            *
 *                                                                          *
 ****************************************************************************/
HDC GetPrinterDC(BOOL bInformation)
{
	HDC      hdc;
#if defined(_MAC)
	iPrinter = 0;

	if (bInformation)
		hdc = CreateIC("PRINTER", NULL, NULL, NULL);
	else
		hdc = CreateDC("PRINTER", NULL, NULL, NULL);

	if (hdc)
		iPrinter = 1;

#else

    LPDEVMODE  lpdevmode = NULL;

    iPrinter = 0;

    /* Get the printer information from win.ini into a buffer and
     * null terminate it.
     */
    GetProfileString ( "windows", "device", "" ,szDevice, sizeof(szDevice));
    for (szDriver = szDevice; *szDriver && *szDriver != ','; szDriver++)
        ;
    if (*szDriver)
        *szDriver++ = 0;

    /* From the current position in the buffer, null teminate the
     * list of ports
     */
    for (szPort = szDriver; *szPort && *szPort != ','; szPort++)
        ;
    if (*szPort)
        *szPort++ = 0;

    /* if the device, driver and port buffers all contain meaningful data,
     * proceed.
     */
    if (!*szDevice || !*szDriver || !*szPort){
        *szDevice = 0;
        return NULL;
    }

    /* Create the printer display context */
    if (hInitData){
        /* Get a pointer to the initialization data */
        lpdevmode = (LPDEVMODE) LocalLock (hInitData);

        if (lstrcmp (szDevice, (LPSTR)lpdevmode)){
            /* User has changed the device... cancel this setup, as it is
             * invalid (although if we worked harder we could retain some
             * of it).
             */
            lpdevmode = NULL;
            LocalUnlock (hInitData);
            LocalFree (hInitData);
            hInitData = NULL;
        }
    }

    if (bInformation)
      hdc = CreateIC (szDriver, szDevice, szPort, lpdevmode);
   else
      hdc = CreateDC (szDriver, szDevice, szPort, lpdevmode);

    /* Unlock initialization data */
    if (hInitData)
        LocalUnlock (hInitData);

    if (!hdc)
        return NULL;


    iPrinter = 1;

    /* Find out if ExtDeviceMode() is supported and set flag appropriately */
    if (GetProcAddress (LoadLibrary(szDriver), szExtDeviceMode))
        iPrinter = 2;

#endif

    return hdc;

}




EventAvail (unsigned short mask, EventRecord * ev)
	{
	MSG msg;
	HWND hwnd;

	hwnd = NULL;

	if ((mask & mDownMask) == mDownMask) {
		if ((PeekMessage (&msg, hwnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_RBUTTONDOWN, WM_RBUTTONDOWN, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_MBUTTONDOWN, WM_MBUTTONDOWN, PM_NOREMOVE)))
			return (true);
		}

	
	if ((mask & mUpMask) == mUpMask) {
		if ((PeekMessage (&msg, hwnd, WM_LBUTTONUP, WM_LBUTTONDBLCLK, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_RBUTTONUP, WM_RBUTTONDBLCLK, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_MBUTTONUP, WM_MBUTTONDBLCLK, PM_NOREMOVE)))
			return (true);
		}

	
	if ((mask & keyDownMask) == keyDownMask) {
		if ((PeekMessage (&msg, hwnd, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_SYSKEYDOWN, WM_SYSKEYDOWN, PM_NOREMOVE)))
			return (true);
		}


	if (((mask & keyUpMask) == keyUpMask) || ((mask & autoKeyMask) == autoKeyMask)) {
		if (PeekMessage (&msg, hwnd, WM_KEYUP, WM_KEYLAST, PM_NOREMOVE))
			return (true);
		}

	
	if ((mask & updateMask) == updateMask) {
		if (PeekMessage (&msg, hwnd, WM_PAINT, WM_PAINT, PM_NOREMOVE))
			return (true);
		}

/*	diskMask  */
		
	if ((mask & activMask) == activMask) {
		if (PeekMessage (&msg, hwnd, WM_ACTIVATE, WM_ACTIVATE, PM_NOREMOVE))
		if (PeekMessage (&msg, hwnd, WM_MDIACTIVATE, WM_MDIACTIVATE, PM_NOREMOVE))
			return (true);
		}


	if (((mask & highLevelEventMask) == highLevelEventMask) || ((mask & osMask) == osMask)){
		if ((PeekMessage (&msg, hwnd, WM_NULL, WM_ACTIVATE-1, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_ACTIVATE+1, WM_PAINT-1, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_PAINT+1, WM_KEYFIRST-1, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_KEYLAST+1, WM_COMMAND-1, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_SYSCOMMAND+1, WM_MOUSEFIRST-1, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_MOUSELAST+1, WM_MDIACTIVATE-1, PM_NOREMOVE)) ||
			(PeekMessage (&msg, hwnd, WM_MDIACTIVATE+1, WM_USER-1, PM_NOREMOVE)))
			return (true);
		}

	return (false);
	}

