
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

#ifdef MACVERSION
	#include <standard.h>
#endif

#ifdef WIN95VERSION
	#include "standard.h"
	#undef abs
	#include <stdio.h>
	#include <malloc.h>
	#include <crtdbg.h>
#endif

#include "error.h"
#include "font.h"
#include "memory.h"
#include "quickdraw.h"
#include "dialogs.h"
#include "ops.h"
#include "strings.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "shell.h"
#if (MEMTRACKER == 1)
	#include "shellprivate.h"
	#include "oplist.h"
#endif
#ifndef odbengine
	#include "icon.h"
	#include "fileloop.h"
	#include "resources.h"
	#include "frontierwindows.h"
	#include "shellhooks.h"
	#include "shellprint.h"
#endif
#include "shell.rsrc.h"
#include "process.h"
#if MACVERSION
	#include "osacomponent.h"
#endif
#include "tablestructure.h"
#include "Winland.h"
#include "odbinternal.h"
#include "winregistry.h"

#if (MEMTRACKER == 1)
	#pragma message ("***************************************MEMORY TRACKING IS ON!!***************************************")
#endif

boolean fldatabasesaveas;

boolean evaluateosascript (const tyvaluerecord *osaval, hdltreenode hparam1, bigstring bsname, tyvaluerecord *vreturned) {
	return (false);
	}

boolean isosascriptnode (hdltreenode htree, tyvaluerecord *osaval) {
	return (false);
	}

void convertToMacExtended (double foo, extended80 * tenbytebuffer) {
	unsigned char myext[10];
	unsigned char * tenbuf;
	short i;

	tenbuf = tenbytebuffer->x80;

	_asm {
		FLD foo;
		FWAIT;
		FSTP tbyte ptr myext;
		FWAIT;
		}

	for (i = 9; i >= 0; i--) {
		*tenbuf = myext[i];
		++tenbuf;
		}
	} /*convertToMacExtended*/

void convertFromMacExtended (double * foo, extended80 * tenbytebuffer) {
	unsigned char myext[10];
	unsigned char * tenbuf;
	short i;
	double getfoo;

	tenbuf = tenbytebuffer->x80;

	for (i = 9; i >= 0; i--) {
		myext[i] = *tenbuf;
		++tenbuf;
		}

	_asm {
		FLD tbyte ptr myext
		FWAIT;
		FSTP getfoo;
		FWAIT;
		}

	*foo = getfoo;
	} /*convertFromMacExtended*/


	//-----------------
///	void alertdialog (bigstring bs) {
///		printf ("%s", bs);
///		}

#ifdef __MWERKS__

	static int _debugreport(const char *msg) {

		int result;

		if (IsDebuggerPresent()) { 

			OutputDebugString(msg);
			
			result = 1;
			}

		else {
			
			int itemnum;

			itemnum = MessageBox (hwndMDIClient, msg, "Debug Message", MB_ABORTRETRYIGNORE | MB_ICONSTOP | MB_APPLMODAL);

			if (itemnum == IDABORT)
				abort();
			
			result = (itemnum == IDRETRY) ? 1 : 0;
			}
		
		return (result);
		} /*_debugreport*/
			
	#define _DEBUGMESSSAGE(msg) \
	        do { if ((1 == _debugreport(msg))) \
	                DebugBreak(); } while (0)
#endif

void Debugger () {
	
	#ifdef __MWERKS__
		_DEBUGMESSSAGE ("Debugger Exit\n");
	#else
		_RPT0 (_CRT_ERROR, "Debugger Exit\n");
	#endif
	}

void DebugStr (char * bs) {
	
	char s [256];

	copyptocstring (bs, s);

	#ifdef __MWERKS__
		_DEBUGMESSSAGE (s);
	#else
		_RPT0 (_CRT_ERROR, s);
	#endif
	}

typedef struct tymemtrack {
	long sig;
	long extra;
	struct tymemhandleblock * next;
	struct tymemhandleblock * prev;
	long line;
	long id;
	unsigned long time;
	char name[1];
	} tymemtrack, * pmemtrack;

typedef struct tymemhandleblock {
	char * memBlock;
	unsigned long memSize;
	#if (MEMTRACKER == 1)
		pmemtrack debugBlock;
	#endif
	} tymemhandleblock;

tymemhandleblock * masterHandleTable = NULL;
tymemhandleblock * masterFreeHandle = NULL;

#define handlesPerBlock 10000

long handlecounter = 0;

/**Handle block routines**/

static CRITICAL_SECTION allocsection;
static boolean allocsectioninitialized = false;


static void frontierReturnHandle (tymemhandleblock * foo) {

	//free (foo);
	
	EnterCriticalSection (&allocsection);

	foo->memBlock = (char *) masterFreeHandle;
	foo->memSize = 0xFFFFFFFF;
	masterFreeHandle = foo;
	
	LeaveCriticalSection (&allocsection);

	} /*frontierReturnHandle*/

