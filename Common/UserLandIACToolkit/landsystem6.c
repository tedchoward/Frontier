
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
#include "landtransport.h"



typedef struct tysys6transportinfo { 
	
	short sys6processid; /*process id of message receiver*/
	} tysys6transportinfo, *ptrsys6transportinfo, **hdlsys6transportinfo;
	
	
#define mostrecentversionnumber 1


typedef struct tymessageheader { /*one of these at the head of each sys6 message*/
	
	short versionnumber; /*makes it easy to upgrade the message format*/
	
	tyverbclass verbclass; /*4-byte class of verb, thank you very much Apple*/
	
	tyverbtoken verbtoken; /*4-byte name of verb, thank you Apple*/
	
	tyapplicationid idsender; /*tells message receiver who to send value to*/
	
	short ctparams; /*the number of parameters in this verb*/
	} tymessageheader, *ptrmessageheader, **hdlmessageheader;
	
	


static boolean landsystem6packparam (typaramrecord param, Handle hpacked) {
	
	/*
	push the verb parameter at the end of the packed handle.
	
	we split the world into two cases -- scalars and non-scalars.  for scalars,
	just push the verbparam record.  for non-scalars, change the val field of
	the verbparam to store the length of the value, which is pushed at the end
	of the handle following the verbparam record.
	
	we noticed that all non-scalar value types currently supported (10/22/90 DW) 
	are simple handles, with no handles dangling from their records.  we exploit
	this fact -- if it's not a scalar, we just size the handle, store the length
	and push the handle contents.  if at some time we have more complex value
	structures, this routine will get more complex.
	*/
	
	register Handle hdata;
	register boolean fl;
	
	if (landscalartype (param.type)) { /*scalars are very easy to handle*/
		
		return (landenlargehandle (hpacked, longsizeof (typaramrecord), &param));
		}
		
	/*assume that the value field points to an already-packed handle*/
	
	hdata = param.val.binaryvalue; /*copy the handle into a register*/
	
	param.val.longvalue = landgethandlesize (hdata); /*set size*/
	
	if (!landenlargehandle (hpacked, longsizeof (typaramrecord), &param))
		return (false);
		
	landlockhandle (hdata);
	
	fl = landenlargehandle (hpacked, param.val.longvalue, *hdata);
	
	landunlockhandle (hdata);
	
	return (fl);
	} /*landsystem6packparam*/
	
	
static boolean landsystem6unpackparam (Handle hpacked, long *ix, typaramrecord *param) {
	
	/*
	unpack the parameter from offset indicated by ix, if it's a scalar we're done,
	if it's heap-allocated there's more work to do.  see comment at head of the
	packing routine.
	*/
	
	typaramrecord info;
	register long ctbytes;
	Handle hdata;
	
	if (!landloadfromhandle (hpacked, ix, longsizeof (info), &info))
		return (false);
		
	if (landscalartype (info.type)) { /*scalars are very easy to handle*/
	
		*param = info; /*copy into returned record*/
		
		return (true);
		}
	
	ctbytes = info.val.longvalue;
	
	if (!landloadfromhandletohandle (hpacked, ix, ctbytes, &hdata))
		return (false);
		
	info.val.binaryvalue = hdata; /*copy handle into value, works for all types*/
	
	*param = info; /*copy into returned record*/
	
	return (true);
	} /*landsystem6unpackparam*/
	
	
static boolean landsystem6packallparams (hdlverbrecord hverb, Handle hpacked) {
	
	register hdlverbrecord hv = hverb;
	register short ct = (**hv).nextparam;
	register short i;
	
	for (i = 0; i < ct; i++) {
		
		if (!landsystem6packparam ((**hv).params [i], hpacked)) 
			return (false);
		} /*for*/
		
	return (true);
	} /*landsystem6packallparams*/


static boolean landsystem6packverb (hdlverbrecord hverb, Handle *hpacked) {
	
	/*
	pack up all the information in the verb record into a single contiguous
	hunk of memory, suitable for transportation.
	
	10/31/90 DW: the number of parameters linked into the verb isn't ctparams,
	that's just the amount of space allocated in verbrecord for params.  the
	logical number of params (as opposed to physical) is the value of nextparam,
	since the array is 0-based and a count is 1-based.
	*/
	
	register hdlverbrecord hv = hverb;
	register Handle h;
	tymessageheader header;
	
	header.versionnumber = mostrecentversionnumber;
	
	header.verbclass = (**hv).verbclass;
	
	header.verbtoken = (**hv).verbtoken;
	
	header.idsender = (**hv).idsender;
	
	header.ctparams = (**hv).nextparam;
	
	*hpacked = nil; /*default return value*/
	
	if (!landnewfilledhandle (&header, longsizeof (header), hpacked))
		return (false);
	
	h = *hpacked; /*copy into register*/
	
	if (!landsystem6packallparams (hv, h)) {
		
		landdisposehandle (h);
		
		*hpacked = nil;
		
		return (false);		
		}
		
	return (true);
	} /*landsystem6packverb*/
	
	
