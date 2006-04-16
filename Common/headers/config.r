
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

type'cnfg' {
	
	integer nohorizscroll = 0, horizscroll = 1; /*window has horiz scrollbar?*/
	
	integer novertscroll = 0, vertscroll = 1;
	
	integer dontfloat = 0, windowfloats = 1; /*is it a floating palette window?*/
	
	integer nomessagearea = 0, messagearea = 1; /*allocate space for a message area?*/
	
	integer dontinsetcontentrect = 0, insetcontentrect = 1; /*if true we inset by 3 pixels*/
	
	integer nonewonlaunch = 0, newonlaunch = 1;
	
	integer dontopenresfile = 0, openresfile = 1;
	
	integer normalwindow = 0, dialogwindow = 1; /*do a GetNewDialog on creating one of these windows?*/
	
	integer notgrowable = 0, isgrowable = 1; /*provide a grow box for window*/
	
	integer dontcreateonnew = 0, createonnew = 1;
	
	integer nowindoidscrollbars = 0, windoidscrollbars = 1;
	
	integer notstoredindatabase = 0, storedindatabase = 1;
	
	integer handlesownsave = 0, parentwindowhandlessave = 1;
	
	integer donteraseonresize = 0, eraseonresize = 1;
	
	integer consumefrontclicks = 0, dontconsumefrontclicks = 1;
	
	integer monochromewindow = 0, colorwindow = 1;
	
	integer onehalf = 2, onethird = 3, onequarter = 4;
	
	literal longint; /*filecreator*/
	
	literal longint; /*filetype*/
	
	integer; 
	
	Rect; /*for growable windows, the minimum size allowed*/
	
	integer; /*version on disk indexes into a STR# list*/
	
	integer size9 = 9, size12 = 12;
	
	integer plain = 0;
	
	integer; /*if 0, no buttons for this window type*/
	
	Rect; /*new windows come up in this spot*/
	};
