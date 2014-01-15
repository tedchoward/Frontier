
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

#include "shell.h"
#include "langinternal.h"
#include "process.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h" /*PBS 03/14/02: AE OS X fix.*/
#endif

#ifdef flcomponent
	
	#include <SetUpA5.h>
	
	typedef pascal OSErr (*AESendCallback)(
							const AppleEvent*	theAppleEvent,
							AppleEvent*			reply,
							AESendMode 			sendMode,
							AESendPriority 		sendPriority,
							long 				timeOutInTicks,
							
							AEIdleUPP			idleProc, 
							AEFilterUPP			filterProc);
	
	typedef pascal OSErr (*AECreateAppleEventCallback)(
							AEEventClass 			theAEEventClass,
							AEEventID 				theAEEventID,
							const AEAddressDesc*	target,
							short 					returnID,
							long 					transactionID,
							AppleEvent*				result);
#endif



#define ctappletypes 24

#define unknownerror -1

#define typeTargetID 'targ'
#define typeSessionID 'ssid'

typedef DescType **hdldesctype;


typedef struct tysys7transportinfo { 
	
	hdlprocessthread hthread; /*must be first field*/
	
	/*note: AppleEvent records, not addresses, must be used to allow multiple thread accesss*/
	
	AppleEvent sys7message; /*in case a user ever needs access*/
	
	AppleEvent sys7reply; /*used internally to build reply*/
	
	boolean flownmessage; /*is message data owned by verb?*/
	
	boolean flownreply; /*is reply data owned by verb?*/
	
	boolean flnetwork: 1;
	
	boolean flprocessid: 1;
	
	boolean fltimedout: 1;
	
	boolean fluseraborted: 1;
	
	boolean fltoself: 1;
	
	boolean flsuspended: 1;
	
	ProcessSerialNumber receiverpsn; /*if a receiver's psn was provided by caller, here it is*/
	
	tynetworkaddress networkaddress; /*if it's a network verb, this is where it's going*/
	
	short replyid;
	
	unsigned long waketime; /*when hthread should be awakened (timeout)*/
	
	} tysys7transportinfo, *ptrsys7transportinfo, **hdlsys7transportinfo;


typedef struct tylandprocessvisitinfo {
	tyapplicationid idrunningprocess;
	} tylandprocessvisitinfo;

typedef struct tylandprocessvisitinfo *tylandprocessvisitinfoptr;


static void suspendcurrentevent(AppleEvent *ev) {

	/*
	2002-12-04 AR: Hack to work around bug in Mac OS X 10.2.2
	as suggested by Steve Zellers at Apple.
	*/
	
	static long sysversion = 0L;
	
	if (sysversion == 0)
		Gestalt (gestaltSystemVersion, &sysversion);
	
	if (sysversion == 0x1022L) {
	
		AEEventID eventID;
		DescType typeCode;
		Size ctbytes;
		
		if (AEGetAttributePtr(ev, keyEventIDAttr, typeType, &typeCode,
				(void*) &eventID, sizeof(eventID), &ctbytes) == noErr) {
				
			if (typeCode == typeType && ctbytes == sizeof(eventID) && eventID == kAEAnswer) {
			
				long **h = (long**) ev->dataHandle;
				long *p = *h;
				
				p[2]++;	/*increment the reference count of reply records*/
				}
			}
		}
	
	AESuspendTheCurrentEvent (ev);
	
	return;
	} /*suspendcurrentevent*/

/*
  pascal OSErr EventDirectFromSelf(AppleEvent * theAppleEventPtr, Boolean * directFlagPtr)
  // Extract the event source attribute and check if it indicates that
  // the event was directly dispatched.
  {
    short theEventSource;
    DescType actualType;
    Size actualSize;
    OSErr retCode;
    
    retCode = AEGetAttributePtr(theAppleEventPtr, keyEventSourceAttr,
      typeShortInteger, &actualType, (Ptr) &theEventSource, sizeof(short),
      &actualSize);
    
    if (retCode == noErr && theEventSource == (short) kAEDirectCall)
      *directFlagPtr = true;
    else *directFlagPtr = false;
    
    return retCode;
  }
*/


static boolean isthisprocess (ProcessSerialNumber *targetPSN) {
	
	register OSErr err;
	Boolean flsame = false; 
	
	ProcessSerialNumber psn;
	
	GetCurrentProcess (&psn);
	
	err = SameProcess (targetPSN, &psn, &flsame);
	
	if (err != noErr)
		return (false);
	
	return (flsame);
	} /*isthisprocess*/


static boolean landchecksameprocess (tynetworkaddress *adr) {

#	pragma unused (adr)

	/*
	3/9/92 dmb: if GetProcessSerialNumberFromPortName returns an error, leave 
	flsendingtoself false, but return true.  adr may be a valid port that 
	doesn't have a psn
	*/
	
	//what the hell do I do here?
	return  ( true );
		
	return (false);
		
	} /*landchecksameprocess*/


pascal boolean landstring2networkaddress (ConstStr255Param bsadr, tynetworkaddress *adr) {

#pragma unused (bsadr, adr)
	/*
	2004-10-21 aradke: Can't do this on Carbon, send back a bogus error.
	*/
	
	landseterror (noResponseErr);
	
	return (false);
	} /*landstring2networkaddress*/


pascal boolean landbrowsenetworkapps (ConstStr255Param bsprompt, tynetworkaddress *adr, tyapplicationid id) {

#	pragma unused (bsprompt, adr, id)

	/*
	7/15/91 DW: return false on system 6.
	
	2/10/92 dmb: added call to new shellactivate.  also, don't hardcode own default strings
	
	6/8/92 dmb: use landseterror to communicate error code to caller
	*/
	
	return false;

	} /*landbrowsenetworkapps*/


pascal OSErr landgetverbattr (hdlverbrecord hverb, OSType attrkey, DescType type, AEDesc *attrdata) {
	
	register hdlsys7transportinfo ht = (hdlsys7transportinfo) (**hverb).transportinfo;
	AppleEvent event = (**ht).sys7message;
	
	return (AEGetAttributeDesc (&event, (AEKeyword) attrkey, type, attrdata));
	} /*landgetverbattr*/