static boolean landsystem6unpackverb (Handle hpacked, hdlverbrecord *hverb) {
	
	/*
	turn a packed handle, transported by our system 6 INIT, into a verb record
	which is almost operating-system-independent.
	
	we have a handle linked into the verb record, transportinfo, in which we
	record the process id of the message sender, so we know who to send the
	returned value to.
	*/
	
	register hdlverbrecord hv;
	long ix = 0;
	tymessageheader header;
	register short i;
	
	if (!landloadfromhandle (hpacked, &ix, longsizeof (header), &header)) 
		return (false);
	
	if (!landnewverb (header.idsender, header.verbclass, header.verbtoken, header.ctparams, hverb))
		return (false);
	
	hv = *hverb; /*copy into register*/
	
	for (i = 0; i < header.ctparams; i++) {
		
		typaramrecord param;
		
		if (!landsystem6unpackparam (hpacked, &ix, &param))
			goto error;
			
		if (!landpushparamrecord (hv, param))
			goto error;
		} /*for*/
	
	return (true); /*loop completed, all params popped with no errors*/
	
	error:
	
	landdisposeverb (hv);
	
	return (false);
	} /*landsystem6unpackverb*/


boolean landsystem6apprunning (tyapplicationid idapp) {
	
	/*
	under system 6, we consider a "running" app to be one that has 
	registered with the UserLand INIT.  note that landapprunning will, 
	in addition, try to send a special message to the app.
	*/
	
	bigstring bs;
	short id;
	
	land4bytestostring (idapp, bs);
	
	return (ipcFind ((PIN *) bs, &id) == noErr);
	} /*landsystem6apprunning*/
	
	
boolean landsystem6newverb (hdlverbrecord hverb) {
	
	register hdlverbrecord hv = hverb;
	register tyapplicationid idreceiver;
	hdlsys6transportinfo h;
	bigstring bs;
	short id;
	
	if (!landnewclearhandle (longsizeof (tysys6transportinfo), (Handle *) &h))
		return (false);
	
	idreceiver = (**hv).idreceiver; /*copy into register*/
	
	if (idreceiver == idnull) /*a message sent to no one*/
		id = -1;
		
	else {
		land4bytestostring (idreceiver, bs);
	
		if (ipcFind ((PIN *) bs, &id) != noErr) {
		
			landdisposehandle ((Handle) h);
			
			return (false);
			}
		}
		
	(**h).sys6processid = id;
	
	(**hv).transportinfo = (Handle) h;
	
	return (true);
	} /*landsystem6newverb*/
	


boolean landsystem6send (hverb, hvalues) hdlverbrecord hverb, *hvalues; {
	
	/*
	by popular request -- a much richer version of verb sending.
	
	we send the verb as in the original version and wait for a returned 
	packet.  that reply can have lots of returned values, not just one as with 
	the original send.
	
	we push all those returned values onto a newly created verb record, and
	the you can call the usual parameter-getting routines to pull off the
	values you're interested in.
	
	11/6/90 dmb: support noreply mode
	
	1/1/90-3/x/90 mao: networking support and several other fixes (incl. queuing events)
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlverbrecord hv = hverb;
	register short idsender, idreceiver;
	short idmessage;
	register boolean fl = true;
	Handle hpacked;
	long ix = 0;

	if (!landsystem6packverb (hv, &hpacked))
		return (false);

	idreceiver = (**(hdlsys6transportinfo) (**hv).transportinfo).sys6processid;
	
	idsender = (**hg).system6processid;
		
	fl = ipcSend (idsender, idreceiver, hpacked) == noErr;
	
	landdisposehandle (hpacked);
	
	if (!fl)
		return (false);

	while (true) { /*wait for returned value*/
		
		EventRecord ev;
		
		landsurrenderprocessor (&ev); /*give the other guy a chance to respond*/
		
		if (landbreakembrace (&ev)) /*user holding down cmd, option and shift keys*/
			return (false);
		
		ipcReceive (idsender, &idmessage, &hpacked);
		
		if (hpacked == nil) /*no message received*/
			continue;
			
		if (idmessage != idreceiver) { /*message is from someone else, queue it*/
			
			landpushqueue (hpacked);
			
			continue;
			}
			
		/*we got our answer, process it and return*/
				
		if (!landnewverb (idnull, nullclass, nulltoken, 0, hvalues)) {
			
			/*failed to create verb handle to hold returned values*/

			landdisposehandle (hpacked);

			return (false); 
			}
		
		hv = *hvalues; /*copy into register*/
		
		while (true) { /*unpack params*/
			
			typaramrecord param;
		
			if (!landsystem6unpackparam (hpacked, &ix, &param)) { /*no more params*/
				
				landdisposehandle (hpacked);
				
				return (true);
				}
				
			if (!landpushparamrecord (hv, param)) {
				
				landdisposehandle (hpacked);
				
				return (false);
				}
			} /*while*/
		} /*while*/
	} /*landsystem6send*/
	

