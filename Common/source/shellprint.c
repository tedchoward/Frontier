
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

#include "memory.h"
#include "cursor.h"
#include "error.h"
#include "kb.h"
#include "quickdraw.h"
#include "strings.h"
#include "shell.h"
#include "shellprivate.h"
#include "shell.rsrc.h"
#include "shellprint.h"


#ifdef MACVERSION
	CGrafPtr currentprintport = NULL;
	#if TARGET_API_MAC_CARBON
		#define iPrAbort kPMCancel
	#endif
#endif

#ifdef WIN95VERSION
HWND currentprintport = NULL;
#endif


typrintinfo shellprintinfo;




/*
static shelldisposeprintinfo (void) {

	disposehandle ((Handle) shellprintinfo.printhandle);
	
	clearbytes (&shellprintinfo, longsizeof (shellprintinfo));
	} /%shelldisposeprintinfo%/
*/

#if defined(WIN95VERSION) || (defined(MACVERSION) && !TARGET_API_MAC_CARBON)

static boolean shellcheckprinterror (boolean flopening) {
	
	/*
	returns false if there was a print error.
	
	1/18/93 dmb: take flopening parameter, & special case this error
	*/
	
	OSErr x;
	bigstring bserror;
	
#ifdef MACVERSION
	//Code change by Timothy Paustian Friday, June 16, 2000 3:50:52 PM
	//Changed to Opaque call for Carbon
	#if TARGET_API_MAC_CARBON == 1
	x = PMSessionError(shellprintinfo.printhandle);
	#else	
	x = PrError ();
	#endif
		
	if (x == noErr) /*no error, keep going*/
		return (true);
	
	if (x != iPrAbort) {
		
		if (flopening) 
			GetIndString (bserror, interfacelistnumber, trychooserstring);
			
		else {
			
			getsystemerrorstring (x, bserror);
			
			parsedialogstring (bserror, "\x06" ".Print", nil, nil, nil, bserror);
			}
		
		shellerrormessage (bserror);
		}
#endif

#ifdef WIN95VERSION
	x = GetLastError();

	if (x == 0)
		return (true);

	getsystemerrorstring (x, bserror);

	shellerrormessage (bserror);
#endif
	
	return (false); /*there was an error*/
	} /*shellcheckprinterror*/

#endif


static void shellcopyprintinfo (void) {
	
	/*
	set up our printing rect according to the print manager's rects.
	
	we'll use the smaller of the printable area (rPage) and the physical 
	paper (rPaper) with 1/2 inch margins

	5.0.1 dmb: measure in pixels (1/72 inch) for Win
	*/
	
	Rect rpage;
	Rect rpaper;
	
	#ifdef MACVERSION
		#if TARGET_API_MAC_CARBON == 1
		{
		OSErr 			theErr;
		PMPageFormat	pageformat = 0;
		boolean			fl;
		theErr = PMSessionValidatePageFormat(shellprintinfo.printhandle, pageformat, &fl);
		if(theErr != noErr || !fl)
			oserror(theErr);
		
		PMGetAdjustedPageRect(shellprintinfo.pageformat, (PMRect *)&rpage);
			
		PMGetAdjustedPaperRect(shellprintinfo.pageformat, (PMRect *)&rpaper);
		shellprintinfo.pagerect = rpage;
		}
		#else
		
		rpage = (**shellprintinfo.printhandle).prInfo.rPage;
		
		rpaper = (**shellprintinfo.printhandle).rPaper;
		#endif
	#endif

	#ifdef WIN95VERSION
		long res = 72L;

		pushport (hwndMDIClient);
		res = GetDeviceCaps(getcurrentDC (), LOGPIXELSX);
		popport ();

		rpaper.left = 0;
		rpaper.top = 0;
		rpaper.right = shellprintinfo.pagesetupinfo.ptPaperSize.x * res / 1000L;
		rpaper.bottom = shellprintinfo.pagesetupinfo.ptPaperSize.y * res / 1000L;

		rpage.left = rpaper.left + (shellprintinfo.pagesetupinfo.rtMinMargin.left * res / 1000L);
		rpage.top = rpaper.top + (shellprintinfo.pagesetupinfo.rtMinMargin.top * res / 1000L);
		rpage.right = rpaper.right - (shellprintinfo.pagesetupinfo.rtMinMargin.right * res / 1000L);
		rpage.bottom = rpaper.bottom - (shellprintinfo.pagesetupinfo.rtMinMargin.bottom * res / 1000L);

		shellprintinfo.margins.left = shellprintinfo.pagesetupinfo.rtMargin.left * res / 1000L;
		shellprintinfo.margins.top = shellprintinfo.pagesetupinfo.rtMargin.top * res / 1000L;
		shellprintinfo.margins.right = shellprintinfo.pagesetupinfo.rtMargin.right * res / 1000L;
		shellprintinfo.margins.bottom = shellprintinfo.pagesetupinfo.rtMargin.bottom * res / 1000L;
	#endif
	
	shellprintinfo.paperrect.top = max (rpage.top, rpaper.top + shellprintinfo.margins.top);
	
	shellprintinfo.paperrect.left = max (rpage.left, rpaper.left + shellprintinfo.margins.left);
	
	shellprintinfo.paperrect.bottom = min (rpage.bottom, rpaper.bottom - shellprintinfo.margins.bottom);
	
	shellprintinfo.paperrect.right = min (rpage.right, rpaper.right - shellprintinfo.margins.right);
	} /*shellcopyprintinfo*/


