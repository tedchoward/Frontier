
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

#define langipcinclude


#ifndef langinclude

	#include "lang.h"

#endif

#ifndef __APPLEEVENTS__

	#include <AppleEvents.h>

#endif

/*types*/

typedef short tyipcmessageflags;


enum { /*ipcmessageflags*/
	
	normalmsg = 0,
	
	noreplymsg = 0x01,
	
	transactionmsg = 0x02,
	
	microsoftmsg = 0x04,
	
	systemmsg = 0x08
	};


/*globals*/

#ifndef THINK_C

	extern typrocessid langipcself;

#endif


/*prototypes*/

extern boolean langipcerrorroutine (bigstring, ptrvoid); /*langipc.c*/

extern boolean setdescriptorvalue (AEDesc, tyvaluerecord *);

extern boolean valuetodescriptor (tyvaluerecord *, AEDesc *);

#ifdef landinclude

extern boolean langipcbuildparamlist (hdltreenode, hdlverbrecord, hdltreenode *);

extern boolean langipcpushparam (tyvaluerecord *, typaramkeyword, hdlverbrecord);

#endif

extern boolean langipcfindapptable (OSType , boolean, hdlhashtable *, bigstring);

extern boolean langipcbrowsenetwork (hdltreenode, tyvaluerecord *);

extern boolean langipcsettimeout (hdltreenode, tyvaluerecord *);

extern boolean langipcsettransactionid (hdltreenode, tyvaluerecord *);

extern boolean langipcsetinteractionlevel (hdltreenode, tyvaluerecord *);

extern boolean langipcgeteventattr (hdltreenode, tyvaluerecord *);

extern boolean langipccoerceappleitem (hdltreenode, tyvaluerecord *);

extern boolean langipcapprunning (hdltreenode, tyvaluerecord *);

extern boolean langipcgetaddressvalue (hdltreenode, tyvaluerecord *);

extern void binarytodesc (Handle, AEDesc *);

extern boolean langipcconvertoplist (const tyvaluerecord *, AEDesc *);

extern boolean langipcconvertaelist (const AEDesc *, tyvaluerecord *);

extern boolean langipcputlistitem (hdltreenode, tyvaluerecord *);

extern boolean langipcgetlistitem (hdltreenode, tyvaluerecord *);

extern boolean langipccountlistitems (hdltreenode, tyvaluerecord *);

extern boolean newselfaddressedevent (AEEventID id, AppleEvent *event);

extern boolean langipcmessage (hdltreenode, tyipcmessageflags, tyvaluerecord *);

extern boolean langipccomplexmessage (hdltreenode, tyvaluerecord *);

extern boolean langipctablemessage (hdltreenode, tyvaluerecord *);

extern boolean langipcbuildsubroutineevent (AppleEvent *, bigstring, hdltreenode);

extern boolean langipchandlercall (hdltreenode, bigstring, hdltreenode, tyvaluerecord *);

extern boolean langipckernelfunction (hdlhashtable, bigstring, hdltreenode, tyvaluerecord *);

extern boolean langipcshowmenunode (long);

extern boolean langipcnoop (void);

extern boolean langipcstart (void);

extern void langipcshutdown (void);

extern boolean langipcinit (void);


extern boolean langipcgetmenuhandle (OSType, short, Handle *); /*langipcmenus.c*/

#ifdef flnewfeatures

extern boolean langipcgetitemlangtext (long, short, short, Handle *, long *);

extern boolean langipccheckformulas (long);

extern void langipcdisposemenuarray (long, Handle);

#endif

extern boolean langipcrunitem (long, short, short, long *);

extern boolean langipckillscript (long);

extern boolean langipcgetmenuarray (long, short, boolean, Handle *);

extern boolean langipcmenustartup (void);

extern boolean langipcmenushutdown (void);

extern boolean langipcsymbolchanged (hdlhashtable, const bigstring, boolean);

extern boolean langipcsymbolinserted (hdlhashtable, const bigstring);

extern boolean langipcsymboldeleted (hdlhashtable, const bigstring);

extern boolean langipcmenuinit (void);




