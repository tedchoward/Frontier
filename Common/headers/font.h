
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

#define fontinclude /*so other includes can tell if we've been loaded*/


#ifndef shelltypesinclude
	
	#include "shelltypes.h"

#endif

#ifdef MACVERSION
	#define geneva kFontIDGeneva
	#define helv kFontIDHelvetica
#endif
#ifdef WIN95VERSION
	extern short geneva;
#endif

extern FontInfo globalfontinfo;


/*prototypes*/

extern void fontgetnumber (bigstring, short *);

extern void fontgetname (short, bigstring);

extern boolean realfont (short, short);

extern void setfontsizestyle (short, short, short);

extern void setglobalfontsizestyle (short, short, short);

extern short setnamedfont (bigstring, short, short, short);

extern void getfontsizestyle (short *, short *, short *);

extern void fontstring (short, short, boolean, boolean, bigstring);

extern void getstyle (short, boolean *, boolean *, boolean *, boolean *, boolean *, boolean *);

extern void diskgetfontname (short, diskfontstring);

extern void diskgetfontnum (diskfontstring, short *);

extern boolean initfonts ();

#ifdef WIN95VERSION

	extern void getWindowsLogFont (LOGFONT *);

	extern void setWindowsFont ();

	extern void clearWindowsFont ();

#endif


/* file.h*/

	
