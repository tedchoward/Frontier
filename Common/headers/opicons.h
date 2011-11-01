
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

#define opiconsinclude


#ifndef opinternalinclude

	#include "opinternal.h"
	
#endif


extern short opgetheadicon (hdlheadrecord);

extern void opdrawheadicon (short, const Rect *, boolean);

extern void opgeticonrect (hdlheadrecord, const Rect *, Rect *);

extern boolean opdefaultgeticonrect (hdlheadrecord, const Rect *, Rect *);

extern boolean opdefaultdrawicon (hdlheadrecord, const Rect *, boolean, boolean);

extern void opdrawarrowicon (hdlheadrecord, long, tydirection);

boolean opdrawheadiconcustom (bigstring bsiconname, const Rect *r, boolean flselected);

boolean opdrawheadiconcustomfromodb (bigstring bsadricon, const Rect *r, boolean flselected);

boolean opgetnodetypetableadr(bigstring bsnodetype, bigstring bsadrnodepath);

boolean resolveHashTable(bigstring bsnodetype, bigstring bsadrnodepath);