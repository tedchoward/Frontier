
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

#define langsystem7include


/*
#ifndef __ALIASES__

	#include <Aliases.h>

#endif
*/


#ifndef langinclude

	#include "lang.h"
	
#endif


	#define filespecsize(fs) (sizeof (tyfilespec)) // - sizeof (CFStringRef) + stringsize ((fs).name))



typedef boolean (*langvisitlistvaluescallback) (tyvaluerecord *, ptrvoid); /*2003-04-28 AR*/


/*prototypes*/

extern boolean filespectoalias (const ptrfilespec , boolean, AliasHandle *);  /*landsystem7.c*/

extern boolean aliastostring (Handle, bigstring);

extern boolean aliastofilespec (AliasHandle, ptrfilespec );

extern boolean coercetoalias (tyvaluerecord *);

extern boolean langpackfileval (const tyvaluerecord *, Handle *);

extern boolean langunpackfileval (Handle, tyvaluerecord *);

extern boolean objspectoaddress (tyvaluerecord *);

extern boolean objspectofilespec (tyvaluerecord *);

extern boolean filespectoobjspec (tyvaluerecord *);

extern boolean objspectostring (Handle, bigstring);

extern boolean coercetoobjspec (tyvaluerecord *);

extern boolean setobjspecverb (hdltreenode, tyvaluerecord *);

extern boolean evaluateobjspec (hdltreenode, tyvaluerecord *);

extern boolean isobjspectree (hdltreenode);


extern boolean filespecaddvalue (tyvaluerecord *, tyvaluerecord *, tyvaluerecord *);

extern boolean filespecsubtractvalue (tyvaluerecord *, tyvaluerecord *, tyvaluerecord *);

extern boolean getobjectmodeldisplaystring (tyvaluerecord *, bigstring);

extern boolean makelistvalue (hdltreenode, tyvaluerecord *);

extern boolean makerecordvalue (hdltreenode, boolean, tyvaluerecord *);

extern boolean langgetlistsize (const tyvaluerecord *, long *);

#ifdef oplanglists
	extern boolean langgetlistitem (const tyvaluerecord *, long, ptrstring, tyvaluerecord *);
	
	extern boolean langpushlistval (struct tylistrecord **, ptrstring, tyvaluerecord *);

	extern boolean langpushlisttext (struct tylistrecord ** hlist, Handle hstring);
	
	extern boolean langpushliststring (struct tylistrecord ** hlist, bigstring bs);
	
	extern boolean langpushlistaddress (struct tylistrecord ** hlist, hdlhashtable ht, bigstring bs);
	
	extern boolean langpushlistlong (struct tylistrecord ** hlist, long num);
	
	extern boolean getnthlistval (struct tylistrecord ** hlist, long n, ptrstring pkey, tyvaluerecord *val);

	extern boolean setnthlistval (struct tylistrecord ** hlist, long n, ptrstring pkey, tyvaluerecord *val);
#else
	extern boolean langgetlistitem (const tyvaluerecord *, long, OSType *, tyvaluerecord *);
#endif

extern boolean coercetolist (tyvaluerecord *, tyvaluetype);

extern boolean coercelistvalue (tyvaluerecord *, tyvaluetype);

extern boolean listaddvalue (tyvaluerecord *, tyvaluerecord *, tyvaluerecord *);

extern boolean listsubtractvalue (tyvaluerecord *, tyvaluerecord *, tyvaluerecord *);

extern boolean listcomparevalue (tyvaluerecord *, tyvaluerecord *, tytreetype, tyvaluerecord *);

extern boolean coercetolistposition (tyvaluerecord *);
	
extern boolean listarrayvalue (tyvaluerecord *, bigstring, tyvaluerecord *, tyvaluerecord *);

extern boolean listassignvalue (tyvaluerecord *, bigstring, tyvaluerecord *, tyvaluerecord *);

extern boolean listdeletevalue (tyvaluerecord *, bigstring, tyvaluerecord *);

extern boolean langvisitlistvalues (tyvaluerecord *, langvisitlistvaluescallback, ptrvoid); /*2003-04-28 AR*/



