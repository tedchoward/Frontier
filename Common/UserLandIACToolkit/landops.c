
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

#ifdef	MPWC

	#include <strings.h>
	
#endif


#ifndef fljustfrontier

boolean landlockhandle (Handle hlock) {
	
	register Handle h = hlock;
	
	if (h == nil)
		return (false);
		
	HLock (h);
	
	return (true);
	} /*landlockhandle*/
	
	
boolean landunlockhandle (Handle hunlock) {
	
	register Handle h = hunlock;
	
	if (h == nil)
		return (false);
		
	HUnlock (h);
	
	return (true);
	} /*landunlockhandle*/


landmoveleft (void *psource, void *pdest, long length) {
	
	/*
	do a mass memory move with the left edge leading.  good for closing
	up a gap in a buffer, among other thingsÉ
	*/
	
	register byte *ps, *pd;
	register long ctloops;
	
	ctloops = length;
	
	if (ctloops > 0) {
	
		ps = psource; /*copy into a register*/
	
		pd = pdest; /*copy into a register*/
	
		while (ctloops--) *pd++ = *ps++;
		}
	} /*landmoveleft*/
	
	
static landfillchar (void *pfill, long ctfill, byte chfill) {
	
	/*
	do a mass memory fill -- copy ctfill chfills at pfill.
	*/
	
	register byte *p = pfill;
	register long ct = ctfill;
	register byte ch = chfill;
	
	while (ct--) *p++ = ch; /*tight loop*/
	} /*landfillchar*/
	

landclearbytes (void *pclear, long ctclear) {
	
	/*
	fill memory with 0's.
	*/
	
	landfillchar (pclear, ctclear, (byte) 0);
	} /*landclearbytes*/
	

land4bytestostring (long bytes, bigstring bs) {
	
	setstringlength (bs, 4);
	
	landmoveleft (&bytes, &bs [1], 4L);
	} /*land4bytestostring*/


landsetcursortype (short newcursor) {

	register CursHandle hcursor;

	if (newcursor == 0) {
		
		SetCursor (&quickdrawglobal (arrow));

		return;
		}

	hcursor = GetCursor (newcursor);
	
	if (hcursor != nil)
		SetCursor (*hcursor);
	} /*landsetcursortype*/


