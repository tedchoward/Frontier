
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

#include <iac.h>
#include <UserTalk.h>
#include <appletdefs.h>
#include <appletmemory.h>
#include <appletprocess.h>
#include <appletstrings.h>
#include <appletfiledesktop.h>
#include "iowascript.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h" /*PBS 03/14/02: AE OS X fix.*/
#endif

#define idnetscapeurl 'NURL'
#define idnetscapejavascript 'NJVS'

#define iowaWeb /*undefine this to eliminate minor hacks*/

#define noAppleScript /*if we include it we crash on exiting*/

#ifdef iowaWeb
	
	#define idnetscape 'MOSS'
	
#endif
 
tylangcomponentrecord **langcomponentarray = nil; /*dynamic array of components*/



#ifdef isFrontier

void initlangcomponents (void) {
	} /*initlangcomponents*/

void closelangcomponents (void) {
	} /*closelangcomponents*/

ComponentInstance findlangcomponent (long idlanguage) {
	
	extern ComponentInstance getosaserver (OSType type);

	return (getosaserver (idlanguage));
	} /*findlangcomponent*/

#else


	
short countlangcomponents (void) {
	
	if (langcomponentarray == nil)
		return (0);
		
	return ((short) (GetHandleSize ((Handle) langcomponentarray) / longsizeof (tylangcomponentrecord)));
	} /*countlangcomponents*/
	
	
static boolean addlangcomponent (ComponentInstance instance, long idlanguage, Handle hname, boolean flrealcomponent) {

	tylangcomponentrecord rec;
	short ct;
	
	rec.instance = instance;
	
	rec.idlanguage = idlanguage;
	
	rec.hname = hname;
	
	rec.flrealcomponent = flrealcomponent;
	
	if (langcomponentarray == nil)
		return (newfilledhandle (&rec, longsizeof (tylangcomponentrecord), (Handle *) &langcomponentarray));
	
	ct = countlangcomponents ();
	
	SetHandleSize ((Handle) langcomponentarray, (ct + 1) * longsizeof (tylangcomponentrecord));

	if (MemError () != noErr)
		return (false);
	
	(*langcomponentarray) [ct] = rec;
	
	return (true);
	} /*addlangcomponent*/
	
	
static boolean addwebcomponent (long idlanguage, bigstring bsname) {
	
	Handle hname;
	
	if (!newtexthandle (bsname, &hname))
		return (false);
	
	return (addlangcomponent (0, idlanguage, hname, false));
	} /*addwebcomponent*/
	
	
static void getcomponentdetails (Component comp, ComponentDescription *details, Handle *hname) {
	
	Handle h;
	
	newclearhandle (0, &h);
	
	GetComponentInfo (comp, details, h, nil, nil);
	
	/*mystery: why is there one garbage char at beginning of name?*/ {
		
		if (h != nil) {
		
			long newsize = GetHandleSize (h) - 1;
		
			moveleft (*h + 1, *h, newsize);
		
			SetHandleSize (h, newsize);
			}
		}
	
	*hname = h;
	} /*getcomponentdetails*/
	
	
void initlangcomponents (void) {
	
	static boolean componentsinitialized = false;
	ComponentDescription desc, details;
	Component comp;
	Component prevcomponent;
	ComponentInstance instance;
	long i, ct;
	Handle hname;
	
	if (componentsinitialized) /*DW 12/5/93 -- with Iowa Runtime inside CE, we are called twice*/
		return;
		
	componentsinitialized = true;
	
	clearbytes (&desc, longsizeof (desc));
	
	desc.componentType = kOSAComponentType;
	
	#ifdef noAppleScript
	
		desc.componentSubType = 'LAND'; /*DW 1/20/95 -- only connect with Frontier, AS crashes when we close*/

	#endif
	
	ct = CountComponents (&desc);
	
	prevcomponent = nil; /*start with the first one*/
	
	for (i = 0; i < ct; i++) {
		
		comp = FindNextComponent (prevcomponent, &desc);
		
		instance = OpenComponent (comp);
		
		getcomponentdetails (comp, &details, &hname);
		
		if (details.componentSubType != kOSAGenericScriptingComponentSubtype)
			addlangcomponent (instance, details.componentSubType, hname, true);
		else
			disposehandle (hname);
		
		prevcomponent = comp;
		} /*for*/
		
	#ifdef iowaWeb /*DW 12/27/94*/
		
		addwebcomponent (0, "\p(-"); /*oh this is really cute*/
		
		addwebcomponent (idnetscapeurl, "\pNetscape URL");
		
		addwebcomponent (idnetscapejavascript, "\pNetscape JavaScript");
		
	#endif
	} /*initlangcomponents*/
	
	
