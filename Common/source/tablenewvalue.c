
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

#include <standard.h>
#include "memory.h"
#include "lang.h"
#include "shell.rsrc.h"
#include "shellundo.h"
#include "tableinternal.h"
#include "tablestructure.h"



boolean tablemakenewvalue (void) {
	
	/*
	create a new, empty value below the current cell.
	
	to get the node to be inserted below the current position, we use
	tableoverridesort to patch the comparenodes langglobals callback and 
	look for the next node in the table
	*/
	
	register hdlhashtable ht;
	bigstring bsname;
	tyvaluerecord val;
	short newrow;
	hdlhashnode hnode, hnext;
	register boolean fl;
	
	pushundoaction (undotypingstring);
	
	if (tableempty ())
		newrow = 0;
	else
		newrow = (**tableformatsdata).rowcursor + 1;
	
	ht = tablegetlinkedhashtable ();
	
	pushhashtable (ht);
	
	hashgetnthnode (ht, newrow, &hnext); /*ignore result -- will be nil if rowcursor is last*/
	
	tableoverridesort (hnext);
	
	setemptystring (bsname);
	
	clearbytes (&val, longsizeof (val));
	
	fl = hashinsert (bsname, val);
	
	assert (tablevalidate (ht, true));
	
	tablerestoresort ();
	
	// (**ht).flneedsort = true;
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	if (hashgetnthnode (ht, newrow, &hnode))
		pushundostep ((undocallback) tableredoclear, (Handle) hnode);
	
	if (!tableeditentercell (newrow, namecolumn)) {
		
		undolastaction (false);
		
		return (false);
		}
	
	return (true);
	} /*tablemakenewvalue*/