static boolean frontierExpandHandles () {
	tymemhandleblock * newblock;
	int i;
	//should only be called when masterFreeHandle == NULL

	if (!allocsectioninitialized) {

		InitializeCriticalSection (&allocsection);

		allocsectioninitialized = true;
		}
	
	newblock = (tymemhandleblock *) malloc (handlesPerBlock * sizeof(tymemhandleblock));

	if (newblock == NULL)
		return (false);

	EnterCriticalSection (&allocsection);

	newblock->memBlock = (char *)masterHandleTable;
	newblock->memSize = handlesPerBlock * sizeof(tymemhandleblock);
	masterHandleTable = newblock;

	//Initialize the rest of the handle block to free handles
	for (i = 1; i < handlesPerBlock; i++) {
		newblock[i].memBlock = (char *) masterFreeHandle;
		newblock[i].memSize = 0xFFFFFFFF;
		masterFreeHandle = &(newblock[i]);
		}

	LeaveCriticalSection (&allocsection);

	return (true);
	} /*frontierExpandHandles*/

static tymemhandleblock * frontierGetHandle () {

	//return ((tymemhandleblock *) malloc (sizeof (tymemhandleblock)));
	
	tymemhandleblock * retVal;

	if (masterFreeHandle == NULL) { //Must allocate more handles
		if (! frontierExpandHandles())
			return (NULL);
		}

	EnterCriticalSection (&allocsection);

	retVal = masterFreeHandle;
	masterFreeHandle = (tymemhandleblock *) masterFreeHandle->memBlock;

	LeaveCriticalSection (&allocsection);

	return (retVal);
	
	} /*frontierGetHandle*/


char * CALLBACK frontierLock (Handle foo) {
	return (((tymemhandleblock *)foo)->memBlock);
	} /*frontierLock*/


#if (MEMTRACKER == 1)
tymemhandleblock * debugmasterhandle = NULL;
#endif


#if (MEMTRACKER == 1)
static void setsig (Handle foo) {
	long * fooPtr;

	fooPtr = (long *) (((tymemhandleblock *)foo)->memBlock + ((tymemhandleblock *)foo)->memSize);
	*fooPtr = 'RAB.';
	} /*setsig*/

static void checksig (Handle foo, char * doing) {
	long * fooPtr;
	char buf[500];
	pmemtrack pm;

	fooPtr = (long *) (((tymemhandleblock *)foo)->memBlock + ((tymemhandleblock *)foo)->memSize);

	if (*fooPtr != 'RAB.') {
		pm = ((tymemhandleblock *)foo)->debugBlock;
		wsprintf (buf, "DATA OVERRUN while %s: %s at line %ld for %ld bytes.  Thread id %ld. [%08lX] at %lu", doing, pm->name, pm->line, ((tymemhandleblock *)foo)->memSize, pm->id, foo, pm->time);
		MessageBox (NULL, buf, "Frontier Debug", MB_OKCANCEL);
		}
	} /*checksig*/


static void unloadbuttonlists (void) {
	
	register short i;
	register short resnum;
	
	for (i = 0; i < topglobalsarray; i++) {
		
		resnum = globalsarray [i].config.idbuttonstringlist; 
		
		if (resnum != 0) { /*a button list is attached*/
			
			opdisposelist ((hdllistrecord) globalsarray [i].buttonlist);
			}
		} /*for*/
	} /*unloadbuttonlists*/

void displaymemtrack () {
	char buf[500];
	tymemhandleblock * foo;
	long len;
	pmemtrack pm;
	long count;
	FILE * memfile;
	boolean flshow = true;
	long ctbytesneeded = 0x7fffffff;
	hdlhashtable ht = internaltable;
	
	internaltable = NULL;
	
	disposehashtable (ht, false);
	
	environmenttable = NULL;
	
	disposehashtable (environmenttable, false);

	hashflushcache (&ctbytesneeded);
	
	unloadbuttonlists ();

	shelldisposescrap ();

	wsprintf (buf, "There are %ld handle still around by count.  Display?", handlecounter);
	if (MessageBox (NULL, buf, "Fronter Debug", MB_OKCANCEL) == IDOK) {
		memfile = fopen ("memfile.txt", "w+");

		foo = debugmasterhandle;
		count = 0;
		while (foo != NULL) {
			++count;
			len = foo->memSize;		//get logical size;
			pm = foo->debugBlock;
			wsprintf (buf, "(%ld) %s at line %ld for %ld bytes.  Thread id %ld. [%08lX] at %lu", count, pm->name, pm->line, len, pm->id, foo, pm->time);

			foo = pm->next;

			fprintf (memfile, "%s\n", buf);

			if (flshow)
				if (MessageBox (NULL, buf, "Frontier Debug", MB_OKCANCEL) == IDCANCEL)
					flshow = false;
			}

		if (count != handlecounter) {
			wsprintf (buf, "Why does handlecounter show %ld while the count of linked memory blocks is %ld.", handlecounter, count);
			MessageBox (NULL, buf, "Frontier Debug", MB_OK);
			fprintf (memfile, "%s\n", buf);
			}

		fclose (memfile);

		}
	} /*displaymemtrack*/


	void CALLBACK debugfrontierFree (Handle foo) {
		tymemhandleblock * foonext;
		tymemhandleblock * fooprev;
		pmemtrack pm, pmnext, pmprev;
		
		if (foo != NULL) {
			--handlecounter;

			//unlink foo
			pm = ((tymemhandleblock *)foo)->debugBlock;

			foonext = pm->next;
			fooprev = pm->prev;

			if (foonext != NULL) {
				pmnext = foonext->debugBlock;

				pmnext->prev = fooprev;
				}

			if (fooprev != NULL) {
				pmprev = fooprev->debugBlock;

				pmprev->next = foonext;
				}

			if ((tymemhandleblock *)foo == debugmasterhandle)
				debugmasterhandle = foonext;

			checksig (foo, "Freeing");

			free (((tymemhandleblock *)foo)->memBlock);
			free (((tymemhandleblock *)foo)->debugBlock);
			frontierReturnHandle ((tymemhandleblock *)foo);
			}
		} /*debugfrontierFree*/


	Handle CALLBACK debugfrontierAlloc (char * filename, unsigned long linenumber, unsigned long threadid, long userSize) {
		pmemtrack pm;
		tymemhandleblock * foo;
		
		if (userSize < 0) // 5.0d12 dmb
			userSize = 0;

		foo = frontierGetHandle ();

		if (foo == NULL)
			return (NULL);

		foo->debugBlock = (pmemtrack) malloc (strlen(filename) + 1 + sizeof(tymemtrack));

		if (foo->debugBlock != NULL) {
			foo->memBlock = (char *) malloc (userSize+4);
			foo->memSize = userSize;

			if (foo->memBlock != NULL)
				{
				++handlecounter;

				pm = foo->debugBlock;
				pm->sig = 'RAB.';
				pm->extra = strlen(filename) + 1 + sizeof(tymemtrack);
				pm->next = debugmasterhandle;
				pm->prev = NULL;
				pm->line = linenumber;
				pm->id = threadid;
				pm->time = GetTickCount();
				strcpy (pm->name, filename);

				if (debugmasterhandle != NULL) {
					pm = debugmasterhandle->debugBlock;
					pm->prev = foo;
					}

				debugmasterhandle = foo;

				setsig ((Handle)foo);

				return ((Handle)foo);
				}

			free (foo->debugBlock);
			}

		frontierReturnHandle (foo);
		return (NULL);
		} /*debugfrontierAlloc*/

	Handle CALLBACK debugfrontierReAlloc(Handle fooIn, long userSize) {
		tymemhandleblock * foo;
		char * fooPtr;

		checksig (fooIn, "ReAllocating");

		foo = (tymemhandleblock *)fooIn;

		fooPtr = realloc (foo->memBlock, userSize+4);

		if ((fooPtr == NULL) && (userSize > 0))  
			return (NULL);

		foo->memBlock = fooPtr;  //maybe NULL if resized to zero
		foo->memSize = userSize;

		setsig (fooIn);

		SetLastError (0);	// dmb 3/6/97
		return (fooIn); /*handle never changes*/
		} /*debugfrontierReAlloc*/

