
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

#define opdraggingmoveinclude


#ifndef opinternalinclude

	#include "opinternal.h"

#endif

#define draggingscrollrate 2 /*max scroll rate is 30 lines per second*/

typedef struct tyhotspot {
	
	Point pt; /*the mouse point that determined the rect*/
	
	short lnum; /*the line number that htarget is displayed on*/
	
	hdlheadrecord htarget; /*where the deposit takes place*/
	
	hdlheadrecord hsource; /*the node that's getting moved*/
	
	tydirection dir; /*deposit to the right or down*/
	
	boolean fldisplayed: 1; /*is the hot spot showing?*/
	
	Handle sourcewindowhandle, destwindowhandle;
	} tyhotspot;


extern void operasehotspot (tyhotspot *hotspot);

extern void opscrollfordrag (tyhotspot *hotspot, tydirection scrolldir);

extern boolean opisdraggingmove (Point, unsigned long);

extern void opdraggingmove (Point, hdlheadrecord);

extern boolean opmovetohotspot (tyhotspot *hotspot);

extern void opgetwindowhandle (Point pt, Handle *windowhandle);

extern void opsetwindowhandlecontext (Handle windowhandle);

