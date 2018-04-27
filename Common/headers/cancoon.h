
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

#define cancooninclude /*so other guys can tell if we've been included*/


#ifndef dbinclude

#include "db.h"

#endif


#ifndef langinclude

#include "lang.h"

#endif


#ifndef processinclude

#include "process.h"

#endif


#ifndef menueditorinclude

#include "menueditor.h"

#endif	



#define ctwindowinfo 6 /*number of windowinfo records saved in each cancoon record*/

#define ixcancooninfo 0
#define ixmsginfo 1
#define ixlangerrorinfo 2
#define ixcommandinfo 3
#define ixaboutinfo 4 
#define ixpaletteinfo 5 

#pragma pack(2)
typedef struct tycancoonwindowinfo { /*lives both in memory and on disk*/	
	
	diskrect windowrect;
	
	diskfontstring fontname; /*only maintained on disk*/
	
	short fontnum; /*only valid when it's in memory*/
	
	short fontsize, fontstyle;
	
	WindowPtr w; /*only valid when it's in memory*/
	
	boolean flhidden; /*6.23.97 dmb*/

	boolean flunused;

	char waste [8];
	} tycancoonwindowinfo;


typedef struct tycancoonrecord { /*one of these for every cancoon file that's open*/

	hdldatabaserecord hdatabase; /*db.c's record*/
	
	hdlhashtable hroottable; /*the root symbol table for this file*/
	
	Handle hrootvariable; /*the variable record for the root symbol table*/
	
	
	hdlmenubarlist hmenubarlist; /*the menubar values that are active in this cancoon file*/
	
	hdlprocesslist hprocesslist; /*background processes attached to us*/
	
	Handle hscriptstring; /*appears in the quickscript dialog, if nil use empty string*/
	
	tycancoonwindowinfo windowinfo [ctwindowinfo];
	
	hdlstring hprimarymsg; /*the message being displayed in home window*/
	
	hdlstring hsecondarymsg; /*the message waiting to be displayed, nil if none*/
	
	hdltreenode hprimaryagent; /*code of agent selected with popup*/
	
	short oldwidth; /*used in the flagflip routine*/
	
		
		Rect messagearea;
		
		Rect aboutarea;
		
		boolean flguestroot; // are we mounted into a host root?

	
	boolean fldirty; /*anyone who changes the record should set this true*/
	
	boolean flbigwindow; /*true if flag icon is down*/
	
	boolean flbackgroundmsgblocked; /*true until user clicks in window*/
	
	boolean flflagdisabled; /*hide the flag?*/
	
	boolean flpopupdisabled; /*hide the agents popup menu?*/
	
	} tycancoonrecord, *ptrcancoonrecord, **hdlcancoonrecord;
#pragma options align=reset

/*the one "superglobal" variable*/

extern hdlcancoonrecord cancoonglobals;


/*prototypes*/

extern boolean ccnewfilewindow (tyexternalid, WindowPtr *, boolean);

extern boolean ccdisposefilerecord (void);

extern boolean ccsavespecialfile (ptrfilespec, hdlfilenum, short, boolean, boolean);

extern boolean ccgetwindowrect (short, Rect *);

extern boolean ccnewsubwindow (hdlwindowinfo, short);

extern boolean ccsubwindowclose (hdlwindowinfo, short);

extern boolean cccopywindowinfo (hdlwindowinfo, short);

extern void setcancoonglobals (hdlcancoonrecord);

extern boolean ccinexpertmode (void);

extern boolean ccfindrootwindow (hdlwindowinfo *);

extern hdldatabaserecord ccwindowgetdatabase (WindowPtr);

extern boolean ccstart (void);