Handle CALLBACK frontierAlloc (long userSize) {
	return (debugfrontierAlloc (__FILE__, __LINE__, GetCurrentThreadId(), userSize));
	} /*frontierAlloc*/

long CALLBACK frontierSize (Handle foo) {
	if (foo == NULL)
		return (0);

    return (((tymemhandleblock *)foo)->memSize);
	}


Handle CALLBACK frontierReAlloc(Handle fooIn, long userSize) {
	return (debugfrontierReAlloc(fooIn, userSize));
	}

void CALLBACK frontierFree (Handle foo) {
	debugfrontierFree(foo);
	}

#else
//***** Start Normal Routines **********

void CALLBACK frontierFree (Handle foo) {

	tymemhandleblock *f = (tymemhandleblock *)foo;

	if (f != NULL) {
		--handlecounter;

		free (f->memBlock);
		frontierReturnHandle (f);
		}
	} /*frontierFree*/

Handle CALLBACK frontierAlloc (long userSize) {
	tymemhandleblock * foo;
	
	if (userSize < 0) // 5.0d12 dmb
		userSize = 0;

	foo = frontierGetHandle ();

	if (foo == NULL)
		return (NULL);

	foo->memBlock = (char *) malloc (userSize+4);
	foo->memSize = userSize;

	if (foo->memBlock != NULL)
		{
		++handlecounter;
		return ((Handle)foo);
		}

	frontierReturnHandle (foo);
	return (NULL);
	}

long CALLBACK frontierSize (Handle foo) {

	/*
	5.0.2b21 dmb: handle disposed handles -- don't crash 
	*/

	if (foo == NULL)
		return (0);

    return (((tymemhandleblock *)foo)->memSize);
	} /*frontierSize*/


Handle CALLBACK frontierReAlloc(Handle fooIn, long userSize) {
	tymemhandleblock * foo;
	char * fooPtr;

	foo = (tymemhandleblock *)fooIn;

	fooPtr = realloc (foo->memBlock, userSize+4);

	if ((fooPtr == NULL) && (userSize > 0))  
		return (NULL);

	foo->memBlock = fooPtr;  //maybe NULL if resized to zero
	foo->memSize = userSize;

	return (fooIn); /*handle never changes*/
	}
#endif

Boolean EmptyRgn (hdlregion rgn) {
	RECT winrect;

	return (GetRgnBox ((HRGN) rgn, &winrect) == NULLREGION);
	}


/*
extern boolean getfiletype (const tyfilespec *, OSType *);
extern boolean fileresolvealias (tyfilespec *);
extern boolean sfdialog (tysfverb, bigstring, ptrsftypelist, tyfilespec *);
extern boolean getapplicationfilespec (bigstring, tyfilespec *);
*/

boolean initmacintosh ()
	{
	return (true);
	}


#ifndef odbengine

WindowPtr getcurrentwindow(){
	return (shellwindow);
	}

boolean fileresolvealias (ptrfilespec fs) {
	return (true);
	}

#endif


// dialogs.c

#ifndef odbengine

