
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

#ifdef MACVERSION
#include <standard.h>
#endif

#ifdef WIN95VERSION
#include "standard.h"
#endif

#include "strings.h"
#include "langexternal.h"
#include "tableinternal.h"
#include "tableverbs.h"




static short tablecomparenames (hdlhashnode hnode1, hdlhashnode hnode2) {

	bigstring bs1, bs2;
	
	gethashkey (hnode1, bs1);

	gethashkey (hnode2, bs2);
	
	alllower (bs1); /*comparison is unicase*/
	
	alllower (bs2);
	
	return (comparestrings (bs1, bs2));
	} /*tablecomparenames*/


#if !flruntime

static short tablecomparekinds (hdlhashnode hnode1, hdlhashnode hnode2) {
	
	tyvaluerecord val1, val2;
	register tyvaluetype t1, t2;
	
	val1 = (**hnode1).val;
	
	val2 = (**hnode2).val;
	
	t1 = val1.valuetype;
	
	t2 = val2.valuetype;
	
	if ((t1 == externalvaluetype) && (t2 == externalvaluetype)) {
		
		register hdlexternalhandle h1 = (hdlexternalhandle) val1.data.externalvalue;
		register hdlexternalhandle h2 = (hdlexternalhandle) val2.data.externalvalue;
		register tyexternalid id1 = (tyexternalid) (**h1).id;
		register tyexternalid id2 = (tyexternalid) (**h2).id;
		
		if (id1 == id2)
			return (tablecomparenames (hnode1, hnode2));
			
		return (langexternalcomparetypes (id1, id2));
		}
	
	if (t1 == externalvaluetype) /*scalars sort before externals*/
		return (1);
		
	if (t2 == externalvaluetype) /*scalars sort before externals*/
		return (-1);
		
	if (t1 == t2)
		return (tablecomparenames (hnode1, hnode2));
		
	return (sgn (t1 - t2));
	} /*tablecomparekinds*/


static short tablecomparevalues (hdlhashtable htable, hdlhashnode hnode1, hdlhashnode hnode2) {
	
	/*
	2.1b2 dmb: new version, corrects bugs that led to semi-random sorts.  now 
	only values of the same kind are compared, and the valuetype is a strict 
	secondary sort.
	*/
	
	register hdlhashnode h1 = hnode1;
	register hdlhashnode h2 = hnode2;
	tyvaluerecord val1, val2, vreturned;
	register boolean fllessthan;
	register boolean flequal = false;
	boolean flcomparable;
	
	flcomparable = (**h1).val.valuetype == (**h2).val.valuetype;
	
	if (flcomparable) {
		
		pushhashtable (htable); /*any temps are allocated in this table*/
		
		disablelangerror (); /*protect us from error dialogs*/
		
		copyvaluerecord ((**h1).val, &val1);
		
		copyvaluerecord ((**h2).val, &val2);
		
		flcomparable = LTvalue (val1, val2, &vreturned);
		
		cleartmpstack (); /*temps no longer needed*/
		
		fllessthan = vreturned.data.flvalue;
		
		if (flcomparable && !fllessthan) {
			
			copyvaluerecord ((**h1).val, &val1);
			
			copyvaluerecord ((**h2).val, &val2);
			
			flcomparable = EQvalue (val1, val2, &vreturned);
			
			cleartmpstack (); /*temps no longer needed*/
			
			flequal = vreturned.data.flvalue;
			}
		
		enablelangerror ();
		
		pophashtable ();
		}
	
	if ((!flcomparable) || flequal) 
		return (tablecomparekinds (hnode1, hnode2));
	
	if (fllessthan)
		return (-1);
	else
		return (1);
	} /*tablecomparevalues*/
	
#endif

short tablecomparenodes (hdlhashtable htable, hdlhashnode hnode1, hdlhashnode hnode2) {
	
	/*
	3/31/93 dmb: return a signed value indicating less than (-1), equality (0), 
	or greater than (1). modified all routines that we call to do the same.
	*/
	
	#if flruntime
	
		return (tablecomparenames (hnode1, hnode2));
	
	#else
		
		switch ((**htable).sortorder) {
			
			case sortbyname: 
				return (tablecomparenames (hnode1, hnode2));
				
			case sortbyvalue:
				return (tablecomparevalues (htable, hnode1, hnode2));
				
			case sortbykind:
				return (tablecomparekinds (hnode1, hnode2));
			
			default:
				return (0);
			} /*switch*/
		
	#endif
	
	} /*tablecomparenodes*/

	
#if !flruntime

static hdlhashnode nextnodecompare;
static langcomparenodescallback origcomparenodescallback;


static short tableoverridecomparenodes (hdlhashtable htable, hdlhashnode hnode1, hdlhashnode hnode2) {
	
	if (hnode2 == nextnodecompare)
		return (-1);
	else
		return (1);
	} /*tableoverridecomparenodes*/


void tableoverridesort (hdlhashnode hnext) {
	
	/*
	temporarily override the current sort, forcing the next node insertion 
	to "sorted" before the indicated node
	*/
	
	origcomparenodescallback = langcallbacks.comparenodescallback;
	
	langcallbacks.comparenodescallback = &tableoverridecomparenodes;
	
	nextnodecompare = hnext; /*set global for tableeditcomparenodes*/
	} /*tableoverridesort*/


void tablerestoresort (void) {

	langcallbacks.comparenodescallback = origcomparenodescallback; /*restore*/
	} /*tablerestoresort*/

#endif



