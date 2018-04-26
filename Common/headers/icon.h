
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

#define iconinclude




#define maxcustomicontypes 20

#define idfirstcustomicon 1000

struct tycustomicontypeinfo {
	
	bigstring bstype; /*headline type*/
	
	short rnum; /*reference to file's resource fork*/
	}; 



/*function prototypes*/
extern boolean ploticonfromodb (const Rect *r, short align, short transform, bigstring bsadricon);

extern void drawlabeledicon (const Rect *, short, bigstring, boolean);

extern void drawiconsequence (Rect, short, short, bigstring);

extern void drawlabeledwindoidicon (Rect, bigstring, boolean, boolean);

extern boolean trackicon (Rect, void (*) (boolean));

extern boolean ploticon (const Rect *, short id);

extern boolean ploticonresource (const Rect *r, short align, short transform, short resid);

extern boolean ploticoncustom (const Rect *r, short align, short transform, bigstring bsiconname); /*7.0b9 PBS*/

boolean customicongetrnum (bigstring bstype, short *rnum);