void centerdialog (HWND hwndDlg) {
	HWND hwndOwner; 
	RECT rc, rcDlg, rcOwner; 
 
     // Get the owner window and dialog box rectangles. 

    if ((hwndOwner = GetParent(hwndDlg)) == NULL) {
        hwndOwner = GetDesktopWindow(); 
		}

    GetWindowRect(hwndOwner, &rcOwner); 
    GetWindowRect(hwndDlg, &rcDlg); 
    CopyRect(&rc, &rcOwner); 

     // Offset the owner and dialog box rectangles so that 
     // right and bottom values represent the width and 
     // height, and then offset the owner again to discard 
     // space taken up by the dialog box. 

    OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
    OffsetRect(&rc, -rc.left, -rc.top); 
    OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

     // The new position is the sum of half the remaining 
     // space and the owner's original position. 

    SetWindowPos(hwndDlg, 
        HWND_TOP, 
        rcOwner.left + (rc.right / 2), 
        rcOwner.top + (rc.bottom / 2), 
        0, 0,          // ignores size arguments 
        SWP_NOSIZE); 
 	} /*centerdialog*/

#ifdef PIKE /*7.0d8 PBS*/
	static char frontierstring [] = "Radio UserLand";
#else
	static char frontierstring [] = "UserLand Frontier";
#endif

static messagebox (LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) {
	
	int itemnumber;

	releasethreadglobals ();

	itemnumber = MessageBox (hwndMDIClient, lpText, lpCaption, uType);
	
	grabthreadglobals ();

	return itemnumber;
	} /*messagebox*/


boolean msgdialog (bigstring bsprompt) {
	
	/*
	put up the standard "msg" dialog, with the provided prompt and return
	true if the user clicked on ok.  false if cancel was clicked.
	*/
	
	char s [256];
	short itemnumber;

	copyptocstring (bsprompt, s);

	itemnumber = messagebox (s, frontierstring, MB_OKCANCEL | MB_APPLMODAL);
	
	return (itemnumber == IDOK);
	} /*msgdialog*/


static unsigned char gMsgAbortButton[256];
static unsigned char gMsgRetryButton[256];
static unsigned char gMsgCancelButton[256];
static unsigned char gMsgIgnoreButton[256];
FARPROC   g_lpMsgBoxProc;          // Message box window procedure
HHOOK     g_hhookCBT;              // CBT hook identifier
static int gMessageBoxMinWidth;
static int gMessageBoxWidth;
static int gButtonWidth;
static int gButtonSpacing;
static long gbaseunits;

#define MAXBUTTONSTRINGWIDTH	11
void setbuttonstring (bigstring src, unsigned char * dest) {
	copyptocstring (src, dest);

	if (stringlength (src) > MAXBUTTONSTRINGWIDTH) {
		dest[MAXBUTTONSTRINGWIDTH-2] = '.';
		dest[MAXBUTTONSTRINGWIDTH-1] = '.';
		dest[MAXBUTTONSTRINGWIDTH] = '.';
		dest[MAXBUTTONSTRINGWIDTH+1] = '\0';
		}

	} /* setbuttonstring */

static void setmessageboxbuttonsize (int numberButtons) {
	gbaseunits = LOWORD(GetDialogBaseUnits());
	gButtonWidth = (35 * gbaseunits) / 4;
	gButtonSpacing = (5 * gbaseunits) / 4;
	gMessageBoxMinWidth = (numberButtons * (gButtonWidth + gButtonSpacing)) + gButtonSpacing;
	} /*setmessageboxbuttonsize*/

//****************************************************************************
// Function: MsgBoxSubClassProc
//
// Purpose: Subclass procedure for message box to change text and background color
//
// Parameters & return value:
//    Standard. See documentaion for WindowProc
//
//****************************************************************************
LRESULT CALLBACK MsgBoxSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
	if (msg == WM_INITDIALOG) {
		SetDlgItemText (hwnd, IDABORT, gMsgAbortButton);
		SetDlgItemText (hwnd, IDRETRY, gMsgRetryButton);
		SetDlgItemText (hwnd, IDCANCEL, gMsgCancelButton);
		SetDlgItemText (hwnd, IDIGNORE, gMsgIgnoreButton);
		}

    return  CallWindowProc((WNDPROC) g_lpMsgBoxProc, hwnd, msg, wParam, lParam);
	} /*MsgBoxSubClassProc*/