static boolean landsystem6poll (hdlverbrecord *hverb) {

	/*
	1/1/91-3/x/91 mao: made networkable, other fixes (queued events)
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlverbrecord hv;
	register hdlsys6transportinfo ht;
	register boolean fl;
	Handle hpacked;	
	short senderid;

	*hverb = nil; /*default, no verb available*/
	
	if (!landpopqueue (&hpacked)) /*nothing waiting in the queue*/
		ipcReceive ((**hg).system6processid, &senderid, &hpacked);	
		
	if (hpacked == nil)
		return (true);
	
	fl = landsystem6unpackverb (hpacked, hverb);
	
	landdisposehandle (hpacked);
	
	if (!fl) /*error unpacking verb*/
		return (false);
	
	hv = *hverb; /*copy into register*/
	
	ht = (hdlsys6transportinfo) (**hv).transportinfo;
	
	(**ht).sys6processid = senderid; /*this guy gets the returned value*/
	
	return (true);
	} /*landsystem6poll*/


boolean landsystem6eventfilter (EventRecord *ev) {
	
	/*
	during null events, poll for incoming messages and process one 
	if available.
	
	return true if we consume the event, false otherwise.
	*/
	
	hdlverbrecord hverb;

	switch ((*ev).what) {
	
		case nullEvent:
			if (landsystem6poll (&hverb) && (hverb != nil)) { /*got on incoming message*/
				
				landhandleverb (hverb);
				
				return (true); /*consume the event*/
				}
		} /*switch*/
	
	return (false); /*don't consume the event*/
	} /*landsystem6eventfilter*/


static boolean landsystem6returnhandle (hdlverbrecord hverb, Handle hpacked) {

	register hdllandglobals hg = landgetglobals ();
	register hdlsys6transportinfo ht;
	register boolean fl;
	
	ht = (hdlsys6transportinfo) (**hverb).transportinfo;

	fl = ipcSend ((**hg).system6processid, (**ht).sys6processid, hpacked) == noErr;
	
	landdisposehandle (hpacked);
		
	return (fl);
	} /*landsystem6returnhandle*/
	

boolean landsystem6returnmultiple (hdlverbrecord hverb) {
	
	/*
	hverb is the verb we're responding to with a series of returned values which
	are linked into the params array of the verbrecord.
	*/
	
	register hdlverbrecord hv = hverb;
	register short ct = (**hv).nextparam;
	Handle hpacked;
	
	if (!landnewemptyhandle (&hpacked))
		return (false);
	
	if (!landsystem6packallparams (hv, hpacked)) {
	
		landdisposehandle (hpacked);
		
		return (false);
		}
	
	return (landsystem6returnhandle (hv, hpacked));
	} /*landsystem6returnmultiple*/
	
	
boolean landsystem6close (void) {
	
	register hdllandglobals hg = landgetglobals ();
	
	landemptyqueue ();
	
	return (ipcClose ((**hg).system6processid) == noErr);
	} /*landsystem6close*/
	

boolean landsystem6init (void) {

	register hdllandglobals hg = landgetglobals ();
	tysystem6processid id;
	bigstring bs;
	
	land4bytestostring ((**hg).applicationid, bs);
	
	if (ipcOpen ((PIN *) bs, &id) != noErr)
		return (false);
	
	(**hg).system6processid = id;	
	
	(**hg).hqueue = nil;
	
	return (true);
	} /*landsystem6init*/


boolean landsystem6setapplicationid (tyapplicationid newid) {

	/*
	allows toolkit user to change application id on the fly (e.g. 'ascr' to 
	'mao!')  this call flushes outstanding queued local and network requests.
	*/

	register hdllandglobals hg = landgetglobals ();
	bigstring bs;
	tysystem6processid newprocessid;
	
	land4bytestostring (newid, bs);
	
	if (ipcOpen ((PIN *) bs, &newprocessid) != noErr)
		return (false);

	if (ipcClose ((**hg).system6processid) != noErr) {
	
		ipcClose (newprocessid); /*get rid of the one we just made*/
		
		return (false);
		}
		
	landemptyqueue (); /*flush out queued requests for old id*/
	
	(**hg).system6processid = newprocessid;
	
	(**hg).applicationid = newid;

	return (true);
	} /*landsystem6setapplicationid*/
	
	
