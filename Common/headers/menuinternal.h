
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

#define menuinternalinclude


/*prototypes*/

extern boolean menuverbunload (hdlexternalvariable);

extern void megetoutlinerect (Rect *); /*menueditor.c*/

extern boolean megetcontentsize (long *, long *);

extern boolean mesomethingdirty (hdlmenurecord);

extern boolean mesmashscriptwindow (void);


extern boolean mesearchrefconroutine (hdlheadrecord); /*menufind.c*/

extern boolean mesearchoutline (boolean, boolean, boolean *);


extern boolean megetmenuiteminfo (hdlheadrecord, tymenuiteminfo *);

extern boolean mesetmenuiteminfo (hdlheadrecord, const tymenuiteminfo *);


extern boolean mecopyrefconroutine (hdlheadrecord, hdlheadrecord); /*menupack.c*/

extern boolean metextualizerefconroutine (hdlheadrecord, Handle);

extern boolean mereleaserefconroutine (hdlheadrecord, boolean);

extern boolean mepackmenustructure (tysavedmenuinfo *, Handle *);

extern boolean mesavemenurecord (hdlmenurecord, boolean, boolean, dbaddress *, Handle *);

extern boolean mesetupmenurecord (tysavedmenuinfo *, hdloutlinerecord, hdlmenurecord *);

extern boolean meunpackmenustructure (Handle, hdlmenurecord *);

extern boolean meloadmenurecord (dbaddress, hdlmenurecord *);

extern boolean mesetscraproutine (hdloutlinerecord);

extern boolean megetscraproutine (hdloutlinerecord *, boolean *);

extern boolean mescraphook (Handle);


extern boolean mezoommenubarwindow (hdloutlinerecord, boolean, hdlwindowinfo *); /*meprograms.c*/


extern boolean meresetwindowrects (hdlwindowinfo); /*menuresize.c*/

extern void meresize (void);




