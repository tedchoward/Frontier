
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

#include "memory.h"
#include "lang.h" 
#include "langinternal.h"
#include "langexternal.h"




#define ctgrowtmpstack 5 /*amount to grow tmpstack by when full*/



void cleartmpstack (void) {

	/*
	the tmpstack hold all the temporary handles that have been allocated 
	since the last time the tmpstack was cleared.
	
	these are values that are produced in evaluating an expression. the 
	evaluator has no way of knowing whether they will be needed so they 
	can't be released, yet.
	
	this is the price you pay for a language that allows everything to
	return a value, and where values can be heap-allocated objects like
	strings.
	
	call this routine when you are advancing to the next statement and
	know that you will not be using any of the temporaries in the stack.
	
	1/8/91 dmb: check currenthashtable for nil
	*/
	
	register short ctloops;
	register tyvaluerecord *p;
	
	if (currenthashtable == nil)
		return;
	
	lockhandle ((Handle) currenthashtable); 
	
	p = (**currenthashtable).tmpstack;
	
	for (ctloops = (**currenthashtable).cttmpstack; ctloops--; ++p) { /*step through tmpstack*/
		
		if ((*p).valuetype != novaluetype) {
			
			(*p).fltmpstack = false;
			
			disposevaluerecord (*p, false);
			
			initvalue (p, novaluetype);
			}
		} /*for*/
		
	unlockhandle ((Handle) currenthashtable);
	} /*cleartmpstack*/


boolean pushtmpstackvalue (tyvaluerecord *vpush) {
	
	/*
	1/14/91 dmb: check currenthashtable for nil.  this might happen when 
	setstringvalue or copyvaluerecord is called outside of language 
	execution.  in these situations, the caller should be managing the memory.
	*/
	
	register short ctloops;
	register tyvaluerecord *p;
	
	if ((*vpush).data.binaryvalue == nil) /*nothing to push*/
		return (true);
	
	if (currenthashtable == nil) /*not an error, but caller must handle disposal*/
		return (true);
	
	assert (validhandle ((*vpush).data.stringvalue));
	
	p = (**currenthashtable).tmpstack;
	
	for (ctloops = (**currenthashtable).cttmpstack; ctloops--; ++p) { /*step through tmpstack*/
	
		if ((*p).valuetype == novaluetype) { /*found an empty slot in tmpstack*/
			
			(*vpush).fltmpstack = true;
			
			*p = *vpush;
			
			return (true);
			}
		} /*for*/
	
	// langerror (tmpstackoverflowerror); /*loop terminated, no room in tmpstack*/
	
	if (!enlargehandle ((Handle) currenthashtable, ctgrowtmpstack * sizeof (tyvaluerecord), nil))
		return (false);
	
	(*vpush).fltmpstack = true;
	
	(**currenthashtable).tmpstack [(**currenthashtable).cttmpstack] = *vpush;
	
	(**currenthashtable).cttmpstack += ctgrowtmpstack;
	
	return (true);
	} /*pushtmpstackvalue*/


boolean pushtmpstack (Handle h) {
	
	tyvaluerecord val;
	
	if (h == nil) /*nothing to push*/
		return (true);
	
	assert (validhandle (h));
	
	if (currenthashtable == nil) /*not an error, but caller must handle disposal*/
		return (true);
	
	initvalue (&val, stringvaluetype);
	
	val.data.binaryvalue = h;
	
	return (pushtmpstackvalue (&val));
	} /*pushtmpstack*/


static boolean removeheaptmp (Handle h) {
	
	/*
	remove the handle from the tmpstack.  this keeps it from getting
	released when we call cleartmpstack.
	
	1/8/91 dmb: check currenthashtable for nil
	
	1/16/91 dmb: return boolean
	
	3/23/93 dmb: exported
	*/
	
	register short ctloops;
	register tyvaluerecord *p;
	
	if (currenthashtable == nil)
		return (false);
	
	p = (**currenthashtable).tmpstack;
	
	for (ctloops = (**currenthashtable).cttmpstack; ctloops--; ++p) { /*step through tmpstack*/
		
		if ((*p).data.binaryvalue == h) { /*found the temp in the stack*/
			
			initvalue (p, novaluetype); /*nil the entry so it can be re-used*/
			
			return (true);
			}
		} /*while*/
	
	return (false); /*didn't find it*/
	} /*removeheaptmp*/


void releaseheaptmp (Handle h) {
	
	/*
	we're releasing a heap-allocated temporary value.  we make sure that its
	entry in the tmpstack is nilled so it doesn't get released when the 
	tmpstack is cleared.
	*/
	
	if (h != nil) {
		
		removeheaptmp (h);
		
		disposehandle (h); 
		//Code change by Timothy Paustian Friday, May 19, 2000 1:06:24 PM
		//This handle is not nilled out, just disposed of. Maybe put a nil ref in here.
		//There are some stale handle references as noticed by spotlight.
		//see if this will get us a better clue on where the problem is.
		}
	} /*releaseheaptmp*/


boolean pushvalueontmpstack (tyvaluerecord *val) {

	/*
	6.1d9 AR: Allow pushing of external types, too.
	*/
	
	Handle h;
	
	if (!langheapallocated (val, &h)) /*nothing to push, not heap allocated*/
		return (true); 
	
	if ((*val).valuetype == codevaluetype)
		return (false);
/*
	if ((*val).valuetype == externalvaluetype)
		return (false);
*/	
	return (pushtmpstackvalue (val));
	} /*pushvalueontmpstack*/


boolean exemptfromtmpstack (tyvaluerecord *val) {
	
	/*
	the caller is asserting that the value must be removed from the tmpstack.
	
	when you're moving a temporary into a symbol table, if we were to automatically 
	release it, the value would disappear.
	
	1/16/91 dmb: return boolean so caller can tell if it was in the temp stack
	*/
	
	Handle h;
	
	if (!langheapallocated (val, &h)) 
		return (false);
	
	if ((*val).fltmpstack) {
		
		if (removeheaptmp (h)) {
			
			(*val).fltmpstack = false;
			
			return (true);
			}
		}
	
	return (false);
	} /*exemptfromtmpstack*/
	

boolean disposetmpvalue (tyvaluerecord *val) {
	
	/*
	val is a value that normally would have been pushed onto the tempstack, 
	but wasn't because it was the returned value at the highest level of 
	evaluatelist.
	
	if it's indeed a value that normally would be a temp, delete it.

	6.1d9 AR: Dispose of external types, too.
	*/
	
	Handle h;
	
	if (!langheapallocated (val, &h)) /*nothing to delete, not heap allocated*/
		return (true);
	
	if ((*val).valuetype == codevaluetype)
		return (false);
/*	
	if ((*val).valuetype == externalvaluetype)
		return (false);
*/
	assert (!(*val).fltmpstack);
	
	disposevaluerecord (*val, false);
	
	return (true);
	} /*disposetmpvalue*/