//****************************************************************************
// Function: CBTProc
//
// Purpose: Callback function of WH_CBT hook
//
// Parameters and return value:
//    See documentation for CBTProc. 
//
// Comments: The message box is subclassed on creation and the original
//    window procedure is restored on destruction
//
//****************************************************************************

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
   LPCBT_CREATEWND lpcbtcreate;
   static HWND hwndMsgBox = NULL;
   static BOOL bSubClass = FALSE;
   static int buttonCount = 0;
   
   if (nCode < 0)
       return CallNextHookEx(g_hhookCBT, nCode, wParam, lParam); 
   
   // Window owned by our task is being created. Since the hook is installed just 
	//   before the MessageBox call and removed after the MessageBox call, the window
	//   being created is either the message box or one of its controls. 
	if (nCode == HCBT_CREATEWND)     
		{
		lpcbtcreate = (LPCBT_CREATEWND)lParam;

		// Check if the window being created is a message box. The class name of
		//   a message box is WC_DIALOG since message boxes are just special dialogs.
		//   We can't subclass the message box right away because the window 
		//   procedure of the message box is not set when this hook is called. So
		//   we wait till the hook is called again when one of the message box 
		//   controls are created and then we subclass. This will happen because
		//   the message box has at least one control.
		if (WC_DIALOG == lpcbtcreate->lpcs->lpszClass) 
			{
			//This is the message box itself; make sure it is wide enough to accept the 
			// new button widths
			if (lpcbtcreate->lpcs->cx < gMessageBoxMinWidth)
				lpcbtcreate->lpcs->cx = gMessageBoxMinWidth;

			gMessageBoxWidth = lpcbtcreate->lpcs->cx;
			
			buttonCount = 0;

			hwndMsgBox = (HWND)wParam;
			bSubClass = TRUE;      // Remember to subclass when the hook is called next
			}
		else if (bSubClass)
			{
			// Subclass the dialog to change the color of the background and text
			bSubClass = FALSE;
			g_lpMsgBoxProc = (FARPROC)SetWindowLong(hwndMsgBox, GWL_WNDPROC, (long)MsgBoxSubClassProc); 
			}

		// Here we know we have initialized the dialog so we must be creating the controls, look for
		// one of the known names for the buttons to ensure we are changing the correct thing.
		// If so, reset the width and position.
		if ((hwndMsgBox != NULL) && (bSubClass == FALSE)) {
			if ((stricmp (lpcbtcreate->lpcs->lpszName, "&Retry") == 0) ||
				(stricmp (lpcbtcreate->lpcs->lpszName, "&Ignore") == 0) ||
				(stricmp (lpcbtcreate->lpcs->lpszName, "&Abort") == 0) ||
				(stricmp (lpcbtcreate->lpcs->lpszName, "Cancel") == 0)) {

				lpcbtcreate->lpcs->cx = gButtonWidth;
				lpcbtcreate->lpcs->x = ((gMessageBoxWidth - gMessageBoxMinWidth) / 2) + gButtonSpacing + (buttonCount * (gButtonWidth + gButtonSpacing));
				++buttonCount;
				}
			}
		}
	else if (nCode == HCBT_DESTROYWND && (HWND)wParam == hwndMsgBox)
		{
		// Reset the original window procedure when the message box is about to 
		//   be destroyed.
		SetWindowLong(hwndMsgBox, GWL_WNDPROC, (LONG)g_lpMsgBoxProc);
		hwndMsgBox = NULL;      
		}   
	return 0;          
	} /*CBTProc*/


boolean twowaydialog (bigstring bsprompt, bigstring bsok, bigstring bscancel) {
    int nResult;
	char s[256];
    
	copyptocstring (bsprompt, s);
	setbuttonstring (bsok, gMsgRetryButton);
	setbuttonstring (bscancel, gMsgCancelButton);
    
	setmessageboxbuttonsize (2);

	releasethreadglobals ();

	// Set a task specific CBT hook before calling MessageBox. The CBT hook will
	//    be called when the message box is created and will give us access to 
	//    the window handle of the MessageBox. The message box
	//    can then be subclassed in the CBT hook to change the color of the text and 
	//    background. Remove the hook after the MessageBox is destroyed.
	g_hhookCBT = SetWindowsHookEx(WH_CBT, CBTProc, shellinstance, 0);
	nResult = MessageBox(hwndMDIClient, s, frontierstring, MB_RETRYCANCEL | MB_APPLMODAL);
	UnhookWindowsHookEx(g_hhookCBT);
    
	grabthreadglobals ();

    return (nResult == IDRETRY);
	}


short threewaydialog (bigstring bsprompt, bigstring bsyes, bigstring bsno, bigstring bscancel) {
    int nResult;
	char s[256];
    
	copyptocstring (bsprompt, s);
	setbuttonstring (bsyes, gMsgAbortButton);
	setbuttonstring (bsno, gMsgRetryButton);
	setbuttonstring (bscancel, gMsgIgnoreButton);
    
	setmessageboxbuttonsize (3);

	releasethreadglobals ();

	// Set a task specific CBT hook before calling MessageBox. The CBT hook will
	//    be called when the message box is created and will give us access to 
	//    the window handle of the MessageBox. The message box
	//    can then be subclassed in the CBT hook to change the color of the text and 
	//    background. Remove the hook after the MessageBox is destroyed.
	g_hhookCBT = SetWindowsHookEx(WH_CBT, CBTProc, shellinstance, 0);
	nResult = MessageBox(hwndMDIClient, s, frontierstring, MB_ABORTRETRYIGNORE | MB_APPLMODAL);
	UnhookWindowsHookEx(g_hhookCBT);
    
	grabthreadglobals ();

	switch (nResult) {
		
		case IDRETRY:
			return 2;
			
		case IDIGNORE:
			return 3;

		default:
			return 1;
		}
	} /*threewaydialog*/


