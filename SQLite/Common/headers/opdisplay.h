
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

#define opdisplayinclude


extern short opnodeindent (hdlheadrecord);

extern void oplineinval (long);

extern void opscrollrect (Rect r, long dh, long dv);

extern hdlheadrecord oppointnode (Point);

extern void opupdatenow (void);

extern boolean opinitdisplayvariables (void);

extern void oppushheadstyle (hdlheadrecord);

extern void opgettextrect (hdlheadrecord, const Rect *, Rect *);

extern short opgetlineheight (hdlheadrecord);

extern short opgetlinewidth (hdlheadrecord);

extern long opgetnodelinecount (hdlheadrecord);

extern boolean opgetnoderect (hdlheadrecord, Rect *);

extern void operaserect (Rect r);

extern boolean opgetscreenline (hdlheadrecord, long *);

extern boolean opinvalnode (hdlheadrecord);

extern void opinvalstructure (hdlheadrecord);

extern void opinvalafter (hdlheadrecord);

extern void opinvalbarcursor (void);

extern void opinvaldisplay (void);

extern void opsmashdisplay (void);

extern boolean opdirtymeasurements (void);

extern boolean oppostfontchange (void);

extern void opgetlineselected (hdlheadrecord, boolean *, boolean *);

extern boolean opgetlinerect (long, Rect *);

extern void opdrawicon (hdlheadrecord, Rect);

extern void opdrawline (hdlheadrecord, Rect);

extern void opindenteddisplay (void);

extern void opdocursor (boolean);

extern void opmakegap (long, short);

extern void opexpandupdate (hdlheadrecord);

extern boolean opscroll (tydirection, boolean, long);

extern void opjumpdisplayto (hdlheadrecord, hdlheadrecord);

extern boolean opneedvisiscroll (hdlheadrecord, long *, long *, boolean);

extern void opdovisiscroll (long, long);

extern boolean opnodevisible (hdlheadrecord);

extern boolean opvisinode (hdlheadrecord, boolean);

extern void opvisisubheads (hdlheadrecord);

extern void operasedisplay (void);

extern short opmaxlevelwidth (hdlheadrecord);

extern boolean opdefaultpredrawline (hdlheadrecord, const Rect *, boolean, boolean);

extern boolean opdefaultdrawtext (hdlheadrecord, const Rect *, boolean, boolean);

extern boolean opdefaultpostdrawline (hdlheadrecord, const Rect *, boolean, boolean);

extern boolean opdefaultgetlineheight (hdlheadrecord, short *);

extern boolean opdefaultgetlinewidth (hdlheadrecord, short *);

extern boolean opdefaultgettextrect (hdlheadrecord, const Rect *, Rect *);

extern boolean opdefaultgetfullrect (hdlheadrecord, Rect *);

