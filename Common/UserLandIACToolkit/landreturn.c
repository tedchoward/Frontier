
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

#include <AERegistry.h>

#include "landinternal.h"


/*
landreturn.c -- these routines can be called by a client application to return
a value after processing a verb record.  to support a synchronous call, the client
must return some value -- if nothing is obvious, just return a boolean true.

you can return multiple values.  when we get the first return value, we empty out 
the verb's parameter list.  all returned values are pushed just like normal 
parameters, and are returned to the calling application by landsendreturnvalues 
when the verb handler returns control to the userland api
*/




pascal boolean landstartreturn (hdlverbrecord hverb) {
	
	/*
	make sure that the original parameters are cleared out of the 
	verb handle so that return values can be pushed.  note that it's
	ok to call this more than once -- it will only do something the 
	first time.
	*/
	
	register hdlverbrecord hv = hverb;
	
	if ((**hv).verbtoken != returntoken) {
		
		(**hv).verbtoken = returntoken;
		}
	
	return (true);
	} /*landstartreturn*/


pascal boolean landreturnboolean (hdlverbrecord hverb, Boolean x) {
	
	unsigned char bool;
	
	bool = bitboolean (x);
	
	landstartreturn (hverb);
	
	return (landpushparam (hverb, booleantype, nil, &bool, longsizeof (bool), returnedvaluekey));
	} /*landreturnboolean*/


pascal boolean landreturnlong (hdlverbrecord hverb, long x) {
	
	landstartreturn (hverb);
	
	return (landpushlongparam (hverb, x, returnedvaluekey));
	} /*landreturnlong*/
	

pascal boolean landreturnstring (hdlverbrecord hverb, bigstring bs) {
	
	landstartreturn (hverb);
	
	return (landpushstringparam (hverb, bs, returnedvaluekey));
	} /*landreturnstring*/


pascal boolean landreturnbinary (hdlverbrecord hverb, Handle x) {
	
	landstartreturn (hverb);
	
	return (landpushparam (hverb, binarytype, x, nil, 0L, returnedvaluekey));
	} /*landreturnbinary*/


pascal boolean landreturnerror (hdlverbrecord hverb, short iderror) {
	
	/*
	our client has a problem processing the indicated verb record.  we return info
	about the error to the calling program.
	
	if it's a scripting system, the error might be displayed in a scripting error
	dialog, with a pointer to the offending line.
	
	10/31/90 DW: we're also using this as an opportunity to test the new toolkit 
	feature that allows multiple returned values.  we return the string as the 
	primary value, but we also provide the index into the string list as an id for 
	the error.
	
	11/6/90 dmb: now use landstartreturn to return multiple values. also, use Apple's 
	keys for error values.
	
	1/28/92 dmb: convert Toolkit errors to standard Mac errors; only push error number/
	string params if it's not a standard error.
	
	4/13/93 dmb: don't push any AE params for standard Mac errors.  fixes a bug in 
	Runtime's handling of send-to-self events that are handled by a system event handler.
	
	3.0.1b1 dmb: for error ids greater than the land errors, the string list is 
	used. the string list no longer needs to have strings for the land errors, 
	because we never use them. also, we push errAEEventFailed into the reply, 
	not the iderror
	*/
	
	register hdllandglobals hg = landgetglobals ();
	bigstring bs;
	OSErr errcode;
	
	landstartreturn (hverb);
	
	switch (iderror) {
	
		case nosuchparamerror:
			errcode = errAEDescNotFound; break;
	
		case wrongtypeerror:
			errcode = errAEWrongDataType; break;
		
		case undefinedverberror:
			errcode = errAEEventNotHandled; break;
		
		case nohandlererror:
			errcode = errAEEventNotHandled; break;
		
		case outofmemoryerror:
			errcode = memFullErr; break;
		
		case usercancellederror:
			errcode = userCanceledErr; break;
		
		default:
			GetIndString (bs, (**hg).iderrorlist, iderror);
			
			landpushintparam (hverb, errAEEventFailed, errornumberkey);
			
			landpushstringparam (hverb, bs, errorstringkey);
			
			errcode = errAEEventFailed;
		}
	
	landseterror (errcode);
	
	return (true);
	} /*landreturnerror*/


