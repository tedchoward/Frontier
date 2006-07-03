
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

#define oplistinclude /*so other guys can tell if we've been included*/


#ifndef shellinclude

	#include "shell.h"

#endif


/*types*/

typedef struct tylistrecord **hdllistrecord;	/* 2004-11-04 aradke: now opaque */

typedef boolean (*oplistreleaseitemcallback) (Handle);	/* 2004-11-04 aradke: part of tylistrecord */

typedef boolean (*opvisitlistcallback) (Handle, ptrstring, ptrvoid);	/* 2004-11-04 aradke */


/*prototypes*/

extern boolean opnewlist (hdllistrecord *, boolean);

extern void opdisposelist (hdllistrecord);

extern boolean oppushhandle (hdllistrecord, ptrstring, Handle);

extern boolean opunshifthandle (hdllistrecord, ptrstring, Handle);

extern boolean oppushdata (hdllistrecord, ptrstring, ptrvoid, long);

extern boolean oppushstring (hdllistrecord, ptrstring, bigstring);

extern boolean opgetlistdata (hdllistrecord, long, ptrstring, ptrvoid, long);

extern boolean opgetlisthandle (hdllistrecord, long, ptrstring, Handle *);

extern boolean opgetliststring (hdllistrecord, long, ptrstring, bigstring);

extern boolean opsetlisthandle (hdllistrecord, long, ptrstring, Handle);

extern boolean opsetlistdata (hdllistrecord, long, ptrstring, ptrvoid, long);

extern long opcountlistitems (hdllistrecord);

extern boolean opgetisrecord (hdllistrecord);	/* 2004-11-04 aradke */

extern void opsetisrecord (hdllistrecord hlist, boolean flisrecord);	/* 2004-11-04 aradke */

extern oplistreleaseitemcallback opsetreleaseitemcallback (hdllistrecord, oplistreleaseitemcallback);	/* 2004-11-04 aradke */

extern boolean opdeletelistitem (hdllistrecord, long, ptrstring);

extern boolean oppacklist (hdllistrecord, Handle *);

extern boolean opunpacklist (Handle, hdllistrecord *);

extern boolean opcopylist (hdllistrecord, hdllistrecord *);

extern boolean oploadstringlist (short, hdllistrecord *);

extern boolean opvisitlist (hdllistrecord, opvisitlistcallback, ptrvoid);	/* 2004-11-04 aradke */

