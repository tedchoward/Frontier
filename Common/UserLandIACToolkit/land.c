
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

#include "ops.h"
#include "process.h"

	#include "aeutils.h" /*PBS 03/14/02: AE OS X fix.*/


/*
4/28/92 dmb: use this static instead of asm global stuff
*/
	
	static hdllandglobals hlandglobals = nil;
	
	#define landsetlandglobals(x)	(hlandglobals = x)
	#define landgetlandglobals()	(hlandglobals)

#pragma pack(2)
typedef struct tytransportinfo { 
	
	hdlprocessthread hthread; /*must be first field*/
	
	} tytransportinfo, *ptrtransportinfo, **hdltransportinfo;
#pragma options align=reset


static THz landzone;


static short landresfile;



pascal boolean landpushfastcontext (tyfastverbcontext *savecontext) {
	
	register tyfastverbcontext *x = savecontext;
	//Code change by Timothy Paustian Monday, June 26, 2000 3:19:32 PM
	//
	(*x).savezone = LMGetApplZone();
		
	(*x).saveresfile = CurResFile ();
	
	//I wonder if these calls are needed. Can we get away with
	//UseResFile and CurResFile

	LMSetApplZone(landzone);

	UseResFile (landresfile);
	
	return (true);
	} /*landpushfastcontext*/


pascal void landpopfastcontext (const tyfastverbcontext *savecontext) {
	
	register const tyfastverbcontext *x = savecontext;
	

	UseResFile ((*x).saveresfile);
	LMSetApplZone((*x).savezone);
	} /*landpopfastcontext*/

	
pascal hdllandglobals landgetglobals (void) {
		
	return (landgetlandglobals ()); /*return value of asm routine*/
	} /*landgetglobals*/


pascal boolean landdisposeparamrecord (const typaramrecord *p) {
	
	/*
	dispose of any storage linked into the parameter record.  
	
	assume all non-scalars are simple handles, with nothing linked to it.  if
	we add more complex types, this routine will get more complex.

	docnote: param now passed by reference for Pascal compatability.
	*/
	
	
		AEDisposeDesc ((AEDesc*) &(*p).desc);
		
	
	return (true);
	} /*landdisposeparamrecord*/
	

pascal boolean landdisposeverb (hdlverbrecord hverb) {
	
	/*
	dispose of all storage owned by the indicated verb record, and the handle
	that stores the record.
	
	we assume that the transportinfo record has no handles linked to it, if 
	it does, call your transport mechanism here to dispose of the fields of 
	the transportinfo handle.
	*/
	
	register hdlverbrecord hv = hverb;
	
	if (hv == nil)
		return (false);
	
	/*
	landdisposeallparams (hv);
	*/
	
	landsystem7disposeverb (hv); /*dispose linked handles in transport info*/
	
	landdisposehandle ((**hv).transportinfo); /*assume no linked handles*/
	
	landdisposehandle ((Handle) hv);
	
	return (true);
	} /*landdisposeverb*/


static boolean landsetupnewverb (tyapplicationid idreceiver, tyverbclass class, tyverbtoken token, short ctparams, hdlverbrecord *hverb) {
	
	/*
	allocate a new verb record with room for the indicated number of parameters.
	
	the caller will then push parameters, and finally send the verb.
	
	if you don't know up-front how many parameters you will push, just send in 0
	and we'll enlarge the handle every time you push a parameter.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlverbrecord hv;
	
	if (!(**hg).flconnected)
		return (false);
	
	if (!landnewclearhandle (longsizeof (tyverbrecord), (Handle *) hverb)) /*allocation failure*/
		return (false);
	
	hv = *hverb; /*copy into register*/
	
	(**hv).idsender = (**hg).applicationid;
	
	(**hv).idreceiver = idreceiver;
	
	(**hv).idtransaction = (**hg).eventsettings.transactionid;
	
	(**hv).verbclass = class;
	
	(**hv).verbtoken = token;
	
	(**hv).ctparams = ctparams;
	
	return (true);
	} /*landsetupnewverb*/