pascal OSErr landsetverbattr (hdlverbrecord hverb, OSType attrkey, DescType attrtype, const void * attrdata, long attrsize) {
	
	register hdlsys7transportinfo ht = (hdlsys7transportinfo) (**hverb).transportinfo;
	AppleEvent event = (**ht).sys7message;
	
	return (AEPutAttributePtr (&event, (AEKeyword) attrkey, attrtype, attrdata, attrsize));
	} /*landsetverbattr*/


#ifdef flcomponent

pascal OSErr landsystem7defaultcreate ( AEEventClass theAEEventClass, AEEventID theAEEventID,
                    const AEAddressDesc *target, short returnID,
                    long transactionID, AppleEvent *result ) {
	
	return (AECreateAppleEvent(theAEEventClass, theAEEventID, target, returnID, transactionID, result));
	} /*landsystem7defaultcreate*/


pascal OSErr landsystem7defaultsend (const AppleEvent *theAppleEvent, AppleEvent *reply,
		AESendMode sendMode, AESendPriority sendPriority, long timeOutInTicks,
        AEIdleUPP idleProc, AEFilterUPP filterProc) {
	
	return (AESend (theAppleEvent, reply, sendMode, sendPriority, timeOutInTicks, idleProc, filterProc));
	} /*landsystem7defaultsend*/

#endif


static boolean landsystem7packverb (hdlverbrecord hverb) {
	
	/*
	pack up all the information in the verb record into an appleevent 
	message.
	*/
	
	register hdlverbrecord hv = hverb;
	AppleEvent evt;
	register boolean flselfaddress = false;
	register hdlsys7transportinfo ht = (hdlsys7transportinfo) (**hv).transportinfo;
	AEAddressDesc adr;
	OSErr errcode;
	
	if ((**hv).idreceiver == idnull) /*verb is being set up to hold existing event*/
		return (true);
	
	if ((**ht).flnetwork) { /*we have a network address*/
		
		/*
		flselfaddress = (**ht).networkaddress.flself;
		*/
		
		flselfaddress = (**ht).fltoself;
		
		if (!flselfaddress) {
			
			TargetID tid;
			
			tid = (**ht).networkaddress.target;
			
			if (tid.sessionID != 0)
				errcode = AECreateDesc (typeSessionID, (Ptr) &tid.sessionID, sizeof (tid.sessionID), &adr);
			else
				errcode = AECreateDesc (typeTargetID, (Ptr) &tid, sizeof (tid), &adr);
			}
		}
	else if ((**ht).flprocessid) { /*we have receiver's process id*/
		
		ProcessSerialNumber psn = (**ht).receiverpsn;
		
		flselfaddress = isthisprocess (&psn);
		
		if (!flselfaddress)
			errcode = AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, longsizeof (psn), &adr);
		}
	else {
		
		flselfaddress = ((**hv).idreceiver == (**hv).idsender);
		
		if (!flselfaddress) {
			
			OSType sig;
			
			sig = (**hv).idreceiver;
			
			errcode = AECreateDesc (typeApplSignature, (Ptr) &sig, longsizeof (sig), &adr);
			}
		}
	
	if (flselfaddress) { /*send to self*/
		
		ProcessSerialNumber psn;
		
		psn.highLongOfPSN = 0;
		
		psn.lowLongOfPSN =  kCurrentProcess;
		
		errcode = AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, longsizeof (psn), &adr);
		
		(**ht).fltoself = true;
		}
	
	if (errcode != noErr)
		return (false);
	
	#ifdef flcomponent
		
		{
		register tyeventcreatecallback cb = (**landgetglobals ()).eventcreatecallback;
		
		errcode = (*(AECreateAppleEventCallback) cb) (
			
				(**hv).verbclass, (**hv).verbtoken, &adr, 
				
				kAutoGenerateReturnID, (**hv).idtransaction, &evt);
		}
	
	#else
		
		errcode = AECreateAppleEvent (
				
				(**hv).verbclass, (**hv).verbtoken, &adr, 
				
				kAutoGenerateReturnID, (**hv).idtransaction, &evt);
	
	#endif
	
	AEDisposeDesc (&adr);
	
	if (errcode != noErr)
		return (false);
	
	(**ht).sys7message = evt;
	
	(**ht).flownmessage = true;
	
	return (true);
	} /*landsystem7packverb*/


static boolean landsystem7getlongattr (AppleEvent *list, AEKeyword key, DescType type, void *pattr) {
	
	/*
	little layer to make code prettier
	*/
	
	OSErr errcode;
	long size;
	
	errcode = AEGetAttributePtr (list, key, type, &type, (Ptr) pattr, longsizeof (long), &size);
	
	return (errcode == noErr);
	} /*landsystem7getlongattr*/


static boolean landsystem7eventisdirect (AppleEvent *list) {
	
	/*
	2/19/93 dmb: determine if this event was sent short-curcuit, from this process
	*/
	
	OSErr errcode;
	long size;
	ProcessSerialNumber psn;
	DescType type;
	
	errcode = AEGetAttributePtr (list, keyAddressAttr, typeProcessSerialNumber, &type, (Ptr) &psn, longsizeof (psn), &size);
	
	if (errcode != noErr)
		return (false);
	
	return ((psn.highLongOfPSN == 0) && (psn.lowLongOfPSN == kCurrentProcess));
	} /*landsystem7eventisdirect*/


pascal boolean landsystem7unpackverb (AppleEvent *message, AppleEvent *reply, hdlverbrecord *hverb) {
	
	/*
	turn an appleevent into a verb record which is almost operating-system-independent.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlverbrecord hv;
	register hdlsys7transportinfo ht;
	tyverbclass verbclass;
	tyverbtoken verbtoken;
	long idtransaction;
	long ctparams;
	boolean fldirect;
	
	if (!landsystem7getlongattr (message, keyEventIDAttr, typeType, &verbtoken))
		return (false);
	
	if (!landsystem7getlongattr (message, keyEventClassAttr, typeType, &verbclass))
		verbclass = nulltoken;
	
	if (!landsystem7getlongattr (message, keyTransactionIDAttr, typeLongInteger, &idtransaction))
		idtransaction = 0;
	
	if (AECountItems (message, &ctparams) != noErr)
		return (false);
	
	if (!landnewverb (idnull, nil, verbclass, verbtoken, (short) ctparams, hverb))
		return (false);
	
	hv = *hverb; /*copy into register*/
	
	(**hv).idreceiver = (**hg).applicationid;
	
	(**hv).idtransaction = idtransaction;
	
	fldirect = landsystem7eventisdirect (message);
	
	(**hv).flverbisdirect = fldirect;
	
	ht = (hdlsys7transportinfo) (**hv).transportinfo;
	
	(**ht).sys7message = *message;
	
	if (reply != nil)
		(**ht).sys7reply = *reply;
	
	return (true); /*loop completed, all params popped with no errors*/
	} /*landsystem7unpackverb*/


