
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

#define shellbuttonsinclude /*so other modules can tell we've been included*/

/*constants*/

#define maxbuttons 16 // to handle more, we'd have to change flags from short to longs

#define boldbuttonstyle bold

#define minbuttonwidth 45

#define pixelsbetweenbuttons 8

	#define buttontextinset 9

		#define buttonsrectheight 29
	#define buttonbackground qd.gray
	#define whitebackground qd.white


#define buttonfont geneva
#define buttonsize 9

#define linepattern black

#define buttontopinset 5

#define buttonbottominset 6



/*function prototypes*/

extern void shellgetbuttonrect (short, Rect *);

extern void shelldrawbutton (short, boolean);

extern void shelldrawbuttons (void);

extern void shellbuttonhit (Point);

extern void shellgetbuttonsrect (hdlwindowinfo, Rect *);

extern void shellinvalbuttons (void);

extern boolean shellgetbuttonstring (short, bigstring);

extern void shellbuttongetoptimalwidth (short *);

extern void shellbuttonadjustcursor (Point);
