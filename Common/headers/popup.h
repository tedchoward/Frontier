
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

#define popupinclude /*so other modules can tell that we've been loaded*/


#ifndef shelltypesinclude
	
	#include "shelltypes.h"

#endif


#ifdef MACVERSION
	
		#define popupwidth 54
	
	#define popupheight 16
	#define geneva9popupmenuid 257
	#define defaultpopupmenuid 258
#endif

#ifdef WIN95VERSION
	#define popupwidth 55
	#define popupheight 18
	#define geneva9popupmenuid 10000
	#define defaultpopupmenuid 10000
#endif

#define popupfont geneva
#define popupfontsize 9

typedef boolean (*fillpopupcallback) (hdlmenu, short *);

typedef boolean (*popupselectcallback) (hdlmenu, short);


/*prototypes*/

extern boolean pushpopupitem (hdlmenu, bigstring, boolean, short);

extern boolean popupmenuhit (Rect, boolean, fillpopupcallback, popupselectcallback);

extern boolean drawpopup (Rect, bigstring, boolean);

extern boolean adjustpopupcursor (Point, Rect);



