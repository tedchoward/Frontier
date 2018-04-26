
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

#ifndef shelltypesinclude
#define shelltypesinclude

#include "standard.h"

#ifdef HEADERTRACE
#pragma message( "**Compiling " __FILE__ )
#endif

#define diskfontnamelength 32 /*number of bytes for a font name stored on disk*/

typedef char diskfontstring [diskfontnamelength + 1];

typedef struct diskrect {
	
	short top;
	
	short left;
	
	short bottom;
	
	short right;
	} diskrect;

typedef struct diskrgb {
	
	short red;
	
	short green;
	
	short blue;
	} diskrgb;


typedef ControlHandle hdlscrollbar;


	typedef MenuHandle hdlmenu;


#pragma pack(2)
typedef struct tytextdisplayinfo { 

	short h; /*the horizontal pen position for all lines*/
	
	short v; /*the vertical pen position for the first line*/
	
	short lh; /*the uniform lineheight of all lines*/
	
	short screenlines; /*the number of lines that fit within the current window's rectangle*/
	
	Rect r; /*the rectangle within which everything is displayed*/
	
	short horizscrollpixels; /*number of pixels to scroll by for each horiz scroll*/
	} tytextdisplayinfo;
#pragma options align=reset


typedef short **hdlintarray;

#pragma pack(2)
typedef struct typopuprecord { /*this record isn't currently used*/
	
	Rect popuprect; /*where the whole popup structure is displayed*/
	} typopuprecord, *ptrpopuprecord, **hdlpopuprecord;
#pragma options align=reset


typedef OSType tyscraptype;


typedef ProcessSerialNumber typrocessid;


typedef enum clickflags { /*these match #defines in Paige.h*/

	clicknormal = 0,
	
	clickextend = 0x0001, /* extend the selection */
	
	clickwords = 0x0002, /* select whole words only */

	clickparas = 0x0004, /* select whole paragraphs only */

	clicklines = 0x0008, /* highlight whole lines */

	clickvertical = 0x0010, /* allow vertical selection */

	clickdiscontiguous = 0x0020, /* enable discontiguous selection */

	clickstyle = 0x0040, /* select whole style range */

	clickcontrol = 0x0200, /* word advance for arrows, Home-End to doc top and bottom */

	clickoption = 0x0400, /* option key held down */

	clickcommand = 	0x0800	/* alt key (Windows) or command key (Mac) */
	
	} tyclickflags;


typedef enum tykeyflags { /*these match tyclickflags, but apply to keystrokes*/

	keynormal = 0,
	
	keyshift = 0x0001,

	keycontrol = 0x0200,

	keyoption = 0x0400,

	keycommand = 0x0800
	
	} tykeyflags;


#pragma pack(2)
typedef struct tybuttonstatus {
	
	boolean fldisplay; /*should button be displayed at all?*/
	
	boolean flenabled; /*if displayed, is it active (or dimmed)?*/
	
	boolean flbold; /*if displayed, should text style be bold?*/
	} tybuttonstatus;
#pragma options align=reset



	/*
	2009-09-03 aradke: FSSpec was deprecated on Mac OS X and FSRef is its replacement.
		FSSpec could represent a non-existant file as long as its parent directory existed, but FSRef can not.
		Therefore, we have to extend FSRef with a string to hold the name of the non-existant file.
		The FSRef field usually points to the parent folder instead of to the file itself (even if the file exists).
		However, volumes have no parent folder so the FSRef field points to the volume itself in that case.
		
		We chose a fixed-size Unicode string for the name field to simplify memory management.
		If we used a CFStringRef we would have to keep track of copying and releasing in lots of places.

		Reference: Apple Technical Note TN2078 -- Migrating to FSRefs & long Unicode names from FSSpecs
		http://developer.apple.com/legacy/mac/library/technotes/tn2002/tn2078.html
	*/

#pragma pack(2)
	typedef struct tyfilespecflags {
	
		//boolean	flvalid;	// true: ref and name have been set -- false: empty filespec
		boolean flvolume;	// true: ref points to volume -- false: ref points to parent of file/folder
		
		} tyfilespecflags;
	
	typedef struct HFSUniStr255 tyfsname, *tyfsnameptr;
	
	typedef struct tyfilespec {
	
		tyfilespecflags	flags;
		FSRef ref;
		tyfsname name;
		
		} tyfilespec;
		
	typedef tyfilespec *ptrfilespec, **hdlfilespec;
#pragma options align=reset
		



#endif

