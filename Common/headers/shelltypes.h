
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


#ifdef MACVERSION
typedef ControlHandle hdlscrollbar;
#endif

#ifdef WIN95VERSION
typedef struct tyscrollbarrecord
	{
	HWND hWnd;
	int  item;
	} scrollbarrecord, *ptrscrollbarrecord, **hdlscrollbar;
#endif

#ifdef MACVERSION
	typedef MenuHandle hdlmenu;
#endif

#ifdef WIN95VERSION
	typedef HMENU hdlmenu;
#endif


typedef struct tytextdisplayinfo { 

	short h; /*the horizontal pen position for all lines*/
	
	short v; /*the vertical pen position for the first line*/
	
	short lh; /*the uniform lineheight of all lines*/
	
	short screenlines; /*the number of lines that fit within the current window's rectangle*/
	
	Rect r; /*the rectangle within which everything is displayed*/
	
	short horizscrollpixels; /*number of pixels to scroll by for each horiz scroll*/
	} tytextdisplayinfo;


typedef short **hdlintarray;


typedef struct typopuprecord { /*this record isn't currently used*/
	
	Rect popuprect; /*where the whole popup structure is displayed*/
	} typopuprecord, *ptrpopuprecord, **hdlpopuprecord;


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


typedef struct tybuttonstatus {
	
	boolean fldisplay; /*should button be displayed at all?*/
	
	boolean flenabled; /*if displayed, is it active (or dimmed)?*/
	
	boolean flbold; /*if displayed, should text style be bold?*/
	} tybuttonstatus;


#ifdef MACVERSION

	typedef struct ExtFSRef {

		FSRef fsref;
		CFStringRef path;
		
		} ExtFSRef, *ExtFSRefPtr, **ExtFSRefHandle;
		
	typedef ExtFSRef tyfilespec, *ptrfilespec, **hdlfilespec;
		
	#endif // MACVERSION


#ifdef WIN95VERSION
	
	typedef struct tyfilespec {

			char fullSpecifier [258];
			
			} tyfilespec, *ptrfilespec, **hdlfilespec;

	#define fsname( fs ) ( fs ) -> fullSpecifier

	#endif // WIN95VERSION

#endif

