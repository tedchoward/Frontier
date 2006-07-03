
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

#ifndef tableverbinclude
#define tableverbinclude


#include "langexternal.h"


/*prototypes*/


extern short tablecomparenodes (hdlhashtable, hdlhashnode, hdlhashnode); /*tablecompare.c*/


extern boolean tableverbfind (hdlexternalvariable, boolean *); /*tablefind.c*/

extern boolean tableverbcontinuesearch (hdlexternalvariable);


extern boolean tablefindvariable (hdlexternalvariable, hdlhashtable *, bigstring); /*tableops.c*/

extern boolean findnamedtable (hdlhashtable, bigstring, hdlhashtable *);

extern boolean findinparenttable (hdlhashtable, hdlhashtable *, bigstring);

extern boolean tableverbunload (hdlexternalvariable);

extern boolean tablemovetoname (hdlhashtable, bigstring);

extern hdldatabaserecord tablegetdatabase (hdlhashtable);


extern boolean tableverbmemorypack (hdlexternalvariable, Handle *, hdlhashnode); /*tablepack.c*/

extern boolean tableverbmemoryunpack (Handle, long *, hdlexternalvariable *, boolean);

extern boolean tableverbpack (hdlexternalvariable, Handle *, boolean *);

extern boolean tableverbunpack (Handle, long *, hdlexternalvariable *, boolean);

extern boolean tableverbpacktotext (hdlexternalvariable, Handle);

extern boolean tableverbgettimes (hdlexternalvariable, long *, long *, hdlhashnode);

extern boolean tableverbsettimes (hdlexternalvariable, long, long, hdlhashnode);

extern boolean tableverbfindusedblocks (hdlexternalvariable, bigstring bspath);


extern boolean tableclienttitlepopuphit (Point, hdlexternalvariable); /*tablepopup.c*/


extern boolean tabledroppasteroutine (void); /*tablescrap*/


extern boolean tableinitverbs (void); /*tableverbs.c*/

extern boolean gettablevalue (hdltreenode, short, hdlhashtable *);


extern boolean tablewindowopen (hdlexternalvariable, hdlwindowinfo *); /*tableexternal.c*/

extern boolean tablevaltotable (tyvaluerecord, hdlhashtable *, hdlhashnode);

extern boolean tableverbgetdisplaystring (hdlexternalvariable, bigstring);

extern boolean tableverbgettypestring (hdlexternalvariable, bigstring);

extern boolean tableverbgetsize (hdlexternalvariable, long *);

extern boolean tableverbinmemory (hdlexternalvariable, hdlhashnode);

extern boolean tableverbdispose (hdlexternalvariable, boolean);

extern boolean tableverbnew (hdlexternalvariable *);

extern boolean tableverbisdirty (hdlexternalvariable);

extern boolean tableverbsetdirty (hdlexternalvariable, boolean);

extern boolean tableedit (hdlexternalvariable, hdlwindowinfo, ptrfilespec, bigstring, rectparam);

extern boolean tablewindowclosed (hdlexternalvariable);

extern boolean tablezoomfromtable (hdlhashtable);

extern boolean tablezoomtoname (hdlhashtable, bigstring);

extern boolean tableclientsurface (hdlexternalvariable);

extern boolean tableverbwindowopen (short);

extern boolean tablepushglobals (hdlhashtable);

extern boolean tablesymbolchanged (hdlhashtable, const bigstring, hdlhashnode, boolean);

extern boolean tablesymbolinserted (hdlhashtable, const bigstring);

extern boolean tablesymboldeleted (hdlhashtable, const bigstring);

extern boolean tableverbclose (void);

extern boolean tableverbsetglobals (void);

extern boolean tableverbsetupdisplay (hdlhashtable, hdlwindowinfo);

extern boolean tableresetformatsrects (void);

extern boolean tablestart (void);

#endif



