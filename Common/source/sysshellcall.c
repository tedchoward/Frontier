
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

#include "frontier.h"
#include "standard.h"

#include "memory.h"
#include "threads.h"
#include "sysshellcall.h"

#include <stdio.h>
#include <stdlib.h>


boolean winshellcall (Handle hcommand, Handle hreturn) {

	/*
	2006-03-09 aradke: call the Windows shell and execute the given command.
		release thread globals before each sys call in order not to block other kernel threads.
	*/

	FILE *f;
	char buf [1024];
	long ct = 0;
		
	if (!enlargehandle (hcommand, 1, "\0"))
		return (false);
	
	lockhandle (hcommand);
	
	releasethreadglobals ();

	f = _popen (*hcommand, "r");
	
	unlockhandle (hcommand);

	grabthreadglobals ();

	while (true) {
		
		releasethreadglobals ();

		ct = fread (buf, 1, sizeof (buf), f); /*fread*/
		
		grabthreadglobals ();

		if (ct > 0)				
			if (!insertinhandle (hreturn, gethandlesize (hreturn), buf, ct))
				break;
		
		if (feof (f))
			break;

		} /*while*/
	
	_pclose (f);
	
	return (true);	
	} /*winshellcall*/

