
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

#define pictverbsinclude

/*prototypes*/

extern boolean pictverbgetdisplaystring (hdlexternalvariable, bigstring);

extern boolean pictverbgettypestring (hdlexternalvariable, bigstring);

extern boolean pictverbdispose (hdlexternalvariable, boolean);

extern boolean pictverbnew (Handle, hdlexternalvariable *);

extern boolean pictverbisdirty (hdlexternalvariable);

extern boolean pictverbsetdirty (hdlexternalvariable, boolean);

extern boolean pictverbmemorypack (hdlexternalvariable, Handle *);

extern boolean pictverbmemoryunpack (Handle, long *, hdlexternalvariable *);

extern boolean pictverbpack (hdlexternalvariable, Handle *, boolean *);

extern boolean pictverbunpack (Handle, long *, hdlexternalvariable *);

extern boolean pictverbpacktotext (hdlexternalvariable, Handle);

extern boolean pictverbgetsize (hdlexternalvariable, long *);

extern boolean pictverbgettimes (hdlexternalvariable, long *, long *);

extern boolean pictverbsettimes (hdlexternalvariable, long, long);

extern boolean pictwindowopen (hdlexternalvariable, hdlwindowinfo *);

extern boolean pictedit (hdlexternalvariable, hdlwindowinfo, ptrfilespec, bigstring, rectparam);

extern boolean pictverbfind (hdlexternalvariable, boolean *);

extern boolean pictstart (void);