landcopystring (bssource, bsdest) bigstring bssource, bsdest; {

	/*
	create a copy of bssource in bsdest.  copy the length byte and
	all the characters in the source string.

	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	register short i, len;
	
	len = (short) bssource [0];
	
	for (i = 0; i <= len; i++) 
		bsdest [i] = bssource [i];
	} /*landcopystring*/


landcopyheapstring (hdlbigstring hsource, bigstring bsdest) {
	
	landcopystring (*hsource, bsdest);
	} /*landcopyheapstring*/


boolean landpushstring (bigstring bssource, bigstring bsdest) {

	/*
	insert the source string at the end of the destination string.
	
	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	register short lensource = stringlength (bssource);
	register short lendest = stringlength (bsdest);
	register byte *psource, *pdest;
	
	if ((lensource + lendest) > lenbigstring)
		return (false);
		
	pdest = (byte *) bsdest + lendest + 1;
	
	psource = (byte *) bssource + 1;
	
	bsdest [0] += (byte) lensource;
	
	while (lensource--) *pdest++ = *psource++;

	return (true);
	} /*landpushstring*/
	
	
boolean landpushlong (long x, bigstring bs) {
	
	bigstring bslong;
	
	NumToString (x, bslong);
	
	return (landpushstring (bslong, bs));
	} /*landpushlong*/
	

boolean landequalstrings (bigstring bs1, bigstring bs2) {

	/*
	return true if the two strings (pascal type, with length-byte) are
	equal.  return false otherwise.
	*/
	
	register ptrbyte p1 = (ptrbyte) bs1, p2 = (ptrbyte) bs2;
	register short ct = *p1 + 1;
	
	while (ct--) 
		
		if (*p1++ != *p2++)
		
			return (false);
		
	return (true); /*loop terminated*/
	} /*landequalstrings*/


boolean landnewfilledhandle (void *pdata, long size, Handle *hnew) {
	
	/*
	create a new handle of the indicated size, filled with the indicated data.
	
	return false if the allocation failed.
	*/
	
	register Handle h;

	h = *hnew = NewHandle (size);

	if (h == nil)
		return (false);

	landmoveleft (pdata, *h, size);
	
	return (true);
	} /*landnewfilledhandle*/
	

landdisposehandle (Handle h) {
	
	/*
	our own bottleneck for this built-in.  we don't require type coercion
	and we check if its nil.
	*/
	
	if (h != nil)
		DisposHandle (h);
	} /*landdisposehandle*/


hdlbigstring landnewstring (bigstring bs) {
	
	return (NewString (bs));
	} /*landnewstring*/
	

boolean landenlargehandle (Handle hgrow, long ctgrow, void *newdata) {
	
	/*
	make the handle big enough to hold the new data, and move the new data in
	at the end of the newly enlarged handle.
	*/
	
	register Handle h = hgrow;
	register long ct = ctgrow;
	register ptrbyte p = newdata;
	register long origsize;
	
	origsize = GetHandleSize (h);
	
	SetHandleSize (h, origsize + ct);
		
	if (MemError () != noErr)
		return (false);
		
	landmoveleft (p, *h + origsize, ct);
	
	return (true);
	} /*landenlargehandle*/ 
	
	
boolean landshrinkhandle (Handle h, long ct) {
	
	/*
	make the handle smaller -- by ct bytes.
	*/
	
	SetHandleSize (h, GetHandleSize (h) - ct);
	
	return (MemError () == noErr);
	} /*landshrinkhandle*/
	
	
boolean landloadfromhandle (hload, ixload, ctload, pdata) Handle hload; long *ixload, ctload; void *pdata; {
	
	/*
	copy the next ctload bytes from hload into pdata and increment the index.
	
	return false if there aren't enough bytes.
	
	start ixload at 0.
	*/
	
	register Handle h = hload;
	register ptrbyte p = pdata;
	register long ct = ctload;
	register long ix = *ixload;
	register long size;
	
	size = GetHandleSize (h);
	
	if ((ix + ct) > size) /*asked for more bytes than there are*/
		return (false); 
		
	landmoveleft (*h + ix, p, ct); /*copy out of the handle*/
	
	*ixload = ix + ct; /*increment the index into the handle*/
	
	return (true);
	} /*landloadfromhandle*/


boolean landloadfromhandletohandle (Handle hload, long *ixload, long ctload, Handle *hnew) {
	
	/*
	load from the source handle, creating a new handle to hold the loaded stuff.
	*/
	
	register Handle h;
	
	h = NewHandle (ctload);

	if (h == nil)
		return (false);
	
	if (!landloadfromhandle (hload, ixload, ctload, *h)) {
		
		landdisposehandle (h);

		return (false);
		}
	
	*hnew = h;
	
	return (true);
	} /*landloadfromhandletohandle*/


boolean landcopyhandle (horig, hcopy) Handle horig, *hcopy; {
	
	register Handle h;
	register long ctbytes;
	
	ctbytes = GetHandleSize (horig);
	
	h = NewHandle (ctbytes);
	
	if (h == nil)
		return (false);
		
	landmoveleft (*horig, *h, ctbytes);
	
	*hcopy = h;
	
	return (true);
	} /*landcopyhandle*/


boolean landnewclearhandle (long ctbytes, Handle *hreturned) {
	
	register long ct = ctbytes;
	register Handle h;
	
	*hreturned = h = NewHandle (ct);
	
	if (h == nil) 
		return (false);
		
	landclearbytes (*h, ct);
	
	*hreturned = h;
	
	return (true);
	} /*landnewclearhandle*/


long landgethandlesize (Handle h) {
	
	return (GetHandleSize (h));
	} /*landgethandlesize*/
	
	
boolean landnewemptyhandle (Handle *h) {
	
	return (landnewclearhandle (0L, h));
	} /*landnewemptyhandle*/


boolean landsurrenderprocessor (EventRecord *ev) {
	
	WaitNextEvent (nullEvent, ev, 1, nil);
	
	return (true);
	} /*landsurrenderprocessor*/

#endif

boolean landbreakembrace (EventRecord *ev) {
	
	register hdllandglobals hg = landgetglobals ();
	
	return ((*(**hg).breakembraceroutine) (ev));
	
	/*
	return (landnoparamcallback ((**hg).breakembraceroutine));
	*/
	} /*landbreakembrace*/


boolean landgetappcreator (OSType *creator) {
	
	/*
	get the 4-character creator identifier for the application we're running 
	inside of.
	*/
	
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	
	#ifdef flnewfeatures
	
		psn.highLongOfPSN = 0;
		
		psn.lowLongOfPSN =  kCurrentProcess;

	#else
	
		GetCurrentProcess (&psn);
	
	#endif
	
	info.processInfoLength = (long) sizeof (info);
	
	info.processName = nil;
	
	info.processAppSpec = nil;
	
	GetProcessInformation (&psn, &info);
	
	*creator = info.processSignature;
	
	return (true);
	} /*landgetappcreator*/




