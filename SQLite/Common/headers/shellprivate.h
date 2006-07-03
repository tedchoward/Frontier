
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

#define shellprivateinclude /*so other includes can tell if we've been loaded*/


/*
names that communicate between the various files that make up the shell level.

handlers are not supposed to include this file.
*/

#define jugglerEvt 15

#define tickstoupdatemenus 20 /*update menus one-third second after last event*/

#define tickstoidle 6 /*only call idle callback routine every tenth second*/


#define ctglobals 32 /*we can remember globals up to ctglobals levels deep*/


typedef struct tyglobalsstack {
	
	short top;
	
	WindowPtr stack [ctglobals];
	} tyglobalsstack;


typedef enum tymenustate {
	dirtymenus,
	
	normalmenus,
	
	optionmenus,
	
	modaldialogmenus
	} tymenustate;


typedef struct tyjugglermessage {
	
	long eventtype: 8; /*bits 24 -- 31*/
	
	long reservedbits: 22; /*bits 2 -- 23*/
	
	long flconvertclipboard: 1; /*bit 1*/
	
	long flresume: 1; /*bit 0*/
	} tyjugglermessage;

#ifdef version42orgreater
	#define cteditors 14
#else
	#define cteditors 12
#endif


/*globals*/

extern tyshellglobals globalsarray [cteditors];

extern short topglobalsarray; /*initially empty*/

extern tyglobalsstack globalsstack; /*for push/pop globals*/

extern boolean flexitmainloop;

// 2006-03-31 kw & aradke: menustate is declared static in shellmenu.c
//extern tymenustate menustate;

extern unsigned long timelastkeystroke;

// 2006-03-31 kw & aradke: flshellimmediatebackground is declared static in shell.c
//extern boolean flshellimmediatebackground; /*service the background queue immediately*/


/*prototypes*/

extern boolean shellfindcallbacks (short, short *); /*shellcallbacks.c*/

extern void shellpatchnilroutines (void);

extern void shellinithandlers (void);

extern void shellloadbuttonlists (void);

extern void shellclearwindowdata (void);


extern boolean shellautoopen (bigstring, short); /*shellfile.c*/

extern void killownedundo (WindowPtr);


extern void shellhandlejugglerevent (void); /*shelljuggler.c*/


extern void shelladjustmenus (void); /*shellmenu.c*/


/* 5.0a5 dmb - no longer needed: extern void shellsetscrollbarinfo (void); /%shellops.c*/


extern boolean shellgetgrowiconrect (hdlwindowinfo, Rect *); /*shellwindow.c*/

extern void shelldrawgrowicon (hdlwindowinfo);

extern void shellerasegrowicon (hdlwindowinfo);

extern boolean isshellwindow (WindowPtr);

extern boolean frontshellwindow (WindowPtr *);

extern boolean shellsavewindowresource (WindowPtr, ptrfilespec, short);

extern boolean shellsavewindowposition (WindowPtr);

extern boolean loadwindowposition (ptrfilespec, short, tywindowposition *);

extern boolean shellsavefontresource (WindowPtr, ptrfilespec, short);

extern boolean shellsavedefaultfont (WindowPtr);

extern boolean loaddefaultfont (WindowPtr);

extern void getdefaultwindowrect (Rect *);

extern void shellresetwindowrects (hdlwindowinfo);

extern void windowresetrects (hdlwindowinfo);

extern boolean emptywindowlist (void);

extern short countwindowlist (void);

extern boolean indexwindowlist (short, hdlwindowinfo *);

extern short counttypedwindows (short);

extern boolean shellfirstchildwindow (hdlwindowinfo, hdlwindowinfo *);

extern void grayownedwindows (WindowPtr);

extern boolean defaultselectioninfo (hdlwindowinfo);

extern boolean newshellwindowinfo (WindowPtr w, hdlwindowinfo *hinfo);

extern void disposeshellwindowinfo (hdlwindowinfo hinfo);

extern boolean newshellwindow (WindowPtr *, hdlwindowinfo *, tywindowposition *);

extern boolean windowinit (WindowPtr);

extern boolean zoomfilewindow (WindowPtr);

extern boolean getwindowmessage (WindowPtr, bigstring);

extern boolean drawwindowmessage (WindowPtr);

extern boolean setwindowmessage (WindowPtr, bigstring);

extern boolean lockwindowmessage (WindowPtr, boolean);

extern void shellerasemessagearea (hdlwindowinfo);

extern boolean windowmadechanges (WindowPtr);

extern boolean windowgetcontentrect (WindowPtr, Rect *);

extern void disposeshellwindow (WindowPtr);

extern boolean shellhidewindow (hdlwindowinfo);

extern boolean shellunhidewindow (hdlwindowinfo);

extern void closewindowfile (WindowPtr);

extern boolean shellfrontrootwindowmessage (bigstring);

extern boolean shellrunwindowconfirmationscript (WindowPtr, short);


extern boolean shellpushtargetglobals (void); /*shellverbs.c*/

extern boolean shellinitverbs (void);

#ifdef WIN95VERSION
	extern boolean shellprocessfindmessage (FINDREPLACE *);
#endif



