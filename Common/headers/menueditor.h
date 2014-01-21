
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

#define menueditorinclude /*so other includes can tell if we've been loaded*/


#ifndef shelltypesinclude
	
	#include "shelltypes.h"

#endif


#ifndef dbinclude

	#include "db.h"

#endif


#ifndef opinclude

	#include "op.h"

#endif


#ifndef menubarinclude

	#include "menubar.h"

#endif


#ifndef langexternalinclude

	#include "langexternal.h"

#endif


#define menuoutlineitem 1 /*the menubar outline*/

#define menucmdkeypopupitem 2 /*popup menu with cmdkeys*/

#define menumessageitem 3 /*where the message goes in this window*/

#define menucmdkeyitem 4 /*displays cmd-key equiv for current menu item*/

#define menuscriptitem 5 /*the script button*/

#ifdef MACVERSION
		#define cmdkeypopupwidth 60
#endif
#ifdef WIN95VERSION
	#define cmdkeypopupwidth 60
#endif

typedef struct tysavedmenuinfo { 
	
	short versionnumber; /*this structure is saved on disk*/
	
	dbaddress adroutline; /*where the menubar outline is stored in the database*/
	
	short vertmin, vertmax, vertcurrent; /*remember the state of the scrollbar*/
	
	diskrect scriptwindowrect; /*the saved position for the script window*/
	
	short flags;
	
	short menuactivelayer;
	
	short lnumcursor;
	
	diskfontstring defaultscriptfontname; /*new scripts start in this font*/
	
	short defaultscriptfontsize; /*and this size*/ 
	
	diskrect menuwindowrect; /*the window size and position last time it was open*/
	
	char waste [42]; /*room to grow*/
	} tysavedmenuinfo;

#ifdef MACVERSION
	#define flautosmash_mask 0x8000
#endif

#ifdef WIN95VERSION
	#define flautosmash_mask 0x0080
#endif

typedef struct tyOLD42savedmenuinfo { 
	
	short versionnumber; /*this structure is saved on disk*/
	
	dbaddress adroutline; /*where the menubar outline is stored in the database*/
	
	short vertmin, vertmax, vertcurrent; /*remember the state of the scrollbar*/
	
	diskrect scriptwindowrect; /*the saved position for the script window*/
	
	boolean flautosmash: 1;
	
	short menuactivelayer;
	
	short lnumcursor;
	
	diskfontstring defaultscriptfontname; /*new scripts start in this font*/
	
	short defaultscriptfontsize; /*and this size*/ 
	
	diskrect menuwindowrect; /*the window size and position last time it was open*/
	
	char waste [42]; /*room to grow*/
	} tyOLD42savedmenuinfo;


#if flruntime

	typedef struct tymenurecord {
		
		hdloutlinerecord menuoutline; /*the display of the menubar structure*/
		
		tysavedmenuinfo menuinfo;
		} tymenurecord, *ptrmenurecord, **hdlmenurecord;

#else

	typedef struct tymenurecord {
		
		hdloutlinerecord menuoutline; /*the display of the menubar structure*/
		
		dbaddress adroutline; /*where the menubar outline is stored in the database*/
		
		hdlmenubarstack hmenustack; /*menubar.c's data structure -- the user's menus*/
		
		Rect menuwindowrect; /*the window size and position last time it was open*/
		
		short menuactiveitem; /*which text item is the active one?*/
		
		short menuactivelayer; /*which layer is active?*/
		
		WindowPtr scriptwindow; /*the window that the script is displayed in, might be nil*/
		
		Rect scriptwindowrect; /*the saved window position for the script window*/
		
		Rect menuoutlinerect; /*the current menubar outline rect*/
		
		Rect cmdkeypopuprect; /*the cmdkey popup rect*/
		
		Rect iconrect; /*the script zoom icon button*/
		
		hdlheadrecord scriptnode; /*the node whose script is being displayed*/
		
		hdloutlinerecord scriptoutline; /*the outline record of the script being edited*/
		
		short defaultscriptfontnum, defaultscriptfontsize; /*font and size for new scripts*/
		
		short movecursorto; /*a signal between meload and meedit*/
		
		boolean fldirty; /*any changes since last save?*/
		
		boolean fllocked; /*are changes allowed to be made?*/
		
		boolean flwindowopen; /*is this menubar being edited now?*/
		
		boolean flactive;
		
		boolean flautosmash;
		
		boolean flinstalled; /*explicitly installed in menubar?*/
		
		boolean flcursormoved; /*do we need to check status of button, popup?*/
		
		/***boolean flpopupactive;*/
		
		/***boolean flzoomscriptwindow; /%zoom out the window in the next idle?*/
		
		long menurefcon; /*for use by application, menueditor doesn't touch it, but menuverbs does*/
		} tymenurecord, *ptrmenurecord, **hdlmenurecord;
	