boolean alertdialog (bigstring bs) {
	
	char s [256];
	short itemnumber;

	copyptocstring (bs, s);

	releasethreadglobals ();

	itemnumber = messagebox (s, frontierstring, MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
	
	grabthreadglobals ();

	return (itemnumber == IDOK);
	} /*alertdialog*/


boolean alertstring (short iderror) {
	
	bigstring bs;
	
	getstringlist (alertstringlistnumber, iderror, bs);
	
	return (alertdialog (bs));
	} /*alertstring*/


short savedialog (bigstring bsfname) {
	
	/*
	returns 1, 2 or 3.
	
	if the user said yes, save the changes, the result is 1.
	
	if the user said no, discard the changes, the result is 2.
	
	if the user said cancel, continue editing, the result is 3.
	
	12/20/91 dmb: if file name is nil, just prompt for quit
	
	1/3/92 dmb: removed code for quit dialog
	*/
	
	char s [256];
	char msg [300];
	short itemnumber;

	copyptocstring (bsfname, s);
	wsprintf (msg, "Save %s before closing?", s);
	itemnumber = messagebox (msg, "save?", MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL);
	
	switch (itemnumber) {
		
		case IDYES: 
			return (1);
		
		case IDNO: 
			return (2);
		
		case IDCANCEL: 
			return (3);
		} /*switch*/
	
	return (1);
	} /*savedialog*/


short replacevariabledialog (bigstring bs) {
	
	/*
	returns 1, 2 or 3.
	
	if the user said yes, save the changes, the result is 1.
	
	if the user said no, discard the changes, the result is 2.
	
	if the user said cancel, continue editing, the result is 3.
	
	12/20/91 dmb: if file name is nil, just prompt for quit
	
	1/3/92 dmb: removed code for quit dialog
	*/
	
	char s [256];
	char msg [300];
	short itemnumber;

	copyptocstring (bs, s);
	wsprintf (msg, "Replace existing %s?", s);
	itemnumber = messagebox (msg, "replace?", MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL);
	
	switch (itemnumber) {
		
		case IDYES: 
			return (1);
		
		case IDNO: 
			return (2);
		
		case IDCANCEL: 
			return (3);
		} /*switch*/
	
	return (1);
	} /*replacevariabledialog*/


boolean revertdialog (bigstring bs) {

	bigstring bsprompt;
	short itemnumber;
	
	shellgetstring (revertitemstring, bsprompt);
	
	parsedialogstring (bsprompt, bs, nil, nil, nil, bsprompt);
	 
	convertpstring (bsprompt);

	itemnumber = messagebox (bsprompt, "revert?", MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL);
	
	return (itemnumber == IDYES);
	} /*revertdialog*/


// Ask Dialog Code

typedef struct tyAskDialogInfo {
	char * caption;
	char * textBuffer;
	char * retBuffer;
	long retBufferLen;
	} tyAskDialogInfo;

LRESULT CALLBACK AskDialogCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	
	tyAskDialogInfo * p;

	switch (message) {
		
		case WM_INITDIALOG:
			centerdialog (hwnd);

			p = (tyAskDialogInfo *) lParam;

			SetWindowText (hwnd, frontierstring); //p->caption

			SetDlgItemText (hwnd, ID_ASKTEXTDIALOGSTATICFIELD, p->textBuffer);
			SetDlgItemText (hwnd, ID_ASKTEXTDIALOGEDITFIELD, stringbaseaddress(p->retBuffer));
			SendDlgItemMessage (hwnd, ID_ASKTEXTDIALOGEDITFIELD, EM_SETSEL, 0, -1);
			
			SetWindowLong (hwnd, DWL_USER, lParam); 

			SetFocus (GetDlgItem (hwnd, ID_ASKTEXTDIALOGEDITFIELD));
			return (false);

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK:
					p = (tyAskDialogInfo *) GetWindowLong (hwnd, DWL_USER);

					GetDlgItemText (hwnd, ID_ASKTEXTDIALOGEDITFIELD, stringbaseaddress(p->retBuffer), p->retBufferLen-1);
					setstringlength (p->retBuffer, strlen(stringbaseaddress(p->retBuffer)));

				    EndDialog (hwnd, TRUE);
					return TRUE;

				case IDCANCEL:
				    EndDialog (hwnd, FALSE);
				    return TRUE;

				default:
					break;
				}

		default:
			break;
		}

	return FALSE;
	} /*AskDialogCallback*/

static boolean winAskDialog (HWND window, bigstring caption, bigstring message, bigstring retString, long retStringSize, boolean flNumeric, boolean flPassword) {
	
	tyAskDialogInfo p;
	char cmsg[400];
	char capmsg[400];
	int res;
	
	memmove (capmsg, stringbaseaddress (caption), stringlength (caption));
	capmsg [stringlength(caption)] = 0;
	
	memmove (cmsg, stringbaseaddress (message), stringlength (message));
	cmsg[stringlength (message)] = 0;
	
	p.retBufferLen = retStringSize;
	retString[stringlength(retString)+1] = 0;
	p.retBuffer = retString;
	p.textBuffer = cmsg;
	p.caption = capmsg;
	
	releasethreadglobals ();

	if (flNumeric)
		res = DialogBoxParam (hInst, MAKEINTRESOURCE (IDD_ASKNUMBER), window, (DLGPROC)AskDialogCallback, (LPARAM) &p);
	else if (flPassword)
		res = DialogBoxParam (hInst, MAKEINTRESOURCE (IDD_ASKPASSWORD), window, (DLGPROC)AskDialogCallback, (LPARAM) &p);
	else
		res = DialogBoxParam (hInst, MAKEINTRESOURCE (IDD_ASKTEXT), window, (DLGPROC)AskDialogCallback, (LPARAM) &p);
	
	grabthreadglobals ();

	if (res > 0)
		return (true);
	
	return (false);
	} /*winAskDialog*/


boolean askdialog (bigstring bsprompt, bigstring bsresult) {
	
	return (winAskDialog (hwndMDIClient, "", bsprompt, bsresult, lenbigstring, false, false));
	} /*askdialog*/

