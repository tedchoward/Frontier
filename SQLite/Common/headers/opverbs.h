
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

#define opverbsinclude

#ifndef opinclude
	
	#include "op.h"

#endif


/*prototypes*/

extern boolean opverbgetsummitstring (hdlexternalvariable, bigstring);

extern boolean opverbgettypestring (hdlexternalvariable, bigstring);

extern boolean opverbdispose (hdlexternalvariable, boolean);

extern void opverbunload (hdlexternalvariable, dbaddress);

extern boolean opverbisdirty (hdlexternalvariable);

extern boolean opverbsetdirty (hdlexternalvariable, boolean);

extern boolean opverblinkcode (hdlexternalvariable, Handle);

extern boolean opverbgetlinkedcode (hdlexternalvariable, hdltreenode *);

extern boolean opverbmemorypack (hdlexternalvariable, Handle *);

extern boolean opverbmemoryunpack (Handle, long *, hdlexternalvariable *);

extern boolean opverbscriptmemoryunpack (Handle, long *, hdlexternalvariable *);

extern boolean opverbpack (hdlexternalvariable, Handle *, boolean *);

extern boolean opverbunpack (Handle, long *, hdlexternalvariable *);

extern boolean opverbscriptunpack (Handle, long *, hdlexternalvariable *);

extern boolean opverbpacktotext (hdlexternalvariable, Handle);

extern boolean opverbgetsize (hdlexternalvariable, long *);

extern boolean opverbgettimes (hdlexternalvariable, long *, long *);

extern boolean opverbsettimes (hdlexternalvariable, long, long);

extern boolean opverbnew (short, Handle, hdlexternalvariable *);

extern boolean opverbcopyvalue (hdlexternalvariable, hdlexternalvariable *);

extern boolean opverbgetlangtext (hdlexternalvariable, boolean, Handle *, long *);

extern boolean getoutlinevalue (hdltreenode, short, hdloutlinerecord *);

extern boolean opverbarrayreference (hdlexternalvariable, long, hdlheadrecord *);

extern boolean opwindowopen (hdlexternalvariable, hdlwindowinfo *);

extern boolean opedit (hdlexternalvariable, hdlwindowinfo, ptrfilespec, bigstring, rectparam);

extern boolean opvaltoscript (tyvaluerecord, hdloutlinerecord *);

extern boolean opverbgetheadstring (hdlheadrecord, bigstring);

extern boolean opverbclose (void);

extern boolean opverbfind (hdlexternalvariable, boolean *);

extern boolean opverbruncursor (void);

extern boolean opverbgetvariable (hdlexternalvariable *);

extern boolean opverbgettargetdata (short);

extern boolean opstart (void);



