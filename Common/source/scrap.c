
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

/*
scrap.c -- routines that support use of the desk scrap.
*/

#include "frontier.h"
#include "standard.h"

#include "shelltypes.h"
#include "scrap.h"
#include "error.h"
#include "memory.h"
#include "threads.h"


#ifdef WIN95VERSION

	static short scrapcount = 0;

	static UINT win_textscraptype;

	static UINT win_pictscraptype;

	static UINT win_hashscraptype;

	static UINT win_opscraptype;

	static UINT win_wpscraptype;

	static UINT win_menuscraptype;

	static UINT win_scriptscraptype;

#endif


boolean resetscrap (void) {
	//Code change by Timothy Paustian Sunday, June 25, 2000 11:05:43 AM
	//Carbon Support
	#ifdef MACVERSION
		#if TARGET_API_MAC_CARBON == 1
		return (ClearCurrentScrap() == noErr);
		#else
		return (ZeroScrap () == noErr);
		#endif
	#endif

	#ifdef WIN95VERSION
		return (EmptyClipboard());
	#endif
	} /*resetscrap*/


short getscrapcount (void) {

	/*
	2002-10-13 AR: Commented out variable declaration for theCount
	to eliminate compiler warning about an unused variable.
	*/
	
	#ifdef MACVERSION
		//Code change by Timothy Paustian Sunday, June 25, 2000 11:05:56 AM
		//Carbon Support
		#if TARGET_API_MAC_CARBON == 1
		
			ScrapRef 	theRef;
			//UInt32	 	theCount;
			static short theShortCount = 1;
			OSStatus	status;
			static ScrapRef lastScrapRef = nil;
			
			status = GetCurrentScrap(&theRef);
			
			if(status != noErr)
				return 0;
			
			assert(theRef != nil);
			
			if (theRef != lastScrapRef)
				theShortCount++;
				
			lastScrapRef = theRef;
			
			return (theShortCount);
			
			//status = GetScrapFlavorCount(theRef, &theCount);
			//if(status != noErr)
			//	return 0;
			
			//restrict the return value to a short size
			//theShortCount = min(theCount, 32000);
			//return theShortCount;
			
		#else
			
			PScrapStuff pscrap;
			
			pscrap = InfoScrap ();
			
			return ((*pscrap).scrapCount);
			
		#endif
		
	#endif

	#ifdef WIN95VERSION
		return (scrapcount);
	#endif
	} /*getscrapcount*/


#ifdef WIN95VERSION

short handlescrapdisposed (void) {
	
	return (++scrapcount);
	} /*handlescrapdisposed*/


tyscraptype win2shellscraptype (UINT winscraptype) {

	if (winscraptype == 0)
		return (0);

	if (winscraptype == win_textscraptype)
		return (textscraptype);

	if (winscraptype == win_pictscraptype)
		return (pictscraptype);

	if (winscraptype == win_opscraptype)
		return (opscraptype);

	if (winscraptype == win_scriptscraptype)
		return (scriptscraptype);

	if (winscraptype == win_wpscraptype)
		return (wpscraptype);

	if (winscraptype == win_hashscraptype)
		return (hashscraptype);

	if (winscraptype == win_menuscraptype)
		return (menuscraptype);

	return (noscraptype);
	} /*win2shellscraptype*/


UINT shell2winscraptype (tyscraptype scraptype) {

	switch (scraptype) {
	
		case textscraptype:
			return win_textscraptype;
		
		case pictscraptype:
			return win_pictscraptype;
		
		case hashscraptype:
			return win_hashscraptype;
		
		case opscraptype:
			return win_opscraptype;
		
		case wpscraptype:
			return win_wpscraptype;
		
		case menuscraptype:
			return win_menuscraptype;
		
		case scriptscraptype:
			return win_scriptscraptype;
		}

	return 0;
	} /*shell2winscraptype*/

#endif


