
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
	
	boolean nohorizscroll, horizscroll; /*window has horiz scrollbar?*/
	
	boolean novertscroll, vertscroll;
	
	boolean dontfloat, windowfloats; /*is it a floating palette window?*/
	
	boolean nomessagearea, messagearea; /*allocate space for a message area?*/
	
	boolean dontinsetcontentrect, insetcontentrect; /*if true we inset by 3 pixels*/
	
	boolean nonewonlaunch, newonlaunch;
	
	boolean dontopenresfile, openresfile;
	
	boolean normalwindow, dialogwindow; /*do a GetNewDialog on creating one of these windows?*/
	
	boolean notgrowable, isgrowable; /*provide a grow box for window*/
	
	boolean dontcreateonnew, createonnew;
	
	boolean nowindoidscrollbars, windoidscrollbars;
	
	boolean notstoredindatabase, storedindatabase;
	
	boolean handlesownsave, parentwindowhandlessave;
	
	boolean donteraseonresize, eraseonresize;
	
	boolean consumefrontclicks, dontconsumefrontclicks;
	
	boolean monochromewindow, colorwindow;
	
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