static pascal void *eventthreadmain (void *hverb) {
	
	/*
	2.1a7 dmb: geteventrecords before handling the verb (which disposes it)
	
	4.0.1b1 dmb: pass name of thread to initprocessthread; call exitprocessthread
	
	4.1b3 dmb: name for initprocessthread is more informative now
	*/
	
	register hdlverbrecord hv = (hdlverbrecord) hverb;
	AppleEvent event, reply;
	FSSpec fs;
	OSType signature;
	ProcessSerialNumber psn;
	byte class [6];
	byte token [6];
	bigstring bs;
	
	landsystem7geteventrecords (hv, &event, &reply);
	
	ostypetostring ((**hv).verbclass, class);
	
	ostypetostring ((**hv).verbtoken, token);
	
	if (landsystem7getsenderinfo (&event, &psn, &fs, &signature) != noErr)
		ostypetostring ((**hv).idsender, fs.name);
	
	langgetstringlist (appleeventthreadstring, bs); // "\papple event ['^0', '^1'] from Ò^2Ó"
	
	parsedialogstring (bs, class, token, fs.name, nil, bs);
	
	initprocessthread (bs); /*must call this*/
	
	(void) landhandleverb (hv);
	
	AEResumeTheCurrentEvent (&event, &reply, (AEEventHandlerUPP) kAENoDispatch, 0);
	
	exitprocessthread ();
	
	return (nil);
	} /*eventthreadmain*/


static pascal OSErr landsystem7handleevent (AppleEvent *message, AppleEvent *reply, long refcon) {
	
	/*
	this is the dispatch routine for userland appleevents, established 
	at init time by landinit.
	
	we'll build a verb record (handle) that is the same as for ipc, 
	except that transport info will contain the original message and reply 
	appleevent records.
	
	10/23/91 dmb: always suspend/resume the event in case handler is 
	multithreaded.
	
	1/28/92 dmb: when landhandleverb fails, check landgeterror instead of always 
	returning memfullerr
	
	4/20/92 dmb: removed bogus disposehandle when landsystem7unpackverb fails
	
	4/29/92 dmb: if refcon is -1, we're being called from langipc's fastevent routine
	
	2/15/93 dmb: don't fork if verb is direct
	
	3.0.1b2 dmb: if newprocessthread fails, use processpsuedothread; don't 
	call threadmain directly, because we need to protect context globals.
	*/
	
	register hdllandglobals hg;
	hdlverbrecord hverb;
	register hdlverbrecord hv;
	hdlprocessthread hthread;
	OSErr errcode;
	boolean flsystemevent;
	tyfastverbcontext savecontext;
	long curA5;
	
	assert (refcon != -1);
	
	flsystemevent = (refcon != 0) && (refcon != typeWildCard);
	
	if (flsystemevent) {
		
		#if THINK_C
			
			asm {
				move.l	a5,-(a7)
				move.l	refcon,a5
				}
			
		#else
			
			curA5 = (long)SetUpAppA5 ();
		
		#endif
		
		landpushfastcontext (&savecontext);
		}
	
	hg = landgetglobals ();
	
	errcode = noErr;
	
	if (!landsystem7unpackverb (message, reply, &hverb)) {
		
		errcode = memFullErr;
		
		goto exit;
		}
	
	hv = hverb; /*copy into register*/
	
	/*
	(**hv).flsystemverb = flsystemevent;
	*/
	
	if (refcon == typeWildCard) {
		
		if (!(*(**hg).findhandlerroutine) (hv)) {
			
			landdisposeverb (hv);
			
			errcode = errAEEventNotHandled;
			
			goto exit;
			}
		}
	
	if ((**hv).flverbisdirect || flsystemevent || !inmainthread ()) { /*don't fork or suspend*/
	
		++fldisableyield;
		
		if (!landhandleverb (hv))
			errcode = landgeterror ();
		
		--fldisableyield;
		
		goto exit;
		}
	
	if (newprocessthread (&eventthreadmain, (tythreadmainparams) hv, &hthread))
		suspendcurrentevent (message);
	else
		processpsuedothread (&eventthreadmain, (tythreadmainparams) hv);
	
	exit: { /*normal exit; not forked thread*/
		
		if (flsystemevent) {
			
			landpopfastcontext (&savecontext);
			
			#if THINK_C
			
				asm {
					move.l	(a7)+,a5
					}
			
			#else
			
				RestoreA5 (curA5);
			
			#endif
			}
		
		return (errcode);
		}
	} /*landsystem7handleevent*/


#if !TARGET_RT_MAC_CFM
	#define landsystem7handleeventUPP ((AEEventHandlerUPP) landsystem7handleevent)
#else
	#if !TARGET_API_MAC_CARBON
		static RoutineDescriptor landsystem7handleeventDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEEventHandlerProcInfo, landsystem7handleevent);
		#define landsystem7handleeventUPP (&landsystem7handleeventDesc)
	#else
		AEEventHandlerUPP	landsystem7handleeventDesc = nil;
		#define landsystem7handleeventUPP (landsystem7handleeventDesc)
	#endif
#endif


static pascal boolean replyidvisit (Handle htinfo, long id) {
	
	hdlsys7transportinfo ht = (hdlsys7transportinfo) htinfo;
	
	return ((**ht).replyid == (short) id);
	} /*replyidvisit*/