tyscraptype getscraptype (void) {
	
	/*
	return the type of the first object on the scrap.
	*/
	
	#ifdef MACVERSION
		//Code change by Timothy Paustian Sunday, June 25, 2000 11:06:10 AM
		//Carbon support for new Scrap Manager API
		#if TARGET_API_MAC_CARBON == 1
		ScrapRef		scrap;
		UInt32			infoNumber = 1;
    	ScrapFlavorInfo info[1];
    	OSStatus		status;
    	
    	status = GetCurrentScrap(&scrap);
    	if(status != noErr)
			return 0;
		//now get the first item on the list
		status = GetScrapFlavorInfoList(scrap, &infoNumber, info);
		if(status != noErr)
			return 0;
		
		return (tyscraptype)info[1].flavorType;
		#else
		
		PScrapStuff pscrap;
		OSType **htype;
		
		pscrap = InfoScrap ();
		
		htype = (OSType **) (*pscrap).scrapHandle;
		
		if (htype == nil)
			return (0);
		
		return (**htype);
		#endif
		
	#endif

	#ifdef WIN95VERSION
		UINT format = 0;

		while (true) {

			format = EnumClipboardFormats (format);

			if (format == 0)
				return (0);

			if (format == win_textscraptype)
				return (textscraptype);

			if (format == win_pictscraptype)
				return (pictscraptype);

			if (format == win_opscraptype)
				return (opscraptype);

			if (format == win_scriptscraptype)
				return (scriptscraptype);

			if (format == win_wpscraptype)
				return (wpscraptype);

			if (format == win_hashscraptype)
				return (hashscraptype);

			if (format == win_menuscraptype)
				return (menuscraptype);
			}
	#endif
	} /*getscraptype*/


boolean getscrap (tyscraptype scraptype, Handle hscrap) {
	
	/*
	return true if something was loaded from the scrap, false 
	otherwise.
	
	12/26/91 dmb: return true if scraptype exists; generate error alert on real error.

	5.0a16 dmb: need to scan more than last 32 bytes for NUL char
	*/
	
	#ifdef MACVERSION
#if TARGET_API_MAC_CARBON == 1
		ScrapRef			theScrap;
	    ScrapFlavorType 	flavorType = (ScrapFlavorType)scraptype;
	    Size 				byteCount;
	    OSStatus			status;
	    ScrapFlavorFlags	flavorFlags;
		boolean				retVal = true;
		Size				prevCount;

	    status = GetCurrentScrap(&theScrap);
	    if(status != noErr)
			oserror (status);

		status = GetScrapFlavorFlags(theScrap, flavorType, &flavorFlags);
		if(status == noTypeErr)
		{
			retVal = false;
		}
		else if(status < 0)
		{
			retVal = false;
			oserror (status);
		}
		//if we get here and the handles nil, we really don't want the data
		//also if retVal is false, it means the type does not exist
		if(hscrap == nil || !retVal)
			return retVal;

		status = GetScrapFlavorSize(theScrap, flavorType, &byteCount);
		if(status != noErr)
			oserror (status);
		prevCount = byteCount;
		SetHandleSize(hscrap, byteCount);
		if (MemError () != noErr)
			return (false);
		//lock the handle before getting the data
		HLock(hscrap);
	    status = GetScrapFlavorData(theScrap, flavorType, &byteCount, *(hscrap));
	    HUnlock(hscrap);

	    if(status != noErr)
			oserror (status);
		//only return true if we got all the data.
		return (byteCount == prevCount);

		#else //precarbon mac		
		long result;
		long offset;
		
		result = GetScrap (hscrap, scraptype, &offset);
		
		if (result == noTypeErr)
			return (false);
		
		if (result < 0)
			oserror (result);

		return (true); /*there was something on the scrap of the given type*/
		#endif
		
	#endif
	
	#ifdef WIN95VERSION
		Handle hdata;
		long ctbytes;
		char *pdata;
		boolean fl;
		/*long i;*/
		UINT wintype;
		
		wintype = shell2winscraptype (scraptype);
		
		if (wintype == 0)
			return (false);
		
		releasethreadglobals (); //getting clipboard data can send us a WM_RENDERFORMAT msg
		
		hdata = GetClipboardData (wintype);
		
		grabthreadglobals ();
		
		if (hdata == NULL) {
			
			oserror (GetLastError ()); // may be no error
			
			return (false);
			}
		
		ctbytes = GlobalSize (hdata);
		
		pdata = GlobalLock (hdata);
		
		if (pdata == NULL)
			return (false);
		
		if (scraptype == textscraptype) {		//Handle reducing the scrap size to the NULL
			/* i = 0x40;
			 if (i > ctbytes)
				 i = ctbytes;

			 while (i > 0) {
				 if (*(pdata + ctbytes - i) == 0) {
					 ctbytes = ctbytes - i;
					 break;
					 }

				 --i;
				 } /*while*/			

			ctbytes = strlen (pdata); /*7.1b23: call strlen to find the first terminator: removes junk from end*/
			} /*if*/

		fl = sethandlecontents (pdata, ctbytes, hscrap);
		
		GlobalUnlock (hdata);
		
		return (fl);
	#endif
	} /*getscrap*/


