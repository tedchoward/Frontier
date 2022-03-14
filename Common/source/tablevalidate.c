
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

#include "cursor.h"
#include "memory.h"
#include "frontier_strings.h"
#include "shell.h"
#include "langinternal.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"




static boolean flonlyinmemory = true;


static boolean validate (hdlhashtable htable, boolean flalert) {
	
	/*
	a bullshit detector for hash tables.  written on 3/21/90 by DW.
	
	we call it invalid if:
	
	1. the table handle is nil -- there's nothing we can do with such a table!
	
	2. one of the hashbuckets has an item chained into it that shouldn't be in 
	its list.  this is a very strong test -- it seems improbable if the chain
	is somehow invalid that all the handles would work out to point to a key
	string that just happens to hash to the same bucket number.
	
	3. the sorted list is out of order -- this seems like a little icing on the
	cake -- how could it pass test 2 and not pass this test?
	
	11/13/90 DW: relaxed the rule about the sorted list being in order, since
	we're allowing it.
	
	11/14/90 DW: added flonlyinmemory -- if false, not being in memory is not an 
	obstacle.  side-effect of doing a full traversal -- all the tables get loaded
	into memory.
	*/
	
	register hdlhashtable ht = htable;
	register short i;
	register hdlhashnode x;
	hdltablevariable hvariable;
	short errcode;
	
	if (ht == nil) {
		
		if (flalert)
			shellinternalerror (idnilhashtable, BIGSTRING ("\x17" "hashtable handle is nil"));
		
		return (false);
		}
	
	if (!(**ht).fllocaltable) { /*8/15/92 dmb*/
		
		for (i = 0; i < (**ht).cttmpstack; i++) {
			
			if ((**ht).tmpstack [i].data.binaryvalue != nil) {
				
				if (flalert)
					shellinternalerror (idnilhashtable, BIGSTRING ("\x14" "non-empty temp stack"));
				
				return (false);
				}
			}
		}
	
	for (i = 0; i < ctbuckets; i++) {
		
		x = (**ht).hashbucket [i];
		
		while (x != nil) { /*chain through the hash list*/
			
			bigstring bs;
			
			if (!flonlyinmemory)
				rollbeachball ();
			
			gethashkey (x, bs);
			
			if (gethandlesize ((Handle) x) != (long) sizeof (tyhashnode) + stringsize (bs)) {
				
				if (flalert)
					shellinternalerror (idbadbucketliststring, BIGSTRING ("\x17" "bad string in hash node"));
				
				return (false);
				}
			
			if (hashfunction (bs) != i) {
				
				if (flalert)
					shellinternalerror (idbadbucketliststring, BIGSTRING ("\x1b" "bad string in a bucket list"));
				
				return (false);
				}
			
			if (!gettablevariable ((**x).val, &hvariable, &errcode))
				goto nextx;
			
			if (!(**hvariable).flinmemory) {
				
				if (flonlyinmemory)
					goto nextx;
					
				if (!tableverbinmemory ((hdlexternalvariable) hvariable, x)) {
					
					if (flalert)
						shellinternalerror (iderrorloadingtable, BIGSTRING ("\x13" "error loading table"));
					
					return (false);
					}
				}
			
			assert (tablesetdebugglobals (ht, x));
			
			if (!validate ((hdlhashtable) (**hvariable).variabledata, flalert)) /*recurse*/
				return (false);
			
			nextx:
			
			x = (**x).hashlink; /*advance to next node in chain*/
			} /*while*/
		} /*for*/
	
	/*
	x = (**ht).hfirstsort;
	
	setstringlength (bslast, 0);
	
	while (x != nil) {
		
		bigstring bs;
		
		gethashkey (x, bs);
		
		if (!stringlessthan (bslast, bs)) {
		
			if (flalert)
				shellinternalerror (idunsortedhashlist, "\psorted list out of order");
			
			return (false);
			}
		
		x = (**x).sortedlink;
		
		copystring (bs, bslast);
		} /%while%/
	*/
	
	return (true);
	} /*validate*/


boolean tablevalidate (hdlhashtable htable, boolean flalert) {
	
	flonlyinmemory = true;
	
	return (validate (htable, flalert));
	} /*tablevalidate*/