boolean askpassword (bigstring bsprompt, bigstring bsresult) {
	
	return (winAskDialog (hwndMDIClient, "", bsprompt, bsresult, lenbigstring, false, true));
	} /*askpassword*/

boolean intdialog (bigstring bsprompt, short *n) {
	
	bigstring bsresult;

	numbertostring (*n, bsresult);

	if (!winAskDialog (hwndMDIClient, "", bsprompt, bsresult, lenbigstring, true, false))
		return (false);
	
	stringtoshort (bsresult, n);

	return (true);
	} /*intdialog*/

// Char Dialog Code

typedef struct tyCharDialogInfo {
	char * prompt;
	char * retBuffer;
	long retBufferLen;
	} tyCharDialogInfo;

LRESULT CALLBACK CharDialogCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	
	tyCharDialogInfo * p;

	switch (message) {
		
		case WM_INITDIALOG:
			centerdialog (hwnd);

			p = (tyCharDialogInfo *) lParam;

			SetDlgItemText (hwnd, IDC_CHARPROMPT, p->prompt);

			SetWindowLong (hwnd, DWL_USER, lParam); 

			SendDlgItemMessage (hwnd, IDC_EDIT1, EM_SETLIMITTEXT, (WPARAM) 1, 0);

			SetFocus (GetDlgItem (hwnd, IDC_EDIT1));
			return (false);

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK:
					p = (tyCharDialogInfo *) GetWindowLong (hwnd, DWL_USER);

					GetDlgItemText (hwnd, IDC_EDIT1, stringbaseaddress(p->retBuffer), p->retBufferLen-1);
					setstringlength (p->retBuffer, strlen(stringbaseaddress(p->retBuffer)));

				    EndDialog (hwnd, TRUE);
					return TRUE;

				case IDCANCEL:
				    EndDialog (hwnd, FALSE);
				    return TRUE;

				case IDC_EDIT1:
					//Since we limit to a single char, always highlight "all" text after a change
					if (HIWORD(wParam) == EN_CHANGE)
						SendDlgItemMessage (hwnd, IDC_EDIT1, EM_SETSEL, (WPARAM) (INT) 0, (LPARAM) (INT) -1);
					break;
					
				default:
					break;
				}

		default:
			break;
		}

	return FALSE;
	} /*CharDialogCallback*/

boolean chardialog (bigstring prompt, short *n) {
	
	tyCharDialogInfo p;
	char cmsg[400];
	bigstring retString;
	int res;
	
	copyptocstring (prompt, cmsg);
	
	p.retBufferLen = sizeof(bigstring);
	setstringlength (retString, 0);
	p.retBuffer = retString;
	p.prompt = cmsg;
	
	releasethreadglobals ();

	res = DialogBoxParam (hInst, MAKEINTRESOURCE (IDD_DIALOGCOMMANDKEY), hwndMDIClient, (DLGPROC)CharDialogCallback, (LPARAM) &p);
	
	grabthreadglobals ();

	*n = getstringcharacter(retString, 0);

	if (res > 0)
		return (true);
	
	return (false);
	} /*chardialog*/
// Char Dialog Code

typedef struct tyUserInfoDialogInfo {
	unsigned char * username;
	unsigned char * userinitials;
	unsigned char * userorg;
	unsigned char * useremail;
	} tyUserInfoDialogInfo;

LRESULT CALLBACK UserInfoDialogCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	
	tyUserInfoDialogInfo * p;

	switch (message) {
		
		case WM_INITDIALOG:
			centerdialog (hwnd);

			p = (tyUserInfoDialogInfo *) lParam;

			SetWindowLong (hwnd, DWL_USER, lParam); 

			SendDlgItemMessage (hwnd, IDC_EDIT1, EM_SETLIMITTEXT, (WPARAM) sizeof(bigstring) - 2, 0);
			SendDlgItemMessage (hwnd, IDC_EDIT2, EM_SETLIMITTEXT, (WPARAM) 4, 0);
			SendDlgItemMessage (hwnd, IDC_EDIT3, EM_SETLIMITTEXT, (WPARAM) sizeof(bigstring) - 2, 0);
			SendDlgItemMessage (hwnd, IDC_EDIT4, EM_SETLIMITTEXT, (WPARAM) sizeof(bigstring) - 2, 0);

			SetDlgItemText (hwnd, IDC_EDIT1, stringbaseaddress(p->username));
			SetDlgItemText (hwnd, IDC_EDIT2, stringbaseaddress(p->userinitials));
			SetDlgItemText (hwnd, IDC_EDIT3, stringbaseaddress(p->userorg));
			SetDlgItemText (hwnd, IDC_EDIT4, stringbaseaddress(p->useremail));

			SendDlgItemMessage (hwnd, IDC_EDIT1, EM_SETSEL, (WPARAM) (INT) 0, (LPARAM) (INT) -1);
			SetFocus (GetDlgItem (hwnd, IDC_EDIT1));
			return (false);

		case WM_COMMAND:
			switch (LOWORD(wParam))
				{
				case IDOK:
					p = (tyUserInfoDialogInfo *) GetWindowLong (hwnd, DWL_USER);

					GetDlgItemText (hwnd, IDC_EDIT1, stringbaseaddress(p->username), sizeof(bigstring) - 2);
					setstringlength (p->username, strlen(stringbaseaddress(p->username)));

					GetDlgItemText (hwnd, IDC_EDIT2, stringbaseaddress(p->userinitials), sizeof(bigstring) - 2);
					setstringlength (p->userinitials, strlen(stringbaseaddress(p->userinitials)));

					GetDlgItemText (hwnd, IDC_EDIT3, stringbaseaddress(p->userorg), sizeof(bigstring) - 2);
					setstringlength (p->userorg, strlen(stringbaseaddress(p->userorg)));

					GetDlgItemText (hwnd, IDC_EDIT4, stringbaseaddress(p->useremail), sizeof(bigstring) - 2);
					setstringlength (p->useremail, strlen(stringbaseaddress(p->useremail)));

				    EndDialog (hwnd, TRUE);
					return TRUE;

				case IDCANCEL:
				    EndDialog (hwnd, FALSE);
				    return TRUE;

				default:
					break;
				}

		default:
			break;
		}

	return FALSE;
	} /*UserInfoDialogCallback*/

