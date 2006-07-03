
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

#define mouseinclude

#define leftmousebuttonaction 0
#define rightmousebuttonaction 1
#define centermousebuttonaction 2
#define wheelmousebuttonaction 3


typedef struct tymouserecord {
	
	boolean fldoubleclick;
	
	Point localpt;
	
	long mouseuptime; 
	
	long mousedowntime; 
	
	Point mouseuppoint;
	
	Point mousedownpoint;
	
	boolean fldoubleclickdisabled;

	short whichbutton;
	} tymouserecord;
	
	
extern tymouserecord mousestatus;


/*prototypes*/

void setmousedoubleclickstatus (boolean fl);

extern boolean mousebuttondown (void);

extern void waitmousebutton (boolean);

extern void waitmouseclick (void);

extern boolean mousestilldown (void);

extern boolean rightmousestilldown (void); /*7.0b26 PBS*/

extern void getmousepoint (Point *);

extern boolean getmousewindowpos (WindowPtr *, Point *);

extern boolean mousetrack (Rect, void (*) (boolean));

extern void mousedoubleclickdisable (void);

extern void mouseup (long eventwhen, long eventposx, long eventposy, long eventwhat);

extern void mousedown (long eventwhen, long eventposx, long eventposy, long eventwhat);

extern boolean mousedoubleclick (void);

extern boolean ismouseleftclick (void);

extern boolean ismouserightclick (void);

extern boolean ismousecenterclick (void);

extern boolean ismousewheelclick (void);

extern void smashmouse (Point);

extern void showmousecursor (void);

extern void hidemousecursor (void);

extern boolean mousecheckautoscroll (Point, Rect, boolean, tydirection *);

extern long getmousedoubleclicktime(void);

