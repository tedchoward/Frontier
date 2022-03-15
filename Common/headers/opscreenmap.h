
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

#define opscreenmapinclude

#pragma pack(2)
typedef struct tymapelement {
	
	hdlheadrecord hnode;
	
	short level;
	
	short vpixels;
	
	boolean flcursorhere;
	
	boolean flmarked; // 5.0a25 dmb
	} tymapelement;
	
	
typedef struct tyscreenmap {
	
	short ctelements;
	
	tymapelement map [];
	} tyscreenmap, *ptrscreenmap, **hdlscreenmap;
#pragma options align=reset

boolean opnewscreenmap (hdlscreenmap *);

void opinvalscreenmap (hdlscreenmap);

void opinvaldirtynodes (void);