pascal boolean landnewverb (tyapplicationid idreceiver, ProcessSerialNumber *psn, tyverbclass class, tyverbtoken token, short ctparams, hdlverbrecord *hverb) {
	
	/*
	allocate a new verb record with room for the indicated number of parameters.
	
	the caller will then push parameters, and finally send the verb.
	
	if you don't know up-front how many parameters you will push, just send in 0
	and we'll enlarge the handle every time you push a parameter.
	
	2.1a7 dmb: take psn param
	*/
	
	register hdlverbrecord hv;
	register boolean fl;
	
	if (!landsetupnewverb (idreceiver, class, token, ctparams, hverb))
		return (false);
	
	hv = *hverb; /*copy into register*/
	
	fl = landsystem7newverb (hv, psn);
	
	/*
	if ((**hg).transport == macsystem7) 
		fl = landsystem7newverb (hv);
	else
		fl = landsystem6newverb (hv);
	*/
	
	if (!fl) {
		
		landdisposeverb (hv);
		
		*hverb = nil;
		
		return (false);
		}
	
	return (true);
	} /*landnewverb*/


pascal boolean landnewnetworkverb (tynetworkaddress *adr, tyverbclass class, tyverbtoken token, short ctparams, hdlverbrecord *hverb) {

	/*
	7/15/91 DW: return false on system 6.
	*/
	
	register hdlverbrecord hv;
	
	if (!landsetupnewverb (idnet, class, token, ctparams, hverb))
		return (false);
	
	hv = *hverb; /*copy into register*/
	
	if (!landsystem7newnetworkverb (hv, adr)) {
		
		landdisposeverb (hv);
		
		*hverb = nil;
		
		return (false);
		}
	
	return (true);
	} /*landnewnetworkverb*/
	

pascal boolean landcomplexsend (hdlverbrecord hverb, hdlverbrecord *hresult) {
	
	/*
	the low-level send routine.  call this directly if you want to look 
	at multiple return values.  otherwise, use landsendverb, below
	*/
	
	register hdlverbrecord hv = hverb;
	register hdllandglobals hg = landgetglobals ();
	register boolean fl;
	
	if (!(**hg).flconnected)
		return (false);
	
	(**hg).landerrorcode = noErr;
	
	fl = landsystem7send (hv, hresult);
	
	/*
	if ((**hg).transport == macsystem7) 
		fl = landsystem7send (hv, hresult);
	else
		fl = landsystem6send (hv, hresult);
	*/
	
	landdisposeverb (hv);
	
	return (fl);
	} /*landcomplexsend*/


pascal boolean landsendverb (hdlverbrecord hverb, typaramrecord *result, bigstring bserror, short *iderror) {
	
	/*
	the simpler way to send a verb record.  we assume there is a single returned
	value, if there are any others, we throw them away.  if you can use more than
	one returned value, call the more complex verb sender -- landcomplexsend.
	
	result holds the returned value, unless the return is an error, in which case
	we copy the error string and the error id (a 16-bit signed number) into the
	returned bserror and iderror.
	
	we return true if there's a valid value in *result.  the client is responsible
	for disposing of any handles in the result.
	
	if we return false with iderror == 0 there has been a communications breakdown
	of some kind.  the client should stop trying to process the verb and get back
	to its main event loop ASAP.
	
	if there was an error, we look for an error string as well.  if none is found, 
	bserror will be the empty string
	
	8/17/92 dmb: fixed handling of zero error numbers
	
	1/25/93 dmb: make sure that an 'errn' that can't be coerced to a short is still 
	recognized as an error condition
	
	3.0.1b1 dmb: #if'd out the special handling of "land" errors, 1-6. archaic, and 
	in the way.
	*/
	
	hdlverbrecord hresult;
	register boolean fl = false;
	typaramrecord errorparam;
	
	/**
	landsetstatsinfo (hverb, false);
	*/
	
	*iderror = 0; /*default --no error*/
	
	setemptystring (bserror); /* no error*/
	
	landclearbytes (result, longsizeof (*result));
	
	if (!landcomplexsend (hverb, &hresult))
		return (false);
	
	landnextparamisoptional (hresult); /*don't require that any reply values exist*/
	
	if (landgetparam (hresult, errornumberkey, notype, &errorparam)) { /*got an error*/
		
		if (!landgetintparam (hresult, errornumberkey, iderror)) { /*couldn't coerce to short*/
			
			landseterror (noErr); /*clear coercion error*/
			
			*iderror = errAEEventFailed;
			}
		
		landnextparamisoptional (hresult); /*don't require next parameter...*/
		
		if (*iderror == 0) { /*'errn' was zero, not an error; check for direct param*/
			
			fl = true; /*not an error*/
			
			landgetparam (hresult, returnedvaluekey, notype, result); /*take it if it's there*/
			}
		else {
			
			if (!landgetstringparam (hresult, errorstringkey, bserror)) { /*no string*/
				
				#if 0
				
				register hdllandglobals hg = landgetglobals ();
				short ixstring = *iderror;
				
				if ((ixstring > 0) && (ixstring <= usercancellederror)) /*one of our errors*/
					GetIndString (bserror, (**hg).iderrorlist, ixstring);
				
				#endif
				}
			}
		}
	else {
		
		fl =
			(**hresult).ctparams == 0 || /*OK not to have anything returned*/
			
			landgetparam (hresult, returnedvaluekey, notype, result) || /*take return key first*/
			
			landgetnthparam (hresult, 1, result); /*got a returned value of some type*/
		}
	
	landdisposeverb (hresult);
	
	return (fl);
	} /*landsendverb*/


