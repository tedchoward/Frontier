
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

#define ioapopupinclude


typedef struct typopupdata {
	
	unsigned short haslabel; /*true if the value is displayed as the label for the popup*/
	
	bigstring bspopup; /*the string that's displayed inside the popup rectangle*/
	
	short checkeditem; /*the item that's checked in the menu*/
	
	Handle hmenuitems; /*semicolon-separated text handle*/
	
	short popupwidth; /*set by cleaning up, the width of the popup menu part of the object*/
	
	MenuHandle hmenu;
	} typopupdata, **hdlpopupdata;


boolean getnthpopupitem (hdlobject, short, bigstring);
