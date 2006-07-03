
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
#include "shelltypes.h"
#include "iowacore.h"
#include "iowascript.h"
#include "iowaruntime.h"
#include "iowafrontier.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h" /*PBS 03/14/02: AE OS X fix.*/
#endif

/*a little code ghetto for stuff that depends on Frontier or Runtime*/

#ifdef macBirdPlugin
	#ifdef fldebug
		#define alertdialog(s)		DebugStr (s)
	#else
		#define alertdialog(s)		((void *)0)
	#endif
#endif

#ifdef isFrontier
	#define idscriptresource	1024
#else
	#define idscriptresource	128
#endif

#define startcardtoken 			'init'
#define setcardtoken 			'sett'
#define closecardtoken 			'clos'
#define edittabletoken 			'edtb'
#define getttabletoken 			'gett'

#ifdef claybasket
	#define doscripttoken			'dosc' /*DW 9/28/95*/
	#define addtoglossarytoken 		'addg' /*DW 10/9/95*/
	#define editglossarytoken		'edtg' /*DW 10/9/95*/
	#define refglossarytoken		'refg' /*DW 10/9/95*/
	#define editobjecttoken 		'edto' /*DW 10/17/95*/
	#define runuserscripttoken		'russ' /*DW 11/4/95*/
	#define runlinkfiltertoken		'rulf' /*DW 11/4/95*/
	#define copytopagetabletoken 	'ctpt' /*DW 11/5/95*/
	#define copyfrompagetabletoken 	'cfpt' /*DW 11/5/95*/
	#define runpagefiltertoken		'rupf' /*DW 11/5/95*/
	#define runfinalfiltertoken		'ruff' /*DW 12/7/95*/
#endif


/*
static bigstring lasttablename = "\p";

static boolean lasttablewasmajor = false;
*/


#define scriptmodebits kOSANullMode




static void getappspec (FSSpec *fs) {
	
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	Str255 s;
	
	GetCurrentProcess (&psn);
	
	info.processInfoLength = (long) sizeof (info);
	
	info.processName = s; /*place to store process name*/
	
	info.processAppSpec = fs; /*place to store process filespec*/
	
	GetProcessInformation (&psn, &info);
	} /*getappspec*/
	
	
static void componenterrormessage (OSErr ec, bigstring errormessage) {

	copystring ("\pComponent Manager error. Its code number is ", errormessage);
	
	pushlong (ec, errormessage);
	
	pushstring ("\p.", errormessage);
	} /*componenterrormessage*/