#endif

typedef struct tylinkeditem {
	
	dbaddress adrlink;
	
	hdloutlinerecord houtline; /*kept in memory until it is saved*/
	} tylinkeditem;


typedef struct tymenuiteminfo { /*linked into the refcon handle in each node*/
	
	byte cmdkey; 
	
	byte cmdmodifiers;
	
	tylinkeditem linkedscript;
	} tymenuiteminfo, *ptrmenuiteminfo, **hdlmenuiteminfo;


/*globals*/
	
extern hdlmenurecord menudata;

extern WindowPtr menuwindow; 

extern hdlwindowinfo menuwindowinfo; 


/*prototypes*/

extern boolean meloadoutline (dbaddress, hdloutlinerecord *); /*menueditor.c*/

extern boolean mepackoutline (hdloutlinerecord, Handle *);

extern boolean mesaveoutline (hdloutlinerecord, dbaddress *);

extern boolean meloadscriptoutline (hdlmenurecord, hdlheadrecord, hdloutlinerecord *, boolean *);

extern boolean mezoomscriptwindow (void);

extern boolean mescriptwindowclosed (void);

extern void mepostcursormove (void);

extern void meexpandto (hdlheadrecord);

extern void mesetcallbacks (hdloutlinerecord);

extern boolean meeditmenurecord (void);

extern boolean menewmenurecord (hdlmenurecord *);

extern void medisposemenurecord (hdlmenurecord, boolean);

extern boolean meinstallmenubar (hdlmenurecord);

extern boolean meremovemenubar (hdlmenurecord);

extern boolean meclearmenubar (void);

#ifdef fldebug

	extern void mecheckglobals (void);

#else

	#define mecheckglobals() ((void) 0)

#endif

extern boolean mesetglobals (void);

extern hdldatabaserecord megetdatabase (hdlmenurecord);

extern boolean mesetscriptoutline (hdlheadrecord, hdloutlinerecord);

extern boolean mesomethingdirty (hdlmenurecord);

extern void mesetcmdkey (byte, tykeyflags);

extern void meupdate (void);

extern void meactivate (boolean);

extern boolean mescroll (tydirection, boolean, long);

extern void megetscrollbarinfo (void);

extern boolean megetundoglobals (long *);

extern boolean mesetundoglobals (long, boolean);

extern boolean memousedown (Point, tyclickflags);

extern boolean mekeystroke (void);

extern boolean mecmdkeyfilter (char);

extern boolean mecut (void);

extern boolean mecopy (void);

extern boolean mepaste (void);

extern boolean meclear (void);

extern boolean meselectall (void);

extern boolean medispose (void);

extern boolean meclose (void);

extern void meidle (void);

extern boolean meadjustcursor (Point);

extern boolean mesetprintinfo (void);

extern boolean meprint (short);

extern void meinit (void);


extern boolean mecontinuesearch (hdlwindowinfo, hdlheadrecord); /*menufind.c*/

extern boolean menuverbsearch (void);




