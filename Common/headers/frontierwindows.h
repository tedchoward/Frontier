
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

#define windowsinclude


/*prototypes*/

extern WindowPtr getnewwindow (short, boolean, Rect *);

extern void disposewindow (WindowPtr);
	
extern void windowsettitle (WindowPtr, bigstring);

extern void windowgettitle (WindowPtr, bigstring);

extern void windowinval (WindowPtr);

extern boolean graywindow (WindowPtr);

extern boolean windowbringtofront (WindowPtr);

extern boolean windowsendtoback (WindowPtr);

extern boolean windowsendbehind (WindowPtr, WindowPtr);

extern boolean getlocalwindowrect (WindowPtr, Rect *);

extern boolean getglobalwindowrect (WindowPtr, Rect *);

extern void movewindow (WindowPtr, short, short);

extern void movewindowhidden (WindowPtr, short, short);

extern void sizewindow (WindowPtr, short, short);

extern void sizewindowhidden (WindowPtr, short, short);

extern void moveandsizewindow (WindowPtr, Rect);

extern WindowPtr getnextwindow (WindowPtr w);

extern WindowPtr getfrontwindow (void);

extern boolean findmousewindow (Point, WindowPtr *, short *);

extern boolean windowsetcolor (WindowPtr, long, boolean);

extern boolean isdeskaccessorywindow (WindowPtr);

extern void showwindow (WindowPtr);

extern void hidewindow (WindowPtr);

extern boolean windowvisible (WindowPtr);

extern hdlregion getupdateregion (WindowPtr);

extern hdlregion getvisregion (WindowPtr);

extern void setwindowrefcon (WindowPtr, long);

extern long getwindowrefcon (WindowPtr);

