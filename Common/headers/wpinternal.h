
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

#define wpinternalinclude /*so other includes can tell if we've been loaded*/



#undef abs
#include "paige.h"


#undef flvariables

#define pasbool(fl) ((fl)? TRUE : FALSE)


/*variables*/

extern pg_ref wpbuffer;



/*prototypes*/

extern boolean wpsetglobals (void); /*wpengine.c*/

extern boolean wppreedit (void);

extern void wppostedit (boolean);

extern void wpresettyping (void);

extern boolean wppushinsertundo (boolean, boolean);

extern boolean wppushformatundo (void);

extern boolean wpdelete (boolean);

extern boolean wpgetpackedmaxpos (Handle, long *);



#ifdef flvariables


extern boolean wpvariablerecalc (void); /*wpvariables*/

extern boolean wpdeletevariable (bigstring);

extern void wpdisposevariablelist (Handle);

extern boolean wpgetnthvariable (short, bigstring);

extern boolean wpgetvariablevalue (bigstring, bigstring);

extern boolean wpgetvariablevalue (bigstring, bigstring);

extern boolean wpsetvariablevalue (bigstring, bigstring);

extern boolean wpnewvariable (bigstring, bigstring);

extern boolean wppackvariablelist (Handle *);

extern boolean wpunpackvariablelist (Handle);

extern boolean wpgetvariablestring (FormatRec *, bigstring);

extern boolean wpremovevariablestyle (void);

extern boolean wpinsertvariable (bigstring);

extern boolean wpgetevaluate (void);

extern boolean wpsetevaluate (boolean);

extern void wphilitevariables (boolean);

extern void wpvariableinit (void);

#endif