boolean landhandleverb (hdlverbrecord hverb) {
	
	/*
	a jacket for handling verbs -- we catch all those that are implemented by the
	API, never calling the using program's handler -- we just return the value that's
	being asked for.

	6/27/91 DW: stats window supported two ways -- we display the stats info on
	receipt of the verb, and reset the memory message on the idrunning message.
	
	7/6/91 DW: if a script has been cancelled, and we receive a message from
	the scripting system we send back usercancellederror. 
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlverbrecord hv = hverb;
	
	register boolean fl;
	register landverbrecordcallback cb = (**hg).handleverbroutine;
	
	fl = (*cb) (hv);
	
	landdisposeverb (hv);
	
	return (fl);
	} /*landhandleverb*/


pascal boolean landsettimeout (long ticks) {
	
	register hdllandglobals hg = landgetglobals ();
	
	(**hg).eventsettings.timeoutticks = ticks;
	
	return (true);
	} /*landsettimeout*/


pascal boolean landsettransactionid (long id) {
	
	register hdllandglobals hg = landgetglobals ();
	
	(**hg).eventsettings.transactionid = id;
	
	return (true);
	} /*landsettransactionid*/


pascal boolean landsetinteractionlevel (short level) {
	
	register hdllandglobals hg = landgetglobals ();
	
	switch (level) {
		
		case -1:
			level = 0; break;
		
		case 0:
			level = kAENeverInteract; break;
		
		default:
			level = kAEAlwaysInteract + kAECanSwitchLayer;
		}
	
	(**hg).eventsettings.interactionlevel = level;
	
	return (true);
	} /*landsetinteractionlevel*/


pascal boolean landeventfilter (EventRecord *ev) {
	
	/*
	insert a call to this routine in your main event loop to check for incoming
	verbs.  if one is available, we will call the landglobals callback routine
	for message handling.
	
	we filter all events.  the system 7 transport watches for high level 
	events, while the system 6 transport polls during null events.
	
	return true if the event record is consumed, false otherwise.
	
	10/30/90 DW: add call to landpoll so that using program doesn't have to install
	two wires to us in the main event loop.
	
	11/6/90 dmb: polling now happens in landsystem6eventfilter.
	
	6/26/91 DW: add filter for events sent to the stats window.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	
	/*
	if (landwindoweventfilter (ev)) /%event consumed by stats window%/
		return (true);
	*/
	
	if (!(**hg).flconnected)
		return (false);
		
	return (landsystem7eventfilter (ev));
	
	/*
	if ((**hg).transport == macsystem7)
		return (landsystem7eventfilter (ev));
	else 
		return (landsystem6eventfilter (ev));
	*/
	} /*landeventfilter*/



pascal boolean landclose (void) {
		
	register hdllandglobals hg = landgetglobals ();
	
	landsystem7close ();
	
	landdisposehandle ((Handle) (**hg).verbarray);
	
	return (true);
	
	/*
	if (!(**hg).flconnected)
		return (false);
	*/
	
	/*
	if ((**hg).transport == macsystem7) 
		return (landsystem7close ());
	else
		return (landsystem6close ());
	*/
	
	landdisposehandle ((Handle) hg);
	} /*landclose*/
	
	
pascal boolean landefaultbreakembrace (EventRecord *ev) {
#pragma unused (ev)

	/*
	return true if the user is holding down the cmd, option and shift keys.
	*/
	
	#pragma unused (ev)
	
	register Ptr p;
	KeyMap keys;
	
	GetKeys (keys);

	p = (Ptr) keys; 

	return (BitTst (p, 63) && BitTst (p, 48) && BitTst (p, 61));
	} /*landefaultbreakembrace*/


pascal void landseterror (OSErr errorcode) {
	
	/*
	set the global error code for later retrieval via landgeterror
	*/
	
	register hdllandglobals hg = landgetglobals ();
	
	(**hg).landerrorcode = errorcode;
	} /*landseterror*/


pascal OSErr landgeterror (void) {
	
	/*
	return the OSError that cause the last land call to return false.
	
	the value returned by this routine has no meaning if the last call 
	did not return false
	*/
	
	register hdllandglobals hg = landgetglobals ();
	
	return ((**hg).landerrorcode);
	} /*landgeterror*/


pascal boolean landnextparamisoptional (hdlverbrecord hverb) {
	
	(**hverb).flnextparamisoptional = true;
	
	return (true);
	} /*landnextparamisoptional*/


pascal boolean landverbgetsnoreply (hdlverbrecord hverb) {
	
	(**hverb).flverbgetsnoreply = true;
	
	return (true);
	} /*landverbgetsnoreply*/


pascal boolean landacceptanyverb (boolean flacceptanyverb) {

	register hdllandglobals hg = landgetglobals ();
	
	(**hg).flacceptanyverb = flacceptanyverb;
	
	if ((**hg).transport == macsystem7) /*dmb 10/8/91*/
		landsystem7acceptanyverb ();
	
	return (true);
	} /*landacceptanyverb*/


pascal boolean landsetapplicationid (tyapplicationid tynewid) {
	
	return (false);
	
	/*
	register hdllandglobals hg = landgetglobals ();
	
	if ((**hg).transport == macsystem7) /%DW 6/25/91%/
		return (false);
	
	return (landsystem6setapplicationid (tynewid));
	*/
	
	} /*landsetapplicationid*/


static landqueuepopcallback threadvisit;

static pascal boolean sleepingthreadvisit (Handle htinfo, long refcon) {
	
	hdltransportinfo ht = (hdltransportinfo) htinfo;
	
	if ((*threadvisit) ((Handle) (**ht).hthread, refcon)) {
		
		processwake ((**ht).hthread);
		
		return (true);
		}
	
	return (false);
	} /*sleepingthreadvisit*/


pascal boolean landvisitsleepingthreads (landqueuepopcallback visitroutine, long refcon) {
	
	/*
	return true (andk stop visiting) if the visit routine ever returns true.
	
	note that when the visit routine does return true, the thread is awakended 
	and the item is popped from the queue
	*/
	
	hdltransportinfo ht;
	
	threadvisit = visitroutine; /*make visible to our visit routine*/
	
	return (landpopqueueitem (&sleepingthreadvisit, refcon, (Handle *) &ht));
	} /*landvisitsleepingthreads*/


pascal boolean landinit (void) {
	
	/*
	start up UserLand IAC Toolkit.  must be called before the application opens
	any files -- so we can call landgetappcreator to get the application id.
	
	determine what transport to use, and initialize landglobals fields
	
	add the two default verbs to the verb list
	
	3.0.1b1 dmb: don't initialize iderrorlist to 129; if the client doesn't 
	set it, it's not valid.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	/*
	SysEnvRec world;
	*/
	long result;
	hdllandglobals hglobals;
	tyapplicationid id;
	
	landzone = LMGetApplZone();
		

	landresfile = CurResFile ();
	
	if (hg == nil) { /*if someone hasn't already put something there*/
		
		if (!landnewclearhandle (longsizeof (tylandglobals), (Handle *) &hglobals))
			return (false);
			
		landsetlandglobals (hglobals);	/*stick the handle in*/
		
		hg = hglobals;
		}
	
	(**hg).transport = macsystem7;
	
	(**hg).applicationid = '    ';
	
	/*
	(**hg).iderrorlist = 129;
	
	(**hg).flconnected = false;
	
	(**hg).flacceptanyverb = false;
	
	(**hg).verbarray = nil;
	
	(**hg).handleverbroutine = nil;
	
	(**hg).breakembraceroutine = nil;
	
	(**hg).macopenapproutine = nil;
	
	(**hg).macopendocroutine = nil;
	
	(**hg).macprintdocroutine = nil;
	
	(**hg).macquitapproutine = nil;
	
	(**hg).mactypes = nil;
	
	(**hg).hqueue = nil;
	
	(**hg).ipcrefnum = 0;
	*/
	
	(**hg).breakembraceroutine = &landefaultbreakembrace;
	
	if (!landgetappcreator (&id))
		return (false);
	
	(**hg).applicationid = id;
	
	if (!gestalt (gestaltAppleEventsAttr, &result) || (result == 0))
		return (false);
	
	if (!landsystem7init ())
		return (false);
	
	landaddclass (userlandclass); /*add built-in, API-supported verbs*/
	
	landaddverb (idrunning);
	
	landaddclass (id); /*initial verbs are in the app's creator id class*/
	
	(**hg).flconnected = true;
	
	//landsetmemstats (); /*copy freemem into globals -- for use in stats window*/
	
	return (true);
	} /*landinit*/