static boolean getcompiledscript (OSAID *scriptid, ComponentInstance *scriptcomp, bigstring errormessage) {
	
	/*
	the original version saves as much as possible from call to 
	call, using statics. not safe to multi-process card running.
	
	dmb 1.0b23: when we're running in Frontier, this can be called in any of our 
	client contexts. findlangcomponent already handle this, but we 
	need to make sure the we don't try to share the same script ID 
	between the component instances we have open in each client 
	process. We could maintain some kind of list, but now that we 
	make this call less often (we only SetRuntimeCard for major
	switches), it's fine to simple recompile the script each time. so 
	we don't use statics except for the script source resource.
	*/
	
	static Handle hcompiledscript = nil;
	#ifdef isFrontier
		boolean flinitialized = false;
		ComponentInstance comp = 0;
		OSAID id = 0;
	#else
		static boolean flinitialized = false;
		static ComponentInstance comp = 0;
		static OSAID id = 0;
	#endif
	
	if (flinitialized) { /*dmb 6.19.96: make sure the component is still around*/
		
		if (GetComponentVersion (comp) == badComponentInstance) { /*no longer valid*/
			
			comp = 0;
			
			id = 0;
			
			flinitialized = false;
			}
		}
	
	if (!flinitialized) { /*first time we're called, load the script from resource fork*/
		
		if (hcompiledscript == nil) {
		
			hcompiledscript = GetResource ('scpt', idscriptresource);
		
			if (hcompiledscript == nil) {
			
				copystring ("\pThere's no 'scpt' 128 resource available", errormessage);
			
				return (false);
				}
			}
		
		if (comp == 0) {
		
			comp = findlangcomponent (idfrontier);
		
			if (comp == 0)  {
			
				copystring ("\pFrontier 3.0 or greater isn't running.", errormessage);
			
				return (false);
				}
			}
		
		if (id == 0) {
		
			AEDesc scriptdesc;
			OSErr ec;
			//Code change by Timothy Paustian Wednesday, August 9, 2000 10:20:26 AM
			//I think I found the problem YAHOO!
			//in carbon we cannot do this because it is an opaque data structure.
			#if TARGET_API_MAC_CARBON == 1
			SInt32	hSize = GetHandleSize(hcompiledscript);
			HLock(hcompiledscript);
			ec = AECreateDesc('scpt', *hcompiledscript, hSize, &scriptdesc);
			HUnlock(hcompiledscript);
			#else
			scriptdesc.descriptorType = /*typeOSAGenericStorage*/ 'scpt';
			scriptdesc.dataHandle = hcompiledscript; 
			#endif
			ec = OSALoad (comp, &scriptdesc, kOSANullMode, &id);
	
			if (ec != noErr) {
			
				componenterrormessage (ec, errormessage);
			
				return (false);
				}
			}
		
		flinitialized = true;
		}
	
	*scriptcomp = comp;
	
	*scriptid = id;
	
	return (true);
	} /*getcompiledscript*/


static boolean callcompiledscript (AppleEvent *message, OSType resulttype, bigstring errormessage, Handle *hreturned) {
	
	/*
	dmb 1.0b23: must delete scriptid when running in Frontier; it can't be 
	shared from call to call.
	*/
	
	ComponentInstance scriptcomp;
	OSAID scriptid, resultid;
	AEDesc resultdesc = {typeNull, nil};
	OSErr ec;
	
	setstringlength (errormessage, 0);
	
	*hreturned = nil;
	
	if (!getcompiledscript (&scriptid, &scriptcomp, errormessage))
		return (false);
	
	ec = OSAExecuteEvent (scriptcomp, message, scriptid, scriptmodebits, &resultid);
	
#ifdef isFrontier
	OSADispose (scriptcomp, scriptid); /*1.0b23*/
#endif

	if (ec == noErr) {
		
		ec = OSACoerceToDesc (scriptcomp, resultid, resulttype, kOSANullMode, &resultdesc);
		
		OSADispose (scriptcomp, resultid);
		}
	
	/*DW 11/17/95 -- site of a former memory leak*/ {
		
		AEDisposeDesc (message);	
		}
	
	switch (ec) {
		
		case noErr: 
		
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				copydatahandle (&resultdesc, hreturned);

			#else
			
				copyhandle (resultdesc.dataHandle, hreturned); /*script ran without error*/
			
			#endif
			
			break;
		
		case errOSAScriptError: 
			if (OSAScriptError (scriptcomp, kOSAErrorMessage, typeChar, &resultdesc) == noErr)
			
				#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
				
					datahandletostring (&resultdesc, errormessage);
				
				#else

					texthandletostring (resultdesc.dataHandle, errormessage);
				
				#endif
			else
				copystring ("\pThe script generated an error, but no message was provided.", errormessage);
			
			break;
		
		default: 
			componenterrormessage (ec, errormessage);
			
			break;
		} /*switch*/
	
	AEDisposeDesc (&resultdesc);
	
	return (*hreturned != nil);
	} /*callcompiledscript*/
	
	
static boolean newevent (OSType token, AppleEvent *event) {

	AEAddressDesc selfAddress; 
	ProcessSerialNumber psn;
	OSErr ec;

	psn.highLongOfPSN = 0;

	psn.lowLongOfPSN = kCurrentProcess;

	ec = AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, sizeof (psn), &selfAddress);
	
	if (ec != noErr)
		return (false);
	
	ec = AECreateAppleEvent ('abcd', token, &selfAddress, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	AEDisposeDesc (&selfAddress);
	
	return (ec == noErr);
	} /*newevent*/


