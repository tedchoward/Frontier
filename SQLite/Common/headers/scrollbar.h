
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

#define scrollbarinclude /*so other includes can tell if we've been loaded*/

#ifndef shelltypesinclude
	
	#include "shelltypes.h"

#endif


typedef struct tyscrollinfo {
	
	long min, max, cur, pag;
	} tyscrollinfo, *ptrscrollinfo;


/*prototypes*/

extern void validscrollbar (hdlscrollbar);

extern boolean pointinscrollbar (Point, hdlscrollbar);

extern void getscrollbarinfo (hdlscrollbar, tyscrollinfo *);

extern void setscrollbarinfo (hdlscrollbar, const tyscrollinfo *);

extern void enablescrollbar (hdlscrollbar);

extern void disablescrollbar (hdlscrollbar);

extern long getscrollbarcurrent (hdlscrollbar);

extern void showscrollbar (hdlscrollbar);

extern void hidescrollbar (hdlscrollbar);

extern void drawscrollbar (hdlscrollbar);

extern void displayscrollbar (hdlscrollbar);

extern void setscrollbarcurrent (hdlscrollbar, long);

extern short getscrollbarwidth (void);

extern boolean newscrollbar (WindowPtr, boolean, hdlscrollbar *);

extern void disposescrollbar (hdlscrollbar);

extern void getscrollbarrect (hdlscrollbar, Rect *);

extern void setscrollbarrect (hdlscrollbar, Rect);

extern void scrollbarflushright (Rect, hdlscrollbar);

extern void scrollbarflushbottom (Rect, hdlscrollbar);

extern boolean findscrollbar (Point, WindowPtr, hdlscrollbar *, short *);

extern boolean scrollbarhit (hdlscrollbar, short, boolean *, boolean *);

extern boolean initscrollbars (void); /*7.0b18 PBS*/