boolean shellinitprint (void) {
	
	/*
	9/5/90 dmb: close print resources after initializing stuff
	
	10/21/91 dmb: added margins field to print info; structure is now here to 
	have user-settable margins.
	
	12/31/91 dmb: initialize shellprintinfo.paperrect to standard 72dpi values in 
	case no printer is chosen and shellcopyprintinfo never gets called
	
	1/18/93 dmb: don't call shellcheckprinterror the first time; if PrOpen fails 
	here, we don't want to raise an alert.
	*/
	
	#if MACVERSION && !TARGET_API_MAC_CARBON
		Handle h;
	#endif
	
	currentprintport = NULL;

	clearbytes (&shellprintinfo, longsizeof (shellprintinfo));
	
	setrect (&shellprintinfo.margins, 36, 36, 36, 36);
	
	setrect (&shellprintinfo.paperrect, 5, 6, 725, 546); /*defaults in case PrOpen fails*/

	shellprintinfo.scaleMult = 1;
	shellprintinfo.scaleDiv = 1;

	#ifdef MACVERSION
		
		#if TARGET_API_MAC_CARBON == 1
		//I realized this is only called once during the startup of the app.
		//Carbon printing really doesn't need any global structures. Better to 
		//allocate them as we use them.
		#else		
		if (!newclearhandle (longsizeof (TPrint), &h))
			return (false);
		
		shellprintinfo.printhandle = (THPrint) h; /*copy into print record*/
		
		PrOpen (); /*initialize the Mac print manager*/
		
		
		if (PrError () != noErr)
			goto error;
		
		PrintDefault (shellprintinfo.printhandle); /*set default print record*/
		
		PrClose (); /*shouldn't leave print resources open all the time*/
		
		if (!shellcheckprinterror (false)) 
			goto error;
		#endif
	#endif

	#ifdef WIN95VERSION
		ZeroMemory (&shellprintinfo.pagesetupinfo, sizeof (PAGESETUPDLG));
		shellprintinfo.pagesetupinfo.lStructSize = sizeof (PAGESETUPDLG);
		shellprintinfo.pagesetupinfo.hwndOwner = NULL;
		shellprintinfo.pagesetupinfo.Flags = PSD_RETURNDEFAULT | PSD_NOWARNING;
		if (! PageSetupDlg (&shellprintinfo.pagesetupinfo)) {
			if (CommDlgExtendedError() != 0)
				goto error;
			}
	#endif
	
	#if !TARGET_API_MAC_CARBON
	shellcopyprintinfo (); /*copies fields from handle into record*/
	#endif

	return (true);
	
#if !defined(MACVERSION) || !TARGET_API_MAC_CARBON
	error:
#endif
	
	/*
	shelldisposeprintinfo ();
	*/
	
	return (false);
	} /*shellinitprint*/