ComponentInstance findlangcomponent (long idlanguage) {
	
	/*
	DW 1/21/95 -- soften it. before returning false, we try to
	find the component using the Component Manager.
	
	dmb 7/31/96: make sure instance isn't stale by trying to get its version
	*/
	
	short ct, i;
	tylangcomponentrecord rec;
	
	ct = countlangcomponents ();
	
	for (i = 0; i < ct; i++) {
		
		rec = (*langcomponentarray) [i];
		
		if (rec.idlanguage == idlanguage) {
		
			if (GetComponentVersion (rec.instance) == badComponentInstance) { /*dmb 1.0b19: no longer valid*/
			
				rec.instance = 0;
				
				rec.idlanguage = 0;
				
				disposehandle (rec.hname);
				
				rec.hname = nil;
				
				(*langcomponentarray) [i] = rec;
				}
			else
				return (rec.instance);
			}
		} /*for*/
		
	/*DW 1/21/95 -- not in the array -- see if it's in memory*/ {
	
		ComponentDescription desc, details;
		long ct;
		
		/*DebugStr ("\pnot in the array -- see if it's in memory");*/
		
		clearbytes (&desc, longsizeof (desc));
	
		desc.componentType = kOSAComponentType;
	
		desc.componentSubType = idlanguage;
	
		ct = CountComponents (&desc);
		
		if (ct > 0) {
		
			Component comp;
			ComponentInstance instance;
			Handle hname;
			
			comp = FindNextComponent (nil, &desc);
		
			instance = OpenComponent (comp);
		
			getcomponentdetails (comp, &details, &hname);
		
			addlangcomponent (instance, details.componentSubType, hname, true);
			
			return (instance);
			}
		}
	
	return (0); /*not found*/
	} /*findlangcomponent*/
	

boolean getlangmenu (MenuHandle *hlangmenu) {
	
	/*
	return a menu filled with the names of all the installed scripting
	components.
	*/
	
	MenuHandle hmenu;
	short ct, i;
	bigstring bs;
	
	hmenu = NewMenu (25000, "\px");
	
	ct = countlangcomponents ();
	
	for (i = 0; i < ct; i++) {
		
		texthandletostring ((*langcomponentarray) [i].hname, bs);
		
		if (equalstrings (bs, "\pUserTalk")) /*DW 12/30/95*/
			copystring ("\pFrontier Script", bs);
	
		AppendMenu (hmenu, bs);
		} /*for*/
		
	*hlangmenu = hmenu;
	
	return (true);
	} /*getlangmenu*/
	
	
void getlanguageid (short ixlanguage, OSType *idlanguage) {
	
	*idlanguage = (*langcomponentarray) [ixlanguage].idlanguage;
	} /*getlanguageid*/
	
	
void closelangcomponents (void) {
	
	short ct, i;
	
	ct = countlangcomponents ();
	
	for (i = 0; i < ct; i++) {
		
		CloseComponent ((*langcomponentarray) [i].instance);
		
		disposehandle ((*langcomponentarray) [i].hname);
		} /*for*/
		
	if (langcomponentarray != nil)
		disposehandle ((Handle) langcomponentarray);
	
	langcomponentarray = nil;
	} /*closelangcomponents*/

#endif	// isFrontier

#ifdef macBirdPlugin

	extern boolean runworldwideurl (Handle hscript, bigstring errorstring);

#else

	static boolean runworldwideurl (Handle hscript, bigstring errorstring) {
		
		AppleEvent event, reply;
		boolean fl;
		
		setstringlength (errorstring, 0);
		
		/*launch the netscape app, if necessary*/ {
			
			if (!assureappisrunning (idnetscape, true)) {
				
				copystring ("\pCouldn't follow the worldwide web link because Netscape isn't running", errorstring);
				
				return (false);
				}
			}
			
		if (!IACnewverb (idnetscape, 'GURL', 'GURL', &event))
			return (false);
		
		IACglobals.event = &event;
		
		if (!IACpushtextparam (hscript, '----'))
			return (false);
			
		if (!IACpushlongparam (0, 'cwin'))
			return (false);
			
		if (!IACsendverb (&event, &reply))
			return (false);
		
		IACglobals.reply = &reply;
		
		fl = !IACiserrorreply (errorstring);
		
		IACdisposeverb (&reply);
		
		return (fl);
		} /*runworldwideurl*/

