
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

#include "landinternal.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h" /*PBS 03/14/02: AE OS X fix.*/
#endif



pascal boolean landgetparam (hdlverbrecord hverb, typaramkeyword key, typaramtype type, typaramrecord *param) {
	
	/*
	search the parameters of the verb record for one with the indicated key.
	
	return the value and type of the parameter, and true if it was found.
	
	10/29/90 DW: added support for optional params.  the user sets the landglobals
	field true if the param is to be optional.  if we don't find one with the
	indicated key, we return false to our caller, without returning an error to 
	the IAC caller.  the landglobals field must be reset every for every getparam
	call.  it's implemented as a global because optional parameters are expected
	to be the exception, not the rule -- we don't clutter up our parameter lists
	with this boolean.
	
	10/30/90 DW: if type is notype, we don't care about the type of the parameter,
	we just look for a match for the key.
	
	11/27/90 dmb: the optional parameter field mentioned above is now in the verb 
	record, not in landglobals.
	*/
	
	register hdlverbrecord hv = hverb;
	register boolean fl;
	AppleEvent event, reply;
	
	landsystem7geteventrecords (hv, &event, &reply);
	
	fl = landsystem7getparam (&event, key, type, param);
	
	if (!fl) {
		
		if ((**hv).flnextparamisoptional)
			landseterror (noErr);
		else
			landreturnerror (hv, nosuchparamerror);
		}
	
	(**hv).flnextparamisoptional = false; /*must be reset every time*/
	
	return (fl);
	} /*landgetparam*/


pascal boolean landgetnthparam (hdlverbrecord hverb, short n, typaramrecord *param) {
	
	/*
	return the nth parameter in the verb's parameter list.  we return all the info
	we have about the parameter in the paramrecord.
	
	n is 1-based.
	
	we don't return an error to the caller if the parameter isn't there -- this allows
	the caller to sniff around the parameter list without making any assertions.
	*/
	
	register hdlverbrecord hv = hverb;
	AppleEvent event, reply;
	
	if (n > (**hv).ctparams) /*there aren't that many parameters*/
		return (false);
	
	landsystem7geteventrecords (hv, &event, &reply);
	
	return (landsystem7getnthparam (&event, n, param));
	} /*landgetnthparam*/


pascal boolean landgetintparam (hdlverbrecord hverb, typaramkeyword key, short *x) {
	
	typaramrecord param;
	
	if (!landgetparam (hverb, key, inttype, &param)) 
		return (false);
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{		
		Handle h;
		
		copydatahandle (&(param.desc), &h);
		
		*x = **(short **) h;
		
		disposehandle (h);
		}
	
	#else
		
		*x = **(short **) param.desc.dataHandle;
	
	#endif
	
	landdisposeparamrecord (&param);
	
	return (true);
	} /*landgetintparam*/
	

pascal boolean landgetlongparam (hdlverbrecord hverb, typaramkeyword key, long *x) {
	
	typaramrecord param;
	
	if (!landgetparam (hverb, key, longtype, &param)) 
		return (false);

	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{		
		Handle h;
		
		copydatahandle (&(param.desc), &h);
		
		*x = **(long **) h;
		}
	
	#else
	
		*x = **(long **) param.desc.dataHandle;
	
	#endif
	
	landdisposeparamrecord (&param);
	
	return (true);
	} /*landgetlongparam*/
	

pascal boolean landgetstringparam (hdlverbrecord hverb, typaramkeyword key, bigstring x) {
	
	/*
	7/20/92 dmb: always use texttype
	*/
	
	typaramrecord param;
	
	if (!landgetparam (hverb, key, texttype, &param)) 
		return (false);
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		 datahandletostring (&(param.desc), x);
	
	#else
	
		texthandletostring (param.desc.dataHandle, x);
	
	#endif
	
	landdisposeparamrecord (&param);
	
	return (true);
	} /*landgetstringparam*/


pascal boolean landgettextparam (hdlverbrecord hverb, typaramkeyword key, Handle *x) {
	
	typaramrecord param;
	
	if (!landgetparam (hverb, key, texttype, &param)) 
		return (false);

	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		 copydatahandle (&(param.desc), x);
	
	#else
		
		*x = param.desc.dataHandle;
	
	#endif

	return (true);
	} /*landgettextparam*/