boolean putscrap (tyscraptype scraptype, Handle hscrap) {
	
	/*
	return true if something was put on the scrap, false 
	otherwise.

	5.0a16 dmb: handle NULL scrap for delayed rendering

	5.0b13 dmb: report errors (win)
	*/

	boolean fl;

	#ifdef MACVERSION
		//Code change by Timothy Paustian Sunday, June 25, 2000 11:28:22 AM
		//New scrap code. We don't need flavorFlags I think
		#if TARGET_API_MAC_CARBON == 1
		ScrapRef			theScrap;
		OSStatus			status;
	    ScrapFlavorType 	flavorType = (ScrapFlavorType) scraptype;
	    ScrapFlavorFlags 	flavorFlags = kScrapFlavorMaskNone;
		Size				flavorSize = GetHandleSize(hscrap);
		
		status = GetCurrentScrap(&theScrap);
		if(status != noErr)
			return false;
		HLock(hscrap);
		fl = (PutScrapFlavor (theScrap, flavorType, flavorFlags, flavorSize, *hscrap) == noErr);
    	HUnlock(hscrap);
    	return fl;
    	
		#else
		
		HLock (hscrap);
		
		fl = PutScrap (GetHandleSize (hscrap), scraptype, *hscrap) == noErr;
		
		HUnlock (hscrap);

		return (fl);
		#endif
		
	#endif

	#ifdef WIN95VERSION
		long ctbytes;
		long allocctbytes;
		Handle hdata;
		char *pdata;
		UINT wintype;
		
		wintype = shell2winscraptype (scraptype);

		if (wintype == 0) {
	
			oserror (DV_E_CLIPFORMAT);

			return (false);
			}
		
		if (hscrap == NULL)
			hdata = NULL;
		
		else {
			ctbytes = gethandlesize (hscrap);

			allocctbytes = ctbytes;

			if (scraptype == textscraptype)
				++allocctbytes;							//Allow for NULL termination

			hdata = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE, allocctbytes);
			pdata = GlobalLock (hdata);
			
			if (pdata == NULL) {

				GlobalFree (hdata);
				
				return (false);
				}

			moveleft (*hscrap, pdata, ctbytes);
			
			if (scraptype == textscraptype)
				*(pdata + ctbytes) = 0;			//NULL terminate TEXT scraps

			//Window handles are not exact size - NULL fill remaining space for safety
			if ((GlobalSize (hdata) - allocctbytes) > 0)
				clearbytes (pdata + allocctbytes, GlobalSize (hdata) - allocctbytes);

			GlobalUnlock (hdata);
			}
		
		SetLastError (0);
		
		fl = (SetClipboardData (wintype, hdata) != NULL);
		
		if (!fl)
			fl = !oserror (GetLastError ());
		
		return (fl);
	#endif
	} /*putscrap*/


boolean openclipboard (void) {

	#ifdef MACVERSION
		return (true);
	#endif

	#ifdef WIN95VERSION
		return (OpenClipboard (shellframewindow));
	#endif
	} /*openclipboard*/


boolean closeclipboard (void) {

	#ifdef MACVERSION
		return (true);
	#endif

	#ifdef WIN95VERSION
		boolean fl;

		releasethreadglobals (); //closing clipboard can send us a WM_DRAWCLIPBOARD msg
		
		fl = CloseClipboard ();
		
		grabthreadglobals ();

		return (fl);
	#endif
	} /*closeclipboard*/


void initclipboard (void) {

	#ifdef WIN95VERSION
		win_textscraptype = CF_TEXT;

		win_pictscraptype = CF_METAFILEPICT;
		
		win_hashscraptype = RegisterClipboardFormat ("Frontier_HASH");

		win_opscraptype = RegisterClipboardFormat ("Frontier_OP");

		win_wpscraptype = RegisterClipboardFormat ("Frontier_WPTX");

		win_menuscraptype = RegisterClipboardFormat ("Frontier_MNBR");

		win_scriptscraptype = RegisterClipboardFormat ("Frontier_SCPT");
	#endif
	} /*initclipboard*/