boolean findfrontiercomponent (void) {
	
	return (findlangcomponent (idfrontier) != 0);
	} /*findfrontiercomponent*/


boolean frontStartCard (AEDesc *filelist) {
	
	/*
	a card is starting up. we call the script at system.verbs.traps.IRUN.stcd
	to create a table for it, unpack the packed table at that location, and 
	copy in the filespec for the application and any files that were dragged
	onto the app.
	
	if tablename is empty, the script assigns a name and returns it. we return
	it to the caller, who is expected to link it into iowadata.
	
	dmb 1.0b19: manage the embedded table more carefully. don't release the 
	packed data until we successfully transfer ownership to frontier. in that 
	case, dispose of our version and set tablestoredinfrontier to true here.
	
	dmb 1.0b20: use tablestoredinfrontier as flfrontcardstarted flag, and 
	return true if we've already been started. This allows other routines 
	to try starting the card in case Frontier was launched after MacBird. 
	also, don't dispose and nil the embedded table field; we might still 
	need it if Frontier goes away before we have a chance to retrieve it.
	
	dmb 1.0b20: added runmode parameter so Frontier will tolerate errors in
	startup script
	
	dmb 1.0b23: removed runmode parameter. Frontier will no longer call the 
	startCard script. It's too soon. We now call it directly after doing 
	default initialization in cardcheckinit in iowaruntime.c
	*/
	
	bigstring tablename;
	boolean flcopyname;
	FSSpec fsapp;
	AppleEvent event;
	
	if ((**iowadata).tablestoredinfrontier)
		return (true);
	
	getappspec (&fsapp);
		
	/*pull the table name from the tablename handle*/ {
	
		if ((**iowadata).tablename == nil) { /*let the script assign a name to the table*/
			
			setstringlength (tablename, 0);
			
			flcopyname = true;
			}
		else {
			texthandletostring ((**iowadata).tablename, tablename);
			
			flcopyname = false;
			}
		}
	
	if (!newevent (startcardtoken, &event))
		return (false);
		
	IACglobals.event = &event;
	
	if (!IACpushstringparam (tablename, 'prm1'))
		goto error;
	
	IACglobals.dontdisposenextparam = true; /*dmb 1.0b19*/
	
	if (!IACpushbinaryparam ((**iowadata).embeddedtable, 'tabl', 'prm2'))
		goto error;
	
	if (!IACpushfilespecparam (&fsapp, 'prm3'))
		goto error;
	
	if (filelist == nil) {
		
		AEDesc desc;
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			newdescnull (&desc, typeNull);
		
		#else
		
			desc.descriptorType = typeNull;
		
			desc.dataHandle = nil;
		
		#endif
		
		if (AEPutParamDesc (IACglobals.event, 'prm4', &desc) != noErr)
			goto error;
		}
	else
		if (AEPutParamDesc (IACglobals.event, 'prm4', filelist) != noErr)
			goto error;
	
	/*
	if (!IACpushbooleanparam ((**iowadata).runmode, 'runm'))
		goto error;
	*/
	
	/*send the event to the compiled script*/ {
		
		hdlcard savediowadata = iowadata; /*if startCard script calls a cardEditor verb, iowadata will be smashed*/
		bigstring errormessage;
		Handle htablename;
		
		installRuntimeEventHandlers (); /*1.0b20 dmb: init script may call back to card*/
		
		if (!callcompiledscript (&event, typeChar, errormessage, &htablename)) {
			
			iowadata = savediowadata;
			
			alertdialog (errormessage);
			
			return (false);
			}
			
		iowadata = savediowadata;
		
		(**iowadata).tablename = htablename;
		
		/*
		texthandletostring (htablename, lasttablename);
		
		lasttablewasmajor = true;
		*/
		}
	
	(**iowadata).tablestoredinfrontier = true;
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontStartCard*/
	

boolean frontSetRuntimeCard (boolean flswitchingin, boolean flmajorswitch) {
	
	/*
	iowadata contains a card that's running. call the script at
	system.verbs.traps.IRUN.sett to make it the current card, 
	all the items in its table are global. and if it's a major
	switch, it's menubar is installed at system.menubars.DRPb.
	
	1.0b19 dmb: now take flswitchingin parameter so Frontier can 
	remove menus when a window is deactivated
	
	1.0b20 dmb: call frontStartCard, which will now do nothing if
	its already been started, but will start it if Frontier wasn't
	running before.
	
	1.0b21 dmb: must track majorswitch parameter along with table 
	name for optimization where we don't call the script
	
	1.0b22 dmb: removed logic to avoid calling frontier. a card 
	running inside of frontier can change the global table without
	our knowing it. besides, it's a component call now, so it's fast.
	
	1.0b23 dmb: big change: runlangscript now runs all scripts inside 
	a "with" statement, so we no longer need to have Frontier point 
	to our table in system.misc.paths. Do we only need to makek this 
	call for major switches
	*/

	AppleEvent event;
	bigstring tablename;
	
	if (!flmajorswitch) /*dmb 1.0b23*/
		return (true);
	
	if ((**iowadata).notscriptedcard)
		return (true);
	
	if (!findfrontiercomponent () || !frontStartCard (nil)) /*1.0b20 dmb*/
		return (false);
	
	if (flswitchingin)
		texthandletostring ((**iowadata).tablename, tablename);
	else
		tablename [0] = 0;
	
	/*
	if (equalstrings (tablename, lasttablename) && (lasttablewasmajor || !flmajorswitch)) /%minimize AE traffic%/
		return (true);
	
	copystring (tablename, lasttablename);
	
	lasttablewasmajor = flmajorswitch;
	*/
	
	if (!newevent (setcardtoken, &event))
		goto error;
		
	IACglobals.event = &event;
	
	if (!IACpushstringparam (tablename, 'prm1'))
		goto error;
	
	if (!IACpushbooleanparam (flmajorswitch, 'prm2'))
		goto error;
	
	/*send the event to the compiled script*/ {
		
		bigstring errormessage;
		Handle hresult;
		
		if (!callcompiledscript (&event, typeChar, errormessage, &hresult)) {
			
			alertdialog (errormessage);
			
			return (false);
			}
			
		disposehandle (hresult);
		}
		
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontSetRuntimeCard*/
	
	
boolean frontEditTable (bigstring tablewindowtitle) {
	
	AppleEvent event;
	bigstring bs;
	
	if ((**iowadata).notscriptedcard)
		return (true);
		
	if (!frontStartCard (nil)) /*1.0b20 dmb*/
		return (false);
	
	if (!newevent (edittabletoken, &event))
		goto error;
		
	IACglobals.event = &event;
	
	texthandletostring ((**iowadata).tablename, bs);
	
	if (!IACpushstringparam (bs, 'prm1'))
		goto error;
	
	if (!IACpushstringparam (tablewindowtitle, 'prm2'))
		goto error;
	
	/*send the event to the compiled script*/ {
		
		bigstring errormessage;
		Handle hresult;
		
		if (!callcompiledscript (&event, typeChar, errormessage, &hresult)) {
			
			alertdialog (errormessage);
			
			return (false);
			}
			
		disposehandle (hresult);
		}
		
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontEditTable*/
	
	
boolean frontGetTable (void) { 
	
	/*
	send a message to Frontier requesting the table associated with
	the card in iowadata.
	*/
	
	AppleEvent event;
	bigstring bs;
	
	if ((**iowadata).notscriptedcard)
		return (true);
	
	if (!(**iowadata).tablestoredinfrontier)  /*1.0b20 dmb - maybe Frontier wasn't running before*/
		return (true);
	
	if (!newevent (getttabletoken, &event))
		return (false);
		
	IACglobals.event = &event;
	
	texthandletostring ((**iowadata).tablename, bs);
	
	if (!IACpushstringparam (bs, 'prm1'))
		goto error;
		
	/*send the event to the compiled script*/ {
		
		bigstring errormessage;
		Handle hpackedtable;
		
		if (!callcompiledscript (&event, 'tabl', errormessage, &hpackedtable)) {
			
			alertdialog (errormessage);
			
			return (false);
			}
		
		assert (hpackedtable != nil);
		
		disposehandle ((**iowadata).embeddedtable); /*1.0b19 dmb: make sure old version is gone*/
		
		(**iowadata).embeddedtable = hpackedtable;
		}
		
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontGetTable*/
	

boolean frontDeleteCardTable (void) {
	
	/*
	iowadata contains a running card that is being terminated. we call
	system.verbs.traps.IRUN.delt to delete the table for this card.
	*/

	AppleEvent event;
	bigstring bs;
	
	if ((**iowadata).notscriptedcard) /*this card is handled by C code*/
		return (true);
	
	if (!(**iowadata).tablestoredinfrontier)  /*1.0b20 dmb - maybe Frontier wasn't running before*/
		return (true);
	
	if (!newevent (closecardtoken, &event))
		goto error;
		
	IACglobals.event = &event;
	
	texthandletostring ((**iowadata).tablename, bs);
	
	if (!IACpushstringparam (bs, 'prm1'))
		goto error;
	
	/*send the event to the compiled script*/ {
		
		bigstring errormessage;
		Handle hresult;
		
		if (!callcompiledscript (&event, typeChar, errormessage, &hresult)) {
			
			alertdialog (errormessage);
			
			return (false);
			}
			
		disposehandle (hresult);
		}
		
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontDeleteCardTable*/
	
#ifdef claybasket

boolean frontRunMacro (bigstring macro, bigstring errorstring, Handle *hresult) {
	
	AppleEvent event;
	
	if (!newevent (doscripttoken, &event))
		goto error;
		
	IACglobals.event = &event;
	
	if (!IACpushstringparam (macro, 'prm1'))
		goto error;
	
	if (!callcompiledscript (&event, typeChar, errorstring, hresult))
		return (false);
		
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontRunMacro*/
	
	
boolean frontRunUserScript (bigstring macro, bigstring path, bigstring errorstring, Handle *hresult) {
	
	AppleEvent event;
	
	if (!newevent (runuserscripttoken, &event))
		goto error;
		
	IACglobals.event = &event;
	
	if (!IACpushstringparam (macro, 'prm1'))
		goto error;
	
	if (!IACpushstringparam (path, 'prm2'))
		goto error;
	
	if (!callcompiledscript (&event, typeChar, errorstring, hresult))
		return (false);
		
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontRunUserScript*/
	
	
boolean frontRunLinkFilter (Handle hlinkstring, short linktype, bigstring errorstring, Handle *hresult) {
	
	AppleEvent event;
	
	if (!newevent (runlinkfiltertoken, &event))
		return (false);
		
	IACglobals.event = &event;
	
	if (!IACpushtextparam (hlinkstring, 'prm1'))
		goto error;
	
	if (!IACpushshortparam (linktype, 'prm2'))
		goto error;
	
	if (!callcompiledscript (&event, typeChar, errorstring, hresult))
		return (false);
		
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontRunLinkFilter*/
	
	
boolean frontRunPageFilter (bigstring errorstring) {
	
	AppleEvent event;
	Handle hresult;
	
	setstringlength (errorstring, 0);
	
	if (!newevent (runpagefiltertoken, &event))
		return (false);
		
	if (!callcompiledscript (&event, typeChar, errorstring, &hresult))
		goto error;
	
	disposehandle (hresult);
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontRunPageFilter*/
	
	
boolean frontRunFinalFilter (Handle htext, bigstring errorstring, Handle *hresult) {
	
	AppleEvent event;
	
	setstringlength (errorstring, 0);
	
	if (!newevent (runfinalfiltertoken, &event))
		return (false);
		
	IACglobals.event = &event;
	
	if (!IACpushtextparam (htext, 'prm1'))
		goto error;
	
	if (!callcompiledscript (&event, typeChar, errorstring, hresult))
		goto error;
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontRunFinalFilter*/
	
	
boolean frontCopyToPageTable (AERecord *rec) {
	
	AppleEvent event;
	bigstring errorstring;
	Handle hresult;
	
	if (!newevent (copytopagetabletoken, &event))
		goto error;
		
	IACglobals.event = &event;
	
	if (!IACpushrecordparam (rec, 'prm1'))
		goto error;
	
	if (!callcompiledscript (&event, typeChar, errorstring, &hresult))
		return (false);
	
	disposehandle (hresult);
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontCopyToPageTable*/
	
	
boolean frontCopyFromPageTable (AERecord *rec) {

	AppleEvent event;
	bigstring errorstring;
	Handle hresult;
	
	if (!newevent (copyfrompagetabletoken, &event))
		return (false);
		
	if (!callcompiledscript (&event, typeAERecord, errorstring, &hresult))
		goto error;
		
	(*rec).descriptorType = typeAERecord;
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		putdeschandle (rec, hresult);
	
	#else
	
		(*rec).dataHandle = hresult;
		
	#endif
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	return (false);
	} /*frontCopyFromPageTable*/
	
	
boolean frontAddToGlossary (void *name, bigstring value, short type, bigstring errorstring) {
	
	AppleEvent event;
	Handle hresult = nil;
	
	if (!newevent (addtoglossarytoken, &event))
		goto error;
		
	IACglobals.event = &event;
	
	if (!IACpushstringparam (name, 'prm1'))
		goto error;
	
	if (!IACpushstringparam (value, 'prm2'))
		goto error;
	
	if (!IACpushshortparam (type, 'prm3'))
		goto error;
	
	if (!callcompiledscript (&event, typeChar, errorstring, &hresult))
		return (false);
		
	disposehandle (hresult);

	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	disposehandle (hresult);
	
	return (false);
	} /*frontAddToGlossary*/


boolean frontEditGlossary (bigstring errorstring) {
	
	AppleEvent event;
	Handle hresult = nil;
	
	if (!newevent (editglossarytoken, &event))
		goto error;
		
	IACglobals.event = &event;
	
	if (!callcompiledscript (&event, typeChar, errorstring, &hresult))
		return (false);
		
	disposehandle (hresult);

	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	disposehandle (hresult);
	
	return (false);
	} /*frontEditGlossary*/


boolean frontGlossaryReference (bigstring name, bigstring errorstring, Handle *hresult) {
	
	AppleEvent event;
	
	*hresult = nil;
	
	if (!newevent (refglossarytoken, &event))
		return (false);
		
	IACglobals.event = &event;
	
	if (!IACpushstringparam (name, 'prm1'))
		goto error;
	
	if (!callcompiledscript (&event, typeChar, errorstring, hresult))
		return (false);
		
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	disposehandle (*hresult);
	
	return (false);
	} /*frontGlossaryReference*/


boolean frontEditObject (bigstring celladdress, bigstring errorstring) {
	
	AppleEvent event;
	Handle hresult = nil;
	
	if (!newevent (editobjecttoken, &event))
		return (false);
		
	IACglobals.event = &event;
	
	if (!IACpushstringparam (celladdress, 'prm1'))
		goto error;
	
	if (!callcompiledscript (&event, typeChar, errorstring, &hresult))
		return (false);
		
	disposehandle (hresult);
	
	return (true);
	
	error:
	
	AEDisposeDesc (&event);	
	
	disposehandle (hresult);
	
	return (false);
	} /*frontEditObject*/

#endif


