
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

#ifndef tabledisplayinclude
#define tabledisplayinclude

#define maxbrowsercols 5

#define isclaydisplay(hf) (hf && (**(hf)).linelayout.claydisplay)


boolean tableinitdisplay (void);

boolean tablepushnodestyle (hdlheadrecord);

boolean tabletitleclick (Point);

boolean tablefindcolumnguide (Point, short *);

boolean tableadjustcolwidth (Point, short);

boolean tablechecksortorder (void);

void tableupdatecoltitles (boolean);

void tableupdate (void);

boolean tablegetlineheight (hdlheadrecord, short *);

boolean tablegetlinewidth (hdlheadrecord, short *);

boolean tabledrawline (hdlheadrecord, const Rect *, boolean, boolean);

boolean tablegettextrect (hdlheadrecord, const Rect *, Rect *);

boolean tablegetedittextrect (hdlheadrecord, const Rect *, Rect *);

boolean tablegeticonrect (hdlheadrecord, const Rect *, Rect *);

boolean tablepredrawline (hdlheadrecord, const Rect *, boolean, boolean);

boolean tablepostdrawline (hdlheadrecord, const Rect *, boolean, boolean);

boolean tabledrawnodeicon (hdlheadrecord, const Rect *, boolean, boolean);

boolean tablegetnodeframe (hdlheadrecord, Rect *);

boolean tableadjustcursor (hdlheadrecord, Point, const Rect *);

boolean tablemouseinline (hdlheadrecord, Point, const Rect *, boolean *);

boolean tablereturnkey (tydirection);

boolean tablegetoutlinesize (long *, long *);

boolean tabledefaultdrawcell (hdlheadrecord, short, const Rect *);

boolean tablegetcellstring (hdlheadrecord, short, bigstring, boolean);

#endif