boolean userinfodialog (bigstring username, bigstring userinitials, bigstring userorg, bigstring useremail) {
	OSVERSIONINFO vi;
	tyUserInfoDialogInfo p;
	int res;
	bigstring path;
	
	p.username = username;
	p.userinitials = userinitials;
	p.userorg = userorg;
	p.useremail = useremail;

	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx (&vi);

	if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
		copyctopstring ("software\\Microsoft\\Windows NT\\CurrentVersion", path);
	else
		copyctopstring ("software\\Microsoft\\Windows\\CurrentVersion", path);

	if (stringlength (username) == 0) {
		getRegKeyString ((Handle) HKEY_LOCAL_MACHINE, path, "\x0f" "RegisteredOwner", username);

		if (stringlength (username) > 0) {
			short pos, i;

			pos = 0;

			setstringlength (userinitials, 0);

			setstringcharacter (userinitials, pos, getstringcharacter(username, 0));

			++pos;

			for (i = 1; i < (stringlength(username) - 1); i++) {
				if (getstringcharacter (username, i) == ' ') {
					setstringcharacter (userinitials, pos, getstringcharacter(username, i+1));
					++pos;
					}
				}

			setstringlength(userinitials, pos); /*6.1fc2 AR: was pos+1*/
			}
		}

	if (stringlength (userorg) == 0) {
		getRegKeyString ((Handle) HKEY_LOCAL_MACHINE, path, "\x16" "RegisteredOrganization", userorg);
		}


	nullterminate (username);
	nullterminate (userinitials);
	nullterminate (userorg);
	nullterminate (useremail);

	releasethreadglobals ();

	res = DialogBoxParam (hInst, MAKEINTRESOURCE (IDD_DIALOGUSERINFO), hwndMDIClient, (DLGPROC)UserInfoDialogCallback, (LPARAM) &p);
	
	grabthreadglobals ();

	if (res > 0)
		return (true);
	
	return (false);
	} /*userinfodialog*/

boolean initdialogs (void) {return true;}


#endif

// land.c

boolean landeventfilter (EventRecord *ev) {return false;}



/*	Build file name takes the path and file components and concatenats them.
	The sources may be a standard string, a pascal string or a c string.
	the type field specifies the string type:
	0 - standard string
	1 - C string
	2 - Pascal String
*/

boolean buildfilename (unsigned char * pathsource, short pathtype, unsigned char * filesource, short filetype, char * dest)
	{
	short pathlen;
	short filelen;
	char * pathaddr;
	char * fileaddr;

	setstringlength (dest, 0);		/* incase we return false - set this string to null*/

	switch (pathtype)
		{
		case 0:			/*current string type*/
			pathlen = stringlength (pathsource);
			pathaddr = stringbaseaddress (pathsource);
			break;

		case 1:
			pathlen = strlen (pathsource);
			pathaddr = pathsource;
			break;

		case 2:
			pathlen = pathsource[0];
			pathaddr = &(pathsource[1]);
			break;

		default:
			return (false);
		}

	switch (filetype)
		{
		case 0:			/*current string type*/
			filelen = stringlength (filesource);
			fileaddr = stringbaseaddress (filesource);
			break;

		case 1:
			filelen = strlen (filesource);
			fileaddr = filesource;
			break;

		case 2:
			filelen = filesource[0];
			fileaddr = &(filesource[1]);
			break;

		default:
			return (false);
		}

	if ((pathlen + filelen) > 256)
		return (false);

	memmove (stringbaseaddress(dest), pathaddr, pathlen);
	memmove (stringbaseaddress(dest)+pathlen, fileaddr, filelen);
	setstringlength (dest, pathlen + filelen);
#ifdef PASCALSTRINGVERSION
		dest [stringlength(dest) + 1] = 0;
#endif
	return (true);
	}

boolean appendcstring (bigstring dest, char * cstringsource) {
	short lendest, lensource;

	lendest = stringlength (dest);
	lensource = strlen (cstringsource);

	if (lensource+lendest > 255)
		return (false);

	memmove (stringbaseaddress (dest)+lendest, cstringsource, lensource);
	setstringlength (dest, lendest+lensource);
	return (true);
	}


// langipcmenus.c

boolean langipcmenustartup (void) {return true;}

boolean langipcmenushutdown (void) {return true;}

boolean langipcsymbolchanged (hdlhashtable ht, const bigstring bs, boolean fl) {return true;}

boolean langipcsymbolinserted (hdlhashtable ht, const bigstring bs) {return true;}

boolean langipcsymboldeleted (hdlhashtable ht, const bigstring bs) {return true;}


// langdialog.h

boolean langdialogstart (void) {return (true);}