static pascal OSErr targettoprocessinfo (const TargetID *target, ProcessSerialNumber *psn, FSSpec *fs, OSType *signature) {
	
	#if !TARGET_API_MAC_CARBON
	ProcessInfoRec info;
	OSErr ec;
	
	if ((*target).location.locationKindSelector == ppcNoLocation) { /*local program*/
		
		ec = GetProcessSerialNumberFromPortName ((PPCPortPtr) &(*target).name, psn);
		
		if (ec != noErr)
			return (ec);
		
		info.processInfoLength = (long) sizeof (info);
		
		info.processName = nil;
		
		info.processAppSpec = fs;
		
		ec = GetProcessInformation (psn, &info);
		
		if (ec != noErr)
			return (ec);
		
		*signature = info.processSignature;
		}
		
	else { /*not a local program*/
	#endif

		clearbytes (psn, sizeof (ProcessSerialNumber));
		
		clearbytes (fs, sizeof (FSSpec)); /*null*/
		
		if ((*target).name.portKindSelector == ppcByCreatorAndType)
			*signature = (*target).name.u.port.portCreator;
		else
			BlockMove ((Ptr) (*target).name.u.portTypeStr + 1, (Ptr) signature, 4L); /*kloooge*/
	
	#if !TARGET_API_MAC_CARBON
	}
	#endif

	
	return (noErr);
	} /*targettoprocessinfo*/


pascal OSErr landsystem7getsenderinfo (const AppleEvent *event, ProcessSerialNumber *psn, FSSpec *fs, OSType *signature) {
	
	OSErr err;
	DescType type;
	long size;
	TargetID target;
	
	err = AEGetAttributePtr (event, keyAddressAttr, typeTargetID, &type, &target, sizeof (target), &size);
	
	if (err == noErr)
		err = targettoprocessinfo (&target, psn, fs, signature);
	
	return (err);
	} /*landsystem7getsenderinfo*/


static pascal OSErr landsystem7replyroutine (AppleEvent *message, AppleEvent *reply, long refcon) {
#pragma unused (reply, refcon)
	/*
	10/24/91 dmb: created.
	
	this is the dispatch routine for events that are replies to messages sent with 
	the kAEQueueReply mode set.
	
	when messages are sent asynchronously, the landsystem7replyroutine send mode is used, 
	the transportinfo handle is put in our internal queue, and the process sleeps.  our 
	job is to pull the transportinfo out of the queue, stash the reply in it, and wake the 
	process.
	*/
	
	long id;
	hdlsys7transportinfo ht;
	
	if (!landsystem7getlongattr (message, keyReturnIDAttr, typeLongInteger, &id))
		return (errAEEventNotHandled);
	
	if (!landpopqueueitem (&replyidvisit, id, (Handle *) &ht))
		return (errAEEventNotHandled);
	
	suspendcurrentevent (message); /*don't want AEM to dispose it*/
	
	(**ht).sys7reply = *message;
	
	(**ht).fltimedout = false; /*a reply was received before waking*/
	
	processwake ((**ht).hthread);
	
	return (noErr);
	} /*landsystem7replyroutine*/


static pascal OSErr landsystem7openapproutine (AppleEvent *message, AppleEvent *reply, long refcon) {
#pragma unused(reply, message, refcon)

	/*
	this is the dispatch routine for the open application (required) core event.
	
	no parameters exist for this event
	
	3/17/91 mao: Pascal callback compatability
	*/
	
	register hdllandglobals hg = landgetglobals ();
	
	return ((*(**hg).macopenapproutine) ());
	} /*landsystem7openapproutine*/


static pascal OSErr landsystem7visitdoclist (AppleEvent *message, landfilespeccallback docvisitroutine) {

	/*
	this is common code between the open/print documents event handlers.
	
	the direct object parameter is a list of aliases.  we'll ask for fsspecs 
	instead, and allow the appleevent manager do the coersion for us

	3/17/91 mao: Pascal callback compatability
	
	5/20/92 dmb: required event callbacks have always returned an OSErr; we were 
	testing boolean result, yielding inverse results
	
	2.1b3 dmb: set new maceventsender field
	*/
	
	register OSErr err;
	register short n;
	register hdllandglobals hg = landgetglobals ();
	AEDescList doclist;
	long ctdocs;
	AEKeyword key;
	DescType type;
	long size;
	FSSpec fs;
	OSType signature;
	ProcessSerialNumber psn;
	
	err = AEGetParamDesc (message, keyDirectObject, typeAEList, &doclist);
	
	if (err != noErr)
		return (err);
	
	if (landsystem7getsenderinfo (message, &psn, &fs, &signature) != noErr)
		signature = 0;
	
	(**hg).maceventsender = signature;
	
	err = AECountItems (&doclist, &ctdocs);
	
	for (n = 1; n <= (short) ctdocs; n++) {
		
		err = AEGetNthPtr (&doclist, n, typeFSS, &key, &type, (Ptr) &fs, sizeof (fs), &size);
		
		if (err != noErr)
			break;
		
		err = (*docvisitroutine) (&fs);
		
		if (err != noErr)
			break;
		} /*for*/
	
	AEDisposeDesc (&doclist);
	
	return (err);
	} /*landsystem7visitdoclist*/


