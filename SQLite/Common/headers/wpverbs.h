
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

#define wpverbsinclude


/*prototypes*/

extern boolean wpverbgetdisplaystring (hdlexternalvariable, bigstring);

extern boolean wpverbgettypestring (hdlexternalvariable, bigstring);

extern boolean wpverbdispose (hdlexternalvariable, boolean);

extern boolean wpverbisdirty (hdlexternalvariable);

extern boolean wpverbsetdirty (hdlexternalvariable, boolean);

extern boolean wpverbnew (Handle, hdlexternalvariable *);

extern boolean wpverbmemorypack (hdlexternalvariable, Handle *);

extern boolean wpverbmemoryunpack (Handle, long *, hdlexternalvariable *);

extern boolean wpverbpack (hdlexternalvariable, Handle *, boolean *);

extern boolean wpverbunpack (Handle, long *, hdlexternalvariable *);

extern boolean wpverbinmemory (hdlexternalvariable);

extern boolean wpverbpacktotext (hdlexternalvariable, Handle);

extern boolean wpverbgetsize (hdlexternalvariable, long *);

extern boolean wpverbgettimes (hdlexternalvariable, long *, long *);

extern boolean wpverbsettimes (hdlexternalvariable, long, long);

extern boolean wpwindowopen (hdlexternalvariable, hdlwindowinfo *);

extern boolean wpedit (hdlexternalvariable, hdlwindowinfo, ptrfilespec, bigstring, rectparam);

extern boolean wpverbfind (hdlexternalvariable, boolean *);

extern boolean wpstart (void);