static boolean shellpagesetupvisit (WindowPtr w, ptrvoid refcon) {

	shellpushglobals (w);
	
	(*shellglobals.pagesetuproutine) ();
	
	shellpopglobals ();
	
	return (true);
	} /*shellpagesetupvisit*/


boolean shellpagesetup (void) {
	
	/*
	9/5/90 dmb:  open and close print resources each time we're called
	
	9/27/91 dmb: don't do anything when the user cancels the dialog
	*/
	
	boolean fl;
	
#ifdef MACVERSION
	//Code change by Timothy Paustian Friday, June 16, 2000 10:08:37 PM
	//new code for the new print manager.
	#if TARGET_API_MAC_CARBON == 1
	{
	OSStatus status;
	//validate the page format record.
	status = PMSessionValidatePageFormat(shellprintinfo.printhandle,
				(PMPageFormat) shellprintinfo.pageformat,
				kPMDontWantBoolean);
	// Display the Page Setup dialog
	if (status == noErr)
	{
		status = PMSessionPageSetupDialog(shellprintinfo.printhandle, 
						(PMPageFormat) shellprintinfo.pageformat, &fl);
		if (!fl)
			status = kPMCancel; // user clicked Cancel button
	}
	
	if(status != noErr)
		return false;
	
	}
	#else
		
	PrOpen ();
	
	if (!shellcheckprinterror (true)) 
		return (false);
	
	PrValidate (shellprintinfo.printhandle);
	
	fl = PrStlDialog (shellprintinfo.printhandle);
	
	PrClose ();
	
	if (!shellcheckprinterror (false))
		return (false);
	#endif
		
#endif
	
#ifdef WIN95VERSION
	shellprintinfo.pagesetupinfo.Flags = PSD_MARGINS | PSD_INTHOUSANDTHSOFINCHES;

	shellprintinfo.pagesetupinfo.hwndOwner = hwndMDIClient;

	fl = PageSetupDlg (&shellprintinfo.pagesetupinfo);
#endif

	if (!fl)
		return (false);
	
	//what does this code do?
	shellcopyprintinfo (); /*copies fields from handle into record*/
	
	shellvisittypedwindows (-1, &shellpagesetupvisit, nil); /*visit all windows*/
	
	return (true);
	} /*shellpagesetup*/


/*
boolean shellsetprintmargins (Rect newmargins) {
	
	/%
	10/21/91 dmb: we may need this some day [never tested]
	%/
	
	PrOpen ();
	
	if (!shellcheckprinterror (true))
		return (false);
	
	PrValidate (shellprintinfo.printhandle);
	
	PrClose ();
	
	shellprintinfo.margins = newmargins;
	
	shellcopyprintinfo (); /%copies fields from handle into record%/
	
	shellvisittypedwindows (-1, &shellpagesetupvisit, nil); /%visit all windows%/
	
	return (true);
	} /%shellsetprintmargins%/


void printresetrects (hdlwindowinfo hinfo, Rect * r) {
	
	/%
	(re)initialize all of the rectangles stored in the window's information 
	record.  start by clearing the buttons rect and setting the content rect 
	to be the owning window's portrect.  the resetwindowrects callback routine 
	is responsible for:
		1.  adjust the contentrect, if necessary
		2.  setting the buttons rect, if used
		3.  positioning any scrollbars
		4.  setting up the grow box rect
	%/
	
	register hdlwindowinfo h = hinfo;
	
	// (**h).contentrect = (*(**h).macwindow).portRect;
	
	(**h).contentrect = *r;
	
	zerorect (&(**h).buttonsrect);
	
	(*shellglobals.resetrectsroutine) (h);
	
	(*shellglobals.resizeroutine) ();
	} /%printresetrects%/
*/


