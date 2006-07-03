
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

#include "landinternal.h"



pascal boolean landaddclass (tyverbclass class) {
	
	register hdllandglobals hg = landgetglobals ();	
	register boolean fl = true;
	
	(**hg).currentclass = class;
	
	if ((**hg).transport == macsystem7) {
		
		/*
		if (!landverbsupported (class, (tyverbtoken) 0))
		*/
			fl = landsystem7addclass (class);
		}
	
	return (fl);
	} /*landaddclass*/


static pascal boolean landaddverbtoken (tyverbtoken token, boolean flfasthandler) {
	
	/*
	push the token on the verbarray list.
	*/
	
	register hdllandglobals hg = landgetglobals ();	
	register hdlverbarray hverbs = (**hg).verbarray;
	tyverbarrayelement item;

	if (hverbs == nil) {
		
		hdlverbarray hnewarray;
		
		if (!landnewemptyhandle ((Handle *) &hnewarray))
			return (false);

		hverbs = hnewarray; /*copy into register*/
		
		(**hg).verbarray = hverbs;
		}
		
	item.class = (**hg).currentclass;
		
	item.token = token;
	
	item.flfasthandler = flfasthandler;
	
	if (!landenlargehandle ((Handle) hverbs, longsizeof (item), &item))
		return (false);
	
	if (!flfasthandler)
		return (true);
	
	return (landsystem7addfastverb (item.class, item.token));
	} /*landaddverbtoken*/


pascal boolean landaddverb (tyverbtoken token) {
	
	/*
	push the verb's token on the verbarray list.
	*/
	
	return (landaddverbtoken (token, false));
	} /*landaddverb*/


pascal boolean landaddfastverb (tyverbtoken token) {
	
	/*
	push the verb's token on the verbarray list and set it up to be invoked 
	as a fast handler
	*/
	
	return (landaddverbtoken (token, true));
	} /*landaddfastverb*/	