#endif

static boolean rungraphicalurl (Handle hscript, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*rungraphicalurl*/
	
	
boolean runlangscript (Handle hscript, Handle tablename, long idlanguage, boolean flgetreturn, bigstring errorstring, Handle *hreturns) {
	
	/*
	execute the script in the card's context. consume hscript.
	
	dmb 1.0b23 - add a "with" statement to establish our context. we
	no longer rely on frontSetRuntimeCard being called.
	*/
	
	ComponentInstance scriptcomp;
	AEDesc scriptdesc;
	AEDesc resultdesc;
	OSErr ec = noErr;
	bigstring bswith;
	
	if (flgetreturn)
		*hreturns = nil;
	
	#ifdef iowaWeb
	
		if (idlanguage == idnetscapeurl) 
			return (runworldwideurl (hscript, errorstring));
			
		if (idlanguage == idnetscapejavascript) 
			return (rungraphicalurl (hscript, errorstring));
	
	#endif
	
	if (idlanguage == 'LAND') {
		
		if (tablename != nil) { /*1.0b23 dmb*/
		
			texthandletostring (tablename, bswith);
			
			insertstring ("\pwith system.compiler.cards.[\"", bswith);
			
			pushstring ("\p\"] {", bswith);
			
			if (!pushathandlestart (bswith + 1, stringlength (bswith), hscript)) {
				
				disposehandle (hscript);
				
				return (false);
				}
			
			copystring ("\p}", bswith);
			
			pushtexthandle (bswith, hscript);
			}
		
		#ifdef isFrontier
		
		/*bypass the component manager; run script directly*/ {
		
			extern boolean langrunhandletraperror (Handle, bigstring, bigstring);
			bigstring bsresult;
			
			if (!langrunhandletraperror (hscript, bsresult, errorstring))
				return (false);	/***need to trap error info*/
			
			if (flgetreturn)
				return (newtexthandle (bsresult, hreturns));
			
			return (true);
			}
			
		#endif
		}
	
	setstringlength (errorstring, 0);	
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		newdescwithhandle (&scriptdesc, typeChar, hscript);
	
	#else
		
		scriptdesc.descriptorType = typeChar;

		scriptdesc.dataHandle = hscript;
	
	#endif
	
	scriptcomp = findlangcomponent (idlanguage);
	
	if (scriptcomp == 0) { /*didn't find the component*/
		
		bigstring bs;
		
		AEDisposeDesc (&scriptdesc);
	
		copystring ("\pCouldn't find the '", errorstring);
		
		setstringlength (bs, 4);
		
		moveleft (&idlanguage, &bs [1], 4);
		
		pushstring (bs, errorstring);
		
		pushstring ("\p' scripting component.", errorstring);
		
		return (false);
		}
	
	ec = OSADoScript (scriptcomp, &scriptdesc, kOSANullScript, typeChar, kOSAModeTransparentScope, &resultdesc);
	
	if (ec != noErr)
		goto error;
	
	AEDisposeDesc (&scriptdesc);
	
	if (flgetreturn)
	
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
			copydatahandle (&resultdesc, hreturns);
			
		#else
		
			*hreturns = resultdesc.dataHandle;
		
		#endif
	else
		AEDisposeDesc (&resultdesc);
	
	return (true);
	
	error:
	
	AEDisposeDesc (&scriptdesc);
	
	if (ec == errOSAScriptError) { /*try to get an error message*/ 
		
		AEDesc errordesc;
		
		ec = OSAScriptError (scriptcomp, kOSAErrorMessage, typeChar, &errordesc);
		
		if (ec == noErr) {
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				datahandletostring (&errordesc, errorstring);
			
			#else
			
				texthandletostring (errordesc.dataHandle, errorstring);
			
			#endif
			
			AEDisposeDesc (&errordesc);
			}
		else 
			copystring ("\pThe script generated an error, but no message was provided.", errorstring);
		}
	
	if (ec == -128) /*1.0b15 DW -- user cancelled the script with cmd-period*/
		return (false);
	
	if (stringlength (errorstring) == 0) {
		
		copystring ("\pComponent Manager error. Its code number is ", errorstring);
		
		pushlong (ec, errorstring);
		
		pushstring ("\p.", errorstring);
		}
	
	return (false);
	} /*runlangscript*/

