
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

#define pictstringlist 167
#define picttypestring 1
#define picttextstring 2
#define pictsizestring 3
#define picterrorstring 5


#pragma pack(2)
typedef struct typictrecord {
	
	PicHandle macpicture; /*the structure that's passed off to DrawPicture*/
	
	Rect windowrect; /*the size of the window that last displayed this pict*/
	
	long timecreated, timelastsave; /*maybe we'll use these at some later date?*/
	
	long ctsaves; /*the number of times this structure has been saved*/
	
	short updateticks; /*how many ticks between updates when window is in front*/
	
	long timelastupdate; /*last time this pict was updated*/
	
	long pictrefcon; /*could be anything -- we don't care*/

	boolean fldirty: 1; /*maybe someday we'll have a pict editor*/
	
	boolean fllocked: 1; /*are changes allowed to be made?*/
	
	boolean flwindowopen: 1; /*true if the record is being edited in a window*/
	
	boolean flbitmapupdate: 1; /*if true, use offscreen bitmap when updating*/
	
	boolean flevalexpressions: 1; /*if true, parse all text that begins with an = sign*/
	
	boolean flscaletofitwindow: 1; /*if true, scale window down to fit inside window*/
	
	} typictrecord, *ptrpictrecord, **hdlpictrecord;
#pragma options align=reset

	
extern WindowPtr pictwindow;

extern hdlwindowinfo pictwindowinfo;

extern hdlpictrecord pictdata;


/*prototypes*/


extern void pictdirty (void);

extern boolean pictgetframerect (hdlpictrecord, Rect *);

extern boolean pictpack (hdlpictrecord, Handle *);

extern boolean pictunpack (Handle, long *, hdlpictrecord *);

extern boolean pictnewrecord (void);

extern boolean pictdisposerecord (hdlpictrecord);

extern boolean pictreadfile (bigstring, PicHandle *);

extern void pictresetscrollbars (void);

extern boolean pictscroll (tydirection, boolean, long);

extern void pictupdatepatcher (void);

extern void pictdepatcher (void);

extern void pictupdate (void);

extern void pictidle (void);

extern void pictscheduleupdate (short);

extern void pictsetbitmapupdate (boolean);

extern void pictsetevaluate (boolean);

extern void pictgetnewwindowrect (hdlpictrecord, Rect *);