boolean iscurrentportprintport (void) {
	
	return (isprintingactive () && (currentprintport == getport()));
	} /*iscurrentportprintport*/


boolean isprintingactive (void) {
	
	return (currentprintport != NULL);
	} /*isprintingactive*/


boolean getprintscale (long * scaleMult, long * scaleDiv) {

	*scaleMult = shellprintinfo.scaleMult;

	*scaleDiv = shellprintinfo.scaleDiv;

	return (true);
	} /*getprintscale*/


boolean shellprint (WindowPtr w, boolean fldialog) {
	
	/*
	9/5/90 dmb:  open and close print resources each time we're called.  also, 
	make sure we close each page opened, even when errors occur
	
	9/28/91 dmb: for exit of edit mode before printing
	
	4/24/92 dmb: make sure bJDocLoop is bSpoolLoop before calling PrPicFile, as 
	per IM II-155.  otherwise, we can get bogus PrErrors
	*/

	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, June 19, 2000 11:13:21 AM
		//Changed to Opaque call for Carbon
		#if TARGET_API_MAC_CARBON == 1
			OSStatus 		theErr;
			PMPrintSession 	hp = 0;			// JES 9.0.1: initialize to 0 -- printing still doesn't work on OS X, but at least Cmd-P won't crash //
			PMPageFormat	pageformat;
			PMPrintSettings	printsettings = nil;
			boolean			accepted;
			//remember that this is a grafport, TPPrPort isn't
			//but this should work because all the code that
			//messes with printing uses a grafport. 
			CGrafPtr			printport;
			UInt32 minPage = 1,
			maxPage = 9999;
		#else
			TPPrPort printport;
			TPrStatus printstatus;
			register THPrint hp = shellprintinfo.printhandle;
		#endif
		SInt32	firstPage = 1;
		SInt32	lastPage = 9999;
	#endif

	#ifdef WIN95VERSION
		PRINTDLG pd;
		DOCINFO di;
		HDC printport;
		hdlwindowinfo hinfo;
	#endif

	short i;
	register boolean fl = false;
	
	if (w == nil) /*defensive driving*/
		return (false);

	#ifdef MACVERSION	
		//Code change by Timothy Paustian Friday, June 16, 2000 9:11:56 PM
		//Changed to Opaque call for Carbon
		#if TARGET_API_MAC_CARBON == 1
		{
		//I may need to save back the graf port.
		theErr = PMCreateSession(&hp); // 2004-10-31 aradke: this was just hp instead of &hp -- not sure what's up!?
		 //This should cover it, but I may need to add a further check.
		if(theErr != noErr)
			return false;
		}
		#else
		PrOpen ();
		if (!shellcheckprinterror (true))
			return (false);
		#endif
	#endif
	
	fl = false; /*until sucessfull print, this is return value*/
	
	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, June 19, 2000 11:21:53 AM
		//Changed to Opaque call for Carbon
		//Here we create the data structures for the print job.
		#if TARGET_API_MAC_CARBON == 1
		//first get the page set up information, this may have been set up
		//previously by a call to the page set up dialog, otherwise, defaults are used.
		theErr = PMCreatePageFormat(&pageformat);
		// Note that PMPageFormat is not session-specific, but calling
		// PMSessionDefaultPageFormat assigns  values specific to the printer
		// associated with the current printing session
		if(theErr != noErr || (pageformat == kPMNoPageFormat)){
			PMRelease(hp);
			return false;
		}
		
		theErr = PMSessionDefaultPageFormat(hp, pageformat);
		if(theErr != noErr)
		{
			PMRelease(hp);
			PMRelease(pageformat);
			return false;
		}
		
		
		//now display the print dialog to get printing parameters from the user.	
		if(fldialog) {
			theErr = PMCreatePrintSettings(&printsettings);
			if(theErr != noErr || (printsettings == kPMNoPrintSettings)){
				PMRelease(hp);
				PMRelease(pageformat);
				return false;
			}
			//fill it with default values reguardless of what happens next.
			theErr = PMSessionDefaultPrintSettings(hp, printsettings);
		
		}
		else
		{
			theErr = PMSessionValidatePrintSettings(hp, printsettings,
									kPMDontWantBoolean);
		}
		
		if(theErr != noErr){
			//we can start jumping to exit now since all the memory is allocated.
			goto exit;
		}
		
		
		//set the page range
		theErr = PMSetPageRange(printsettings, minPage, maxPage);
		if(theErr != noErr){
			goto exit;
		}
		
		//finally display the dialog
		theErr = PMSessionPrintDialog(hp, printsettings, pageformat,
												&accepted);
		//either the user canceled or their was some other error.
		//don't print
		if (!accepted || (theErr != noErr)){
			goto exit;
		}
			
		//if we are to here then we have pageformat and printsetting and are
		//ready to print. Save away the important info in the shellprintinfo data structure
		shellprintinfo.printhandle = hp;
		shellprintinfo.pageformat = pageformat;
		shellprintinfo.printsettings = printsettings;
		#else
		
		if (fldialog) {
			if (!PrJobDialog (hp))
				goto exit;
			}
		else
			PrValidate (hp);
		#endif
		
	#endif

	#ifdef WIN95VERSION
		ZeroMemory (&pd, sizeof (PRINTDLG));
		pd.lStructSize = sizeof (PRINTDLG);
		pd.hwndOwner = hwndMDIClient;
		pd.Flags = PD_RETURNDC;

		if (!fldialog)
			pd.Flags = pd.Flags | PD_RETURNDEFAULT;

		if (! PrintDlg (&pd)) {
			goto exit;
			}
	#endif
	
	setcursortype (cursoriswatch);
	
	shellupdateallnow (); /*update all windows that were dirtied by the print dialog*/
	
	shellpushglobals (w);
	
	(*shellglobals.settextmoderoutine) (false); /*make sure editing changes are accepted*/
	
	#ifdef WIN95VERSION
		di.cbSize = sizeof(DOCINFO);
		di.lpszDocName = "Frontier Document";
		di.lpszOutput = NULL;
		di.lpszDatatype = NULL;
		di.fwType = 0;

		shellprintinfo.printport = printport = pd.hDC;
		shellprintinfo.scaleMult = GetDeviceCaps(pd.hDC, LOGPIXELSX);
		shellprintinfo.scaleDiv = GetDeviceCaps(getcurrentDC (), LOGPIXELSX);

		StartDoc (printport, &di);
		winpushport (w, printport);
		currentprintport = w;

		getwindowinfo (w, &hinfo);
	#endif
	
	#ifdef MACVERSION
		pushport (nil); /*save current port on stack*/
		//Code change by Timothy Paustian Friday, June 16, 2000 4:19:04 PM
		//Changed to Opaque call for Carbon
		//I will have to watch out for this,
		//to save the port I need to use..
		//PMSessionGetGraphicsContext (
		//This does not return the printport, but TPrPort gPort field
		//I think I can get away with this because the only function that uses
		//it is pgInitDevice and that assumes you are passing a port.
		//It is OK in the old code because the first item in the struct is a port
		#if TARGET_API_MAC_CARBON == 1 
		//we have to call this first before we can get the graphics context
		theErr = PMSessionBeginDocument(hp, printsettings, pageformat);
		if(theErr != noErr)
			goto exit;
		theErr = PMSessionGetGraphicsContext(hp, nil, (void**)&printport);//the second parameter is currently ignored.
		shellprintinfo.printport = printport;
		if(theErr != noErr)
			goto exit;
		#else
		shellprintinfo.printport = printport = PrOpenDoc (hp, nil, nil);
		currentprintport = w;
		#endif
		
	#endif

	(*shellglobals.beginprintroutine) (); /*prepares for printing*/

	(*shellglobals.setprintinfoproutine) (); /*fills in fields of printinfo record*/
	//this only counts the number of pages.
	
	#ifdef MACVERSION

		//limit this to the number of pages the user asked for. 
		#if TARGET_API_MAC_CARBON == 1
				
			theErr = PMGetFirstPage(printsettings, &firstPage);
			if (theErr == noErr){
				theErr = PMGetLastPage(printsettings, &lastPage);
			}
			//sanity checks
			if(theErr != noErr)
				goto exit;
			//we are not setting up a idle proc. Do we need one?
		
		#endif
		
		if(lastPage > shellprintinfo.ctpages)
			lastPage = shellprintinfo.ctpages;
			
		for (i = 1; i <= lastPage; i++) { /*print one page*/
			
			#if TARGET_API_MAC_CARBON == 1
			theErr = PMSessionBeginPage(hp, pageformat, (PMRect *) &shellprintinfo.pagerect);
			if (theErr != noErr)
				break;
			//now set the graphics context.
			//in carbon we have to set up the port each time
			//to make sure it is in the printer port. Since other things can be going on
			//during the printing, the port may have changed.
			{
				GrafPtr thePort, oldPort;
				GetPort(&oldPort);
				theErr = PMSessionGetGraphicsContext(hp, nil, (void **) &thePort);//the second parameter is currently ignored.
				SetPort(thePort);
				// Draw the page
				SetFractEnable (true);
				fl = (*shellglobals.printroutine) (i);
					break;
				//reset back to the old port.
				SetPort(oldPort);
			}
			#else

			if (PrError () != noErr)
				break;
			
			PrOpenPage (printport, nil);
			
			if (PrError () == noErr) {
				
				SetFractEnable (true);
				
				fl = (*shellglobals.printroutine) (i);
				
				SetFractEnable (false);
				}
			
			PrClosePage (printport);
			#endif
			
			if (!fl)
				break;
			
			if (keyboardescape ()) {
				#if TARGET_API_MAC_CARBON == 1
				PMSessionSetError(hp, iPrAbort);
				#else
				PrSetError (iPrAbort);
				#endif
				}
			} /*for*/

	#endif

	#ifdef WIN95VERSION
		
	for (i = 1; i <= shellprintinfo.ctpages; i++) { /*print one page*/

		if (StartPage(printport) > 0) {
			fl = (*shellglobals.printroutine) (i);
			}

		EndPage (printport);
		
		if (!fl)
			break;
		} /*for*/

	#endif

	#ifdef MACVERSION
		#if TARGET_API_MAC_CARBON == 1
		theErr = PMSessionEndDocument(hp);
		if(theErr !=  noErr)
			goto exit;
		//everything worked and printing is done. so set fl to true.
		fl = true;
		//the PrPicFile is not supported in carbon.
		
		#else
		
		PrCloseDoc (printport);
		
		if (fl) {
			
			if ((PrError () == noErr) && ((**hp).prJob.bJDocLoop == bSpoolLoop))
				PrPicFile (hp, nil, nil, nil, &printstatus);
			
			fl = shellcheckprinterror (false);
			}
		#endif
		
		popport ();
	#endif

	#ifdef WIN95VERSION
		EndDoc (printport);
		winpopport();
		currentprintport = NULL;
		DeleteDC (printport);
	#endif
	
	(*shellglobals.endprintroutine) ();
	
	shellpopglobals ();
	
	exit:
	
	#ifdef MACVERSION
		#if TARGET_API_MAC_CARBON == 1
		PMRelease(hp);
		PMRelease(pageformat);
		PMRelease(printsettings);
		#else
		PrClose ();
		#endif
	#endif

	currentprintport = NULL;

	return (fl);
	} /*shellprint*/

