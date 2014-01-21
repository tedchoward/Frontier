
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

#ifndef claybrowserstrucinclude
#define claybrowserstrucinclude


#include "claybrowser.h"

extern boolean lineinsertedcallbackdisabled;

extern boolean browserdeletenodeswithtmpbitset (void);

extern boolean browsercommitchanges (void);

extern boolean browsercopyrefcon (hdlheadrecord, hdlheadrecord);

extern boolean browsertextualizerefcon (hdlheadrecord, Handle);

extern boolean browserreleaserefcon (hdlheadrecord, boolean);

extern boolean browserpredrag (hdlheadrecord *, tydirection *);

extern boolean browserdragcopy (hdlheadrecord, hdlheadrecord);

extern boolean browserexportscrap (hdloutlinerecord);

extern void browsersortfolder (hdlheadrecord);

extern boolean browsergetparentspec (hdlheadrecord, tybrowserspec *);

extern void browserinsertagain (hdlheadrecord);

extern boolean browserlineinserted (hdlheadrecord);

extern boolean browserlinedeleted (hdlheadrecord);

extern boolean browserclearundo (void);

extern boolean browsersetscrap (hdloutlinerecord);

extern boolean browsergetscrap (hdloutlinerecord *, boolean *);

extern boolean browserdeletedummyvalues (hdlheadrecord);

#endif