static pascal OSErr landsystem7opendocsroutine (AppleEvent *message, AppleEvent *reply, long refcon) {
#pragma unused (reply, refcon)
	/*
	this is the dispatch routine for the open documents (required) core event.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	
	return (landsystem7visitdoclist (message, (**hg).macopendocroutine));
	} /*landsystem7opendocsroutine*/


static pascal OSErr landsystem7printdocsroutine (AppleEvent *message, AppleEvent *reply, long refcon) {

	/*
	this is the dispatch routine for the print documents (required) core event.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	
	if ((**hg).macprintdocroutine == nil)
		return (landsystem7handleevent (message, reply, refcon));
	
	return (landsystem7visitdoclist (message, (**hg).macprintdocroutine));
	} /*landsystem7printdocsroutine*/


static pascal OSErr landsystem7quitapproutine (AppleEvent *message, AppleEvent *reply, long refcon) {
#pragma unused (message, reply, refcon)
	/*
	this is the dispatch routine for the quit application (required) core event.
	
	no parameters exist for this event

	3/17/91 mao: Pascal callback compatability
	*/
	
	register hdllandglobals hg = landgetglobals ();
	
	return ((*(**hg).macquitapproutine) ());
	} /*landsystem7quitapproutine*/


static boolean landvisitprocesses (
		pascal boolean (* visitroutine) (tylandprocessvisitinfoptr visitinfo, ProcessInfoRec *processinfo),
		tylandprocessvisitinfoptr visitinfo) {

	ProcessInfoRec processinfo;
	ProcessSerialNumber psn;
	bigstring bsname;
	FSSpec fss;
	
	processinfo.processName = bsname; /*place to store process name*/
	
	processinfo.processAppSpec = &fss; /*place to store process filespec*/
	
	psn.highLongOfPSN = kNoProcess;
	
	psn.lowLongOfPSN = kNoProcess;
	
	while (GetNextProcess (&psn) == noErr) {
		
	 	processinfo.processInfoLength = sizeof (ProcessInfoRec);
	 	
		if (GetProcessInformation (&psn, &processinfo) == noErr) {
		
			if (!(*visitroutine) (visitinfo, &processinfo))
				return (false);
			}
		}
	
	return (true);
	} /*landvisitprocesses*/


static pascal boolean landprocesscreatorvisit (tylandprocessvisitinfoptr visitinfo, ProcessInfoRec *processinfo) {
	
	return (processinfo->processSignature != visitinfo->idrunningprocess);
	} /*landprocesscreatorvisit*/


boolean landsystem7apprunning (tyapplicationid idapp) {

	/*
	2002-11-15 AR: Switched from using a global to a stack-allocated struct
	for passing context to the landproccesscreatorvist callback function
	*/
	
	tylandprocessvisitinfo info;
	
	info.idrunningprocess = idapp;
	
	return (!landvisitprocesses (&landprocesscreatorvisit, &info));
	} /*landsystem7apprunning*/


static boolean landsystem7newtransportinfo (hdlverbrecord hverb) {
	
	hdlsys7transportinfo ht;
	
	if (!landnewclearhandle (longsizeof (tysys7transportinfo), (Handle *) &ht))
		return (false);
	
	(**hverb).transportinfo = (Handle) ht;
	
	return (true);
	} /*landsystem7newtransportinfo*/


boolean landsystem7newverb (hdlverbrecord hverb, ProcessSerialNumber *psn) {
	
	/*
	6/16/93 dmb: take psn paramter
	*/
	
	register hdlsys7transportinfo ht;
	
	if (!landsystem7newtransportinfo (hverb))
		return (false);
	
	if ((psn != nil) && ((*psn).lowLongOfPSN != kNoProcess)) {
		
		ht = (hdlsys7transportinfo) (**hverb).transportinfo;
		
		(**ht).flprocessid = true;
		
		(**ht).receiverpsn = *psn;
		}
	
	return (landsystem7packverb (hverb));
	} /*landsystem7newverb*/


boolean landsystem7newnetworkverb (hdlverbrecord hverb, tynetworkaddress *adr) {
	
	register hdlsys7transportinfo ht;
	boolean fl;
	
	if (!landsystem7newtransportinfo (hverb))
		return (false);
	
	ht = (hdlsys7transportinfo) (**hverb).transportinfo;
	
	(**ht).flnetwork = true;
	
	(**ht).networkaddress = *adr;
	
	fl = landchecksameprocess (adr);
	
	(**ht).fltoself = fl;
	
	return (landsystem7packverb (hverb));
	} /*landsystem7newnetworkverb*/


void landsystem7disposeverb (hdlverbrecord hverb) {
	
	register hdlsys7transportinfo ht = (hdlsys7transportinfo) (**hverb).transportinfo;
	
	if ((**ht).flownmessage)
		AEDisposeDesc (&(**ht).sys7message);
	
	if ((**ht).flownreply)
		AEDisposeDesc (&(**ht).sys7reply);
	} /*landsystem7disposeverb*/


static pascal Boolean landsystem7idleroutine (EventRecord *ev, long *sleep, RgnHandle *mousergn) {
#pragma unused (sleep, mousergn)
	
	/*
	10/23/91 dmb: we now pass the event through to the application so it can 
	respond to it
	*/
	
	
	if (landbreakembrace (ev)) /*user holding down cmd, option and shift keys*/
		return (-1); /*stop waiting*/
	
	if (!processyield ())
		return (-1);
	
	return (false); /*keep waiting*/
	} /*landsystem7idleroutine*/


#if !TARGET_RT_MAC_CFM
	
	#define landsystem7idleUPP (&landsystem7idleroutine)
	
	
#elif TARGET_API_MAC_CARBON == 1

	AEIdleUPP landsystem7idleUPP;
	
#else

	static RoutineDescriptor landsystem7idleDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEIdleProcInfo, landsystem7idleroutine);
	
	#define landsystem7idleUPP (&landsystem7idleDesc)
	
#endif


pascal boolean landsystem7geteventrecords (hdlverbrecord hverb, AppleEvent *event, AppleEvent *reply) {
	
	/*
	3/30/93 dmb: this routine allows us to keep the sys7transportinfo structure private.
	*/
	
	register hdlsys7transportinfo ht = (hdlsys7transportinfo) (**hverb).transportinfo;
	
	*event = (**ht).sys7message;
	
	*reply = (**ht).sys7reply;
	
	return (true);
	} /*landsystem7geteventrecords*/


boolean landsystem7send (hdlverbrecord hverb, hdlverbrecord *hvalues) {
	
	/*
	pack the verb into an appleevent and send it to the destination program.  then
	unpack the reply into a new verb handle pointed to by hvalues
	
	10/25/91 dmb: implemented queued-response verbs
	
	10/29/91 dmb: use filter routine to avoid deadlock of main thread
	
	1/2/92 dmb: don't allow interaction when sending network verbs (even when local)
	
	7/29/92 dmb: don't queue reply if fldisableyield is set; sleeping is like yielding
	
	3.0.2b1 dmb: calculate a waketime here, instead of a bedtime. this way, we look 
	at the timeoutticks eventinfo setting here in the right thread, instead of always 
	looking at the setting of the thread the happens to be checking for timeouts, 
	which is always the main thread.
	*/
	
	register hdlverbrecord hv = hverb;
	register hdlsys7transportinfo ht = (hdlsys7transportinfo) (**hv).transportinfo;
	register OSErr errcode;
	register hdllandglobals hg = landgetglobals ();
	AppleEvent message;
	AppleEvent reply;
	long mode;
	long interaction;
	register boolean fl;
	long ctparams;
	long id;
	
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	
		landsystem7idleUPP = NewAEIdleUPP (&landsystem7idleroutine);

	#endif
		
	/*
	if (!landsystem7packverb (hv, &message))
		return (false);
	*/
	
	message = (**ht).sys7message;
	
	if ((**hv).flverbisdirect) {
		register landverbrecordcallback cb = (**hg).handleverbroutine;
		
		mode = kAEWaitReply;
		
		errcode = AECreateList (nil, 0, true, &reply);
		
		(**ht).sys7reply = reply;
		
		landseterror (noErr);
		
		(*cb) (hv);
		
		errcode = landgeterror ();
		}
	else { /*send the message & wait for reply*/
		
		if ((**hv).flverbgetsnoreply)
			mode = kAENoReply;
		
		else {
			
			if ((inmainthread ()) || (**ht).fltoself || fldisableyield)
				mode = kAEWaitReply;
			else
				mode = kAEQueueReply;
			}
		
		if ((**hg).eventsettings.interactionlevel != 0)
			interaction = (**hg).eventsettings.interactionlevel;
		
		else {
			
			if ((**ht).flnetwork)
				interaction = kAENeverInteract;
			else
				interaction = kAEAlwaysInteract + kAECanSwitchLayer;
			}
		
		#ifdef flcomponent
			
			{
			register tyeventsendcallback cb = (**hg).eventsendcallback;
			
			errcode = (*(AESendCallback) cb) (
				
				&message, &reply, mode + interaction + kAEDontRecord, kAENormalPriority, 
				
				kNoTimeOut, landsystem7idleUPP, nil);
			}
		
		#else
		
			errcode = AESend (
				
				&message, &reply, mode + interaction, kAENormalPriority, 
				
				kNoTimeOut, landsystem7idleUPP, nil);
		
		#endif
		
		#if TARGET_API_MAC_CARBON == 1
		
			DisposeAEIdleUPP (landsystem7idleUPP);
		
		#endif
		
		landsystem7getlongattr (&message, keyReturnIDAttr, typeLongInteger, &id); /*before disposal*/
		}
	
	AEDisposeDesc (&message);
	
	(**ht).flownmessage = false;
	
	if (errcode != noErr) {
		
		landseterror (errcode);
		
		return (false);
		}
	
	if (mode == kAEQueueReply) {
		
		hdlprocessthread hthread = getcurrentthread ();
		unsigned long waketime = (**hg).eventsettings.timeoutticks;
		
		(**ht).hthread = hthread;
		
		/*
		if ((long) waketime > 0)
			waketime += TickCount (); 	//when to time out
		else
			waketime = 0xffffffff; 		//never time out
		
		(**ht).waketime = waketime;
		*/
		
		(**ht).replyid = id;
		
		(**ht).fltimedout = true; /*4.1b3 dmb: until set otherwise, assume this*/
		
		(**ht).fluseraborted = false; /*4.1b3 dmb: default state, ctn'd*/
		
		landpushqueue ((Handle) ht);
		
		if (!processsleep (hthread, waketime)) /*killed in our sleep -- bummer!*/ {
			
			landseterror (usercancellederror); /*silent error*/
			
			return (false);
			}
		
		/*we're back! grab reply from transport info*/
		
		if ((**ht).fltimedout) {
			
			landseterror (errAETimeout);
			
			return (false);
			}
		
		if ((**ht).fluseraborted) {
			
			landseterror (errAEWaitCanceled);
			
			return (false);
			}
		
		reply = (**ht).sys7reply;
		}
	
	/*we got our answer, process it and return*/
	
	errcode = AECountItems (&reply, &ctparams);
	
	if (!landnewverb (idnull, nil, nullclass, nulltoken, ctparams, hvalues)) {
		
		/*failed to create verb handle to hold returned values*/
		
		return (false); 
		}
	
	hv = *hvalues; /*copy into register*/
	
	fl = true; /*parameter retrieval optimism*/
	
	ht = (hdlsys7transportinfo) (**hv).transportinfo;
	
	(**ht).sys7message = reply;
	
	(**ht).flownmessage = true;
	
	return (fl);
	} /*landsystem7send*/


static pascal void landsystem7checktimeouts (EventRecord *ev) {
	
	/*
	3.0.2b1 dmb: timeout time is now calculated when sending with the correct 
	thread active. here, eventsettings.timeoutticks tells us about the main 
	thread, not the threads that are sleeping (which can each have their own 
	setting).
	
	4.1b3 dmb: don't check timeout here; the processsleep handles that now.
	*/
	
	hdlsys7transportinfo ht;
	
	if (landbreakembrace (ev)) {
		
		if (landpopqueue ((Handle *) &ht)) { /*abort 1st in queue*/
		
			(**ht).fluseraborted = true;
		
			processwake ((**ht).hthread);
			}
		}
	
	/*
	while (landpopqueueitem (&timeoutvisit, (long) TickCount (), (Handle *) &ht)) {
		
		(**ht).fltimedout = true;
		
		processwake ((**ht).hthread);
		}
	*/
	
	} /*landsystem7checktimeouts*/


#if !flruntime

boolean landsystem7eventfilter (EventRecord *ev) {
	
	/*
	watch for high level events (AppleEvents)
	
	return true if we consume the event, false otherwise.
	*/
	
	switch ((*ev).what) {
		
		case kHighLevelEvent:
			AEProcessAppleEvent (ev);
			
			return (true); /*consume the event*/
		
		case nullEvent:
			landsystem7checktimeouts (ev);
			
			return (false);
		} /*switch*/
	
	return (false); /*don't consume the event*/
	} /*landsystem7eventfilter*/

#endif

#if 0 // def THINK_C

	static boolean landsystem7installhandler (AEEventClass class, AEEventID id, tyeventhandler handler) {
		
		/*
		little layer to make code prettier
		*/
		
		OSErr errcode;
		
		errcode = AEInstallEventHandler (class, id, (EventHandlerProcPtr) handler, 0, false);
		
		return (errcode == noErr);
		} /*landsystem7installhandler*/
	
	
	pascal boolean landsystem7installfasthandler (tyverbclass class, tyverbtoken token, tyeventhandler handler) {
		
		OSErr errcode;
		
		errcode = AEInstallEventHandler (class, token, (EventHandlerProcPtr) handler, (long) CurrentA5, true);
		
		return (errcode == noErr);
		} /*landsystem7installfasthandler*/

	
	boolean landsystem7addclass (tyverbclass class) {
		
		return (landsystem7installhandler (class, typeWildCard, (ProcPtr) &landsystem7handleevent));
		} /*landsystem7addclass*/
	
	
	boolean landsystem7addfastverb (tyverbclass class, tyverbtoken token) {
		
		return (landsystem7installfasthandler (class, token, &landsystem7handleevent));
		} /*landsystem7addfastverb*/
	
	
	boolean landsystem7acceptanyverb (void) {
		
		OSErr errcode;
		
		errcode = AEInstallEventHandler (typeWildCard, typeWildCard, (EventHandlerProcPtr) &landsystem7handleevent, typeWildCard, false);
		
		return (errcode == noErr);
		} /*landsystem7acceptanyverb*/
	
	
	pascal boolean landsystem7removefasthandler (tyverbclass class, tyverbtoken token) {
		
		EventHandlerProcPtr handler;
		long refcon;
		
		if (AEGetEventHandler (class, token, &handler, &refcon, true) != errAEHandlerNotFound) {
			
			if (refcon == (long) CurrentA5) /*it's our handler*/
				AERemoveEventHandler (class, token, handler, true);
			}
		} /*landsystem7removefasthandler*/

#else

	static boolean landsystem7installhandlerUPP (AEEventClass class, AEEventID id, AEEventHandlerUPP handler) {
		
		OSErr errcode;
		
		errcode = AEInstallEventHandler (class, id, handler, 0, false);
		
		return (errcode == noErr);
		} /*landsystem7installhandler*/
	
	
	static pascal boolean landsystem7installfasthandlerUPP (tyverbclass class, tyverbtoken token, AEEventHandlerUPP handler) {
		
		OSErr errcode;
		//Code change by Timothy Paustian Monday, June 26, 2000 3:45:05 PM
		//A5 worlds make no sense on PPC so why pass this as our refcon
		#if TARGET_API_MAC_CARBON == 1
		errcode = AEInstallEventHandler (class, token, handler, (long) nil, true);
		#else
		errcode = AEInstallEventHandler (class, token, handler, (long) LMGetCurrentA5 (), true);
		#endif
		
		return (errcode == noErr);
		} /*landsystem7installfasthandler*/
	
	
	static boolean landsystem7installhandler (AEEventClass class, AEEventID id, tyeventhandler handler) {
		
		/*
		little layer to make code prettier
		*/
		#if TARGET_API_MAC_CARBON == 1

			return (landsystem7installhandlerUPP (class, id, NewAEEventHandlerUPP ((AEEventHandlerProcPtr) handler)));
		
		#else

			return (landsystem7installhandlerUPP (class, id, NewAEEventHandlerProc ((AEEventHandlerProcPtr) handler)));
		
		#endif
		} /*landsystem7installhandler*/
	
	
	pascal boolean landsystem7installfasthandler (tyverbclass class, tyverbtoken token, tyeventhandler handler) {
		
		#if TARGET_API_MAC_CARBON == 1
	
			return (landsystem7installfasthandlerUPP (class, token, NewAEEventHandlerUPP ((AEEventHandlerProcPtr) handler)));
		
		#else

			return (landsystem7installfasthandlerUPP (class, token, NewAEEventHandlerProc ((AEEventHandlerProcPtr) handler)));
		
		#endif
		} /*landsystem7installfasthandler*/

	
	boolean landsystem7addclass (tyverbclass class) {
		
		//Code change by Timothy Paustian Friday, July 28, 2000 1:31:13 PM
		//create the new handler if its still nil
		#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
		if (landsystem7handleeventUPP == nil)
			landsystem7handleeventUPP = NewAEEventHandlerUPP(landsystem7handleevent);
		#endif
		
		return (landsystem7installhandlerUPP (class, typeWildCard, landsystem7handleeventUPP));
		} /*landsystem7addclass*/
	
	
	boolean landsystem7addfastverb (tyverbclass class, tyverbtoken token) {
		//Code change by Timothy Paustian Friday, July 28, 2000 1:31:13 PM
		//create the new handler is its still nil
		#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
		if (landsystem7handleeventUPP == nil)
			landsystem7handleeventUPP = NewAEEventHandlerUPP(landsystem7handleevent);
		#endif
		return (landsystem7installfasthandlerUPP (class, token, landsystem7handleeventUPP));
		} /*landsystem7addfastverb*/
	
	
	boolean landsystem7acceptanyverb (void) {
		
		OSErr errcode;
		//Code change by Timothy Paustian Friday, July 28, 2000 1:31:13 PM
		//create the new handler is its still nil
		#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
		if (landsystem7handleeventUPP == nil)
			landsystem7handleeventUPP = NewAEEventHandlerUPP(landsystem7handleevent);
		#endif
		errcode = AEInstallEventHandler (typeWildCard, typeWildCard, landsystem7handleeventUPP, typeWildCard, false);
		
		return (errcode == noErr);
		} /*landsystem7acceptanyverb*/
	
	
	pascal boolean landsystem7removefasthandler (tyverbclass class, tyverbtoken token) {
		
		AEEventHandlerUPP handler;
		long refcon;
		
		if (AEGetEventHandler (class, token, &handler, &refcon, true) != errAEHandlerNotFound) {
			
			#if TARGET_API_MAC_CARBON == 1
			if(refcon == (long) 'LAND'){
			#else
			if (refcon == (long) LMGetCurrentA5 ()) { /*it's our handler*/
			#endif
			
				AERemoveEventHandler (class, token, handler, true);
				//I need to dispose of these handlers
				return (true);
				}
			}
		
		return (false);
		} /*landsystem7removefasthandler*/

#endif


pascal boolean landsystem7pushparam (AERecord *evt, typaramtype type, Handle hval, void *pval, long len, typaramkeyword key) {
	
	/*
	push the given parameter record onto the verb's parameter list.  all of 
	the landpushxxx routines below funnel through this routine
	
	1/30/92 dmb: landseterror (memFullErr) if enlargehandle fails
	
	4/28/92 dmb: this is now a transport-specific operation
	
	4.0b7 dmb: don't dispose of hval if we fail. also check for null event
	*/
	
	AEDesc desc;
	OSErr err = noErr;
	
	if (hval != nil) {
		
		desc.descriptorType = type;
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			newdescwithhandle (&desc, type, hval);
			
		#else
		
			desc.dataHandle = hval;
		
		#endif
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/

			if (AEGetDescDataSize (evt) > 0) /*instead of checking that's it null*/
		
		#else
		
			if ((*evt).dataHandle != NULL)	// 4.0b7 dmb: make sure it's not a null reply
		
		#endif
		
			err = AEPutParamDesc (evt, key, &desc);
		
		if (err == noErr)
			disposehandle (hval);
		}
	else {
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/

			if (AEGetDescDataSize (evt) > 0) /*instead of checking that's it null*/
			
		#else
		
			if ((*evt).dataHandle != NULL)	// 4.0b7 dmb: make sure it's not a null reply
		
		#endif
		
			err = AEPutParamPtr (evt, key, type, pval, len);
		}
	
	if (err != noErr) {
		
		landseterror (err);
		
		return (false);
		}
	
	return (true);
	} /*landsystem7pushparam*/


pascal boolean landsystem7getparam (const AERecord *evt, typaramkeyword key, typaramtype type, typaramrecord *param) {
	
	/*
	4/28/92 dmb: created
	
	2.1b3 dmb: operate on AERecord, not hdlverbrecord
	*/
	
/*
	register hdlverbrecord hv = hverb;
	register hdlsys7transportinfo ht = (hdlsys7transportinfo) (**hv).transportinfo;
	AppleEvent evt = (**ht).sys7message;
*/
	register OSErr errcode;
	
	errcode = AEGetKeyDesc (evt, key, type, &(*param).desc);
	
	if (errcode != noErr) {
		
		landseterror (errcode);
		
		return (false);
		}
	
	(*param).key = key;
	
	/*
	fl = landsystem7unpackdesc (&desc, key, param); /%may set desc's data to nil%/
	
	AEDisposeDesc (&desc);
	*/
	
	return (true);
	} /*landsystem7getparam*/


pascal boolean landsystem7getnthparam (const AERecord *evt, short n, typaramrecord *param) {
	
	/*
	4/28/92 dmb: created
	
	7/17/92 dmb: don't set error if parameter is optional
	
	2.1b3 dmb: operate on AERecord, not hdlverbrecord
	*/
	
/*
	register hdlsys7transportinfo ht = (hdlsys7transportinfo) (**hverb).transportinfo;
	AppleEvent evt = (**ht).sys7message;
*/
	register OSErr errcode;
	AEKeyword key;
	
	errcode = AEGetNthDesc (evt, (long) n, typeWildCard, &key, &(*param).desc);
	
	if (errcode != noErr) {
		
		landseterror (errcode);
		
		return (false);
		}
	
	(*param).key = key;
	
	return (true);
	
	/*
	fl = landsystem7unpackdesc (&desc, key, param); /%may set desc's data to nil%/
	
	AEDisposeDesc (&desc);
	
	return (fl);
	*/
	} /*landsystem7getnthparam*/


static short landcountverbs (hdlverbarray hverbs) {
	
	/*
	return the number of verbs stored in the array.  factors out some code
	that was appearing all over the place.
	*/
	
	return ((short) (landgethandlesize ((Handle) hverbs) / longsizeof (tyverbarrayelement)));
	} /*landcountverbs*/
	

static boolean removefasthandlers (hdlverbarray hverbs) {
	
	/*
	return true if the token is found in the indicated verb array.
	
	we split this code out so that Toolkit users can easily find out if another
	application supports a particular verb.
	
	9/4/91 DW: special case -- if token is 0 only match the class, it's a wildcard
	search for any verb in a class.
	*/
	
	register hdlverbarray hvs = hverbs; 
	register short ctloops, i;
	
	if (hvs == nil) /*no verbs have been registered*/
		return (false);
	
	ctloops = landcountverbs (hvs);
	
	for (i = 0; i < ctloops; i++) {
		
		tyverbarrayelement item;
		
		item = (**hverbs) [i];
		
		if (item.flfasthandler)
			landsystem7removefasthandler (item.class, item.token);
		}
	
	return (true);
	} /*removefasthandlers*/


boolean landsystem7close (void) {

	register hdllandglobals hg = landgetglobals ();
	
	/*
	landdisposehandle ((Handle) (**hg).mactypes);
	*/
	
	removefasthandlers ((**hg).verbarray);
	
	return (true); 
	} /*landsystem7close*/


static pascal OSErr whyinternational (DescType itxt, const void *x, Size len, DescType text, SInt32 refcon, AEDesc *result) {
#pragma unused (itxt, refcon)

	if (*(short *) x != smRoman) /*can't treat this as plain text*/
		return (errAECoercionFail);
	
	return (AECreateDesc (text, ((Ptr) x) + 4, len - 4, result)); /*skip over script & language codes*/
	} /*whyinternational*/


boolean landsystem7init (void) {
	
	/*
	initialize the appleevent manager and set up a dispatch routine for 
	the event classes that we want to handle.
	
	we set up special handlers for the four required event, then a wildcard 
	dispatcher to handle everything else.
	
	10/8/91 dmb: don't install handler for applicationid here; it will happen 
	during the verb-adding process, via landaddclass.
	
	3/2/92 dmb: our double values are SANE extended, and must use type 'exte'
	
	12/11/92 dmb: 
	*/
	
	register hdllandglobals hg = landgetglobals ();
	

	#ifdef flcomponent
	
		#if !TARGET_API_MAC_CARBON
		RememberA5 ();
		#endif /*for event handlers, browser hook*/
	
	#endif
	
	(**hg).macnetglobals.flhavebrowsed = false;
	
	(**hg).eventsettings.timeoutticks = kNoTimeOut;
	
	/* already zero:
	
	(**hg).eventsettings.transactionid = kAnyTransactionID;
	
	*/
	if (!landsystem7installhandler (kCoreEventClass, kAEAnswer, landsystem7replyroutine))
		return (false);
	
	if (!landsystem7installhandler (kCoreEventClass, kAEOpenApplication, landsystem7openapproutine))
		return (false);
	
	if (!landsystem7installhandler (kCoreEventClass, kAEOpenDocuments, landsystem7opendocsroutine))
		return (false);
	
	if (!landsystem7installhandler (kCoreEventClass, kAEPrintDocuments, landsystem7printdocsroutine))
		return (false);
	
	if (!landsystem7installhandler (kCoreEventClass, kAEQuitApplication, landsystem7quitapproutine))
		return (false);
	
	#if TARGET_API_MAC_CARBON == 1
	
		AEInstallCoercionHandler ('itxt', 'TEXT', (AECoercionHandlerUPP) NewAECoercePtrUPP (whyinternational), 0, false, false);
	
	#else
	
		AEInstallCoercionHandler ('itxt', 'TEXT', (AECoercionHandlerUPP) NewAECoercePtrProc (whyinternational), 0, false, false);
	
	#endif
	
	return (true);
	} /*landsystem7init*/


