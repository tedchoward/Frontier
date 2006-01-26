
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

#	if TARGET_API_MAC_CARBON

#		define iPrAbort kPMCancel

#	endif

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
	
#	ifdef MACVERSION
	
	//Code change by Timothy Paustian Friday, June 16, 2000 3:50:52 PM
	//Changed to Opaque call for Carbon
	// this is pointless --- explicitly excluded
#		if TARGET_API_MAC_CARBON == 1
	
	x = PMSessionError(shellprintinfo.printhandle);
	
#		else	
	
	x = PrError ();
	
#		endif
	
	if (x == noErr) /*no error, keep going*/
		return (true);
	
	if (x != iPrAbort) {
		
		if (flopening) 
		{
			GetIndString (bserror, interfacelistnumber, trychooserstring);
		}
		else {
			
			getsystemerrorstring (x, bserror);
			
			parsedialogstring (bserror, "\x06" ".Print", nil, nil, nil, bserror);
		}
		
		shellerrormessage (bserror);
	}
	
#	endif
	
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


static boolean
carbonKillPrintVars(void)
{
	
#if TARGET_API_MAC_CARBON == 1
	
	// empty out carbon print vars
	// setting to nil is important since it is checked for
	if (nil != shellprintinfo.printsettings)
	{
		PMRelease(shellprintinfo.printsettings);
		shellprintinfo.printsettings = nil;
	}
	
	if (nil != shellprintinfo.pageformat)
	{
		PMRelease(shellprintinfo.pageformat);
		shellprintinfo.pageformat = nil;
	}
	
	if (nil != shellprintinfo.printhandle)
	{
		PMRelease(shellprintinfo.printhandle);
		shellprintinfo.printhandle = nil;
	}
	
#endif
	
	return(true);
}	


static boolean
carbonValidSession(void)
{
#if TARGET_API_MAC_CARBON == 1
	return (nil != shellprintinfo.printhandle);
#else
	return (false);
#endif
}


static boolean
carbonSessionPrintSession()
{
#if TARGET_API_MAC_CARBON == 1
	OSStatus	theErr;

	if (nil == shellprintinfo.printhandle)
	{
		theErr = PMCreateSession(&shellprintinfo.printhandle);
		
		if(theErr != noErr)
		{
			shellprintinfo.printhandle = nil;
			return (false);
		}
	}
#endif	
	return(true);
}


static boolean
carbonSessionDefaultPageAndSettingValidate(void)
{
#if TARGET_API_MAC_CARBON == 1
	OSStatus	theErr;
	Boolean		f;
	
	
	theErr = PMSessionDefaultPageFormat(
										shellprintinfo.printhandle,
										shellprintinfo.pageformat);
	if(theErr != noErr)
		goto error;
	
	theErr = PMSessionValidatePageFormat(
										 shellprintinfo.printhandle,
										 shellprintinfo.pageformat, &f);
	if(theErr != noErr)
		goto error;
	
	theErr = PMSessionDefaultPrintSettings(
										   shellprintinfo.printhandle,
										   shellprintinfo.printsettings);
	if(theErr != noErr)
		goto error;
	
	theErr = PMSessionValidatePrintSettings(
											shellprintinfo.printhandle,
											shellprintinfo.printsettings, &f);
	if(theErr != noErr)
		goto error;

	
	return (true);
	
error:
		carbonKillPrintVars();

#endif
	
	return false;
	
}


static boolean
carbonCreateFormatAndSetting(void)
{
#if TARGET_API_MAC_CARBON == 1
	OSStatus	theErr;

	theErr = PMCreatePageFormat(&shellprintinfo.pageformat);
	if(theErr != noErr || (shellprintinfo.pageformat == kPMNoPageFormat))
		goto error;
	
	theErr = PMCreatePrintSettings(&shellprintinfo.printsettings);
	if(theErr != noErr)
		goto error;

	return (true);
	
error:
		carbonKillPrintVars();
	
#endif
	return false;
}


static boolean
carbonStdSetup(void)
{
	boolean fl;
	
	fl = carbonKillPrintVars();
	
	fl &= !carbonCreateFormatAndSetting();
	
	fl &= !carbonSessionPrintSession();
	
	fl &= !carbonSessionDefaultPageAndSettingValidate();
	
	return(fl);
}


#define rect2rect(dstr, srcr) \
{\
	srcr.top	= (short)lround(dstr.top); \
	srcr.bottom	= (short)lround(dstr.bottom); \
	srcr.left	= (short)lround(dstr.left); \
	srcr.right	= (short)lround(dstr.right); \
}


static void shellcopyprintinfo (void) {
	
	/*
	 set up our printing rect according to the print manager's rects.
	 
	 we'll use the smaller of the printable area (rPage) and the physical 
	 paper (rPaper) with 1/2 inch margins
	 
	 5.0.1 dmb: measure in pixels (1/72 inch) for Win
	 */
	
	Rect
	rpage,
	rpaper;
	
	typrintinfo *dbgprintinfo = &shellprintinfo;
	
#ifdef MACVERSION
#	if TARGET_API_MAC_CARBON == 1
	
	static int firstTime = 1;
	
	OSErr 			theErr;
	boolean			fl;
	PMRect			dRect;
	
	
	if (1 == firstTime)
	{
		carbonSessionPrintSession();
		carbonCreateFormatAndSetting();
		carbonSessionDefaultPageAndSettingValidate();
	}
	
	
	theErr = PMSessionValidatePageFormat(
										 shellprintinfo.printhandle,
										 shellprintinfo.pageformat, &fl);
	
	if(theErr != noErr) // || !fl) // why should i kill this if nothing has changed???
		oserror(theErr);
	
	theErr = PMGetAdjustedPageRect(shellprintinfo.pageformat, &dRect);
	
	shellprintinfo.pagerect = dRect;
	
	theErr = PMGetAdjustedPaperRect(shellprintinfo.pageformat, &dRect);
	
	rect2rect(dRect, shellprintinfo.paperrect);
	
	rect2rect(shellprintinfo.pagerect, rpage);
	
	rpaper = shellprintinfo.paperrect;
	
#	else
	
	rpage = (**shellprintinfo.printhandle).prInfo.rPage;
	
	rpaper = (**shellprintinfo.printhandle).rPaper;
	
#	endif
	
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
	
#	if TARGET_API_MAC_CARBON == 1
	
	//I realized this is only called once during the startup of the app.
	//Carbon printing really doesn't need any global structures. Better to 
	//allocate them as we use them.
	
	// Nope.
	// We need a global var for calling page setup
	
	shellprintinfo.printport = nil;
	shellprintinfo.printhandle = nil;
	shellprintinfo.pageformat = nil;
	shellprintinfo.printsettings = nil;
	// shellprintinfo.pagerect = {0,0,0,0};
	
#	else		
	
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
#	endif
	
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
	
	//#if !TARGET_API_MAC_CARBON
	
	shellcopyprintinfo (); /*copies fields from handle into record*/
	
	//#endif
	
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
#pragma unused (refcon)
	
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
	
#	if TARGET_API_MAC_CARBON == 1
	
	if (!carbonValidSession())
	{
		carbonStdSetup();
	}
	
	{
		OSStatus status;
		
		status = PMSessionPageSetupDialog(
										  shellprintinfo.printhandle, 
										  shellprintinfo.pageformat,
										  &fl);
		
		if (!fl)
			status = kPMCancel; // user clicked Cancel button
		
		if(status != noErr)
			return false;
		
	}
	
#	else
	
	PrOpen ();
	
	if (!shellcheckprinterror (true)) 
		return (false);
	
	PrValidate (shellprintinfo.printhandle);
	
	fl = PrStlDialog (shellprintinfo.printhandle);
	
	PrClose ();
	
	if (!shellcheckprinterror (false))
		return (false);
	
#	endif
	
#endif
	
#ifdef WIN95VERSION
	
	shellprintinfo.pagesetupinfo.Flags = PSD_MARGINS | PSD_INTHOUSANDTHSOFINCHES;
	
	shellprintinfo.pagesetupinfo.hwndOwner = hwndMDIClient;
	
	fl = PageSetupDlg (&shellprintinfo.pagesetupinfo);
	
#endif
	
	if (!fl)
		return (false);
	
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


#ifdef WIN95VERSION

// kw - 2006-06 - splitted for each platform
#pragma mark shellprint windows
boolean shellprint (WindowPtr w, boolean fldialog) {
	
	/*
	 9/5/90 dmb:  open and close print resources each time we're called.  also, 
	 make sure we close each page opened, even when errors occur
	 
	 9/28/91 dmb: for exit of edit mode before printing
	 
	 4/24/92 dmb: make sure bJDocLoop is bSpoolLoop before calling PrPicFile, as 
	 per IM II-155.  otherwise, we can get bogus PrErrors
	 */

	
	PRINTDLG pd;
	DOCINFO di;
	HDC printport;
	hdlwindowinfo hinfo;
	
	short i;
	boolean fl = false;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	fl = false; /*until sucessfull print, this is return value*/

	ZeroMemory (&pd, sizeof (PRINTDLG));
	pd.lStructSize = sizeof (PRINTDLG);
	pd.hwndOwner = hwndMDIClient;
	pd.Flags = PD_RETURNDC;
	
	if (!fldialog)
		pd.Flags = pd.Flags | PD_RETURNDEFAULT;
	
	if (! PrintDlg (&pd)) {
		goto exit;
	}
	
	setcursortype (cursoriswatch);
	
	shellupdateallnow (); /*update all windows that were dirtied by the print dialog*/
	
	shellpushglobals (w);
	
	(*shellglobals.settextmoderoutine) (false); /*make sure editing changes are accepted*/
		
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
	
	
	/*prepares for printing*/
	(*shellglobals.beginprintroutine) ();
	
	/*fills in fields of printinfo record*/
	(*shellglobals.setprintinfoproutine) ();
	//this only counts the number of pages.
	
	
	for (i = 1; i <= shellprintinfo.ctpages; i++) { /*print one page*/
		
		if (StartPage(shellprintinfo.printport) > 0) {
			fl = (*shellglobals.printroutine) (i);
		}
		
		EndPage (shellprintinfo.printport);
		
		if (!fl)
			break;
	} /*for*/
	
	
	EndDoc (printport);
	
	winpopport();
	
	currentprintport = NULL;
	
	DeleteDC (printport);
	
	
	(*shellglobals.endprintroutine) ();
	
	shellpopglobals ();
	
exit:
		
		
		currentprintport = NULL;
	
	return (fl);
	
} /*shellprint*/

		
#endif

#ifdef MACVERSION
#	if TARGET_API_MAC_CARBON == 1
#		pragma mark shellprint carbon

boolean shellprint (WindowPtr w, boolean fldialog) {

	/*
	9/5/90 dmb:  open and close print resources each time we're called.  also, 
	make sure we close each page opened, even when errors occur
	
	9/28/91 dmb: for exit of edit mode before printing
	
	4/24/92 dmb: make sure bJDocLoop is bSpoolLoop before calling PrPicFile, as 
	per IM II-155.  otherwise, we can get bogus PrErrors
	*/

	OSStatus 		theErr;
	boolean			accepted;

	//remember that this is a grafport, TPPrPort isn't
	//but this should work because all the code that
	//messes with printing uses a grafport. 

	// CGrafPtr			printport;
	UInt32
		minPage = 1,
		maxPage = 9999;

	SInt32	firstPage = 1;
	SInt32	lastPage = 9999;

	short i;
	boolean fl = false;
	
	if (w == nil) /*defensive driving*/
		return (false);

	if (!carbonValidSession())
	{
		carbonStdSetup();
	}
	
	fl = false; /*until sucessfull print, this is return value*/
	
	/*fills in fields of printinfo record*/
	(*shellglobals.setprintinfoproutine) ();

	//this only counts the number of pages.
	maxPage = shellprintinfo.ctpages;
	
	//set the page range
	theErr = PMSetPageRange(shellprintinfo.printsettings, minPage, maxPage);
	if(theErr != noErr){
		goto exit;
	}

	//finally display the dialog
	if (fldialog)
	{
		theErr = PMSessionPrintDialog(
							 shellprintinfo.printhandle,
							 shellprintinfo.printsettings,
							 shellprintinfo.pageformat,
							&accepted);

		//either the user canceled or their was some other error.
		//don't print
		if (!accepted || (theErr != noErr)){
			goto exit;
		}
	}

	setcursortype (cursoriswatch);

	shellupdateallnow (); /*update all windows that were dirtied by the print dialog*/

	shellpushglobals (w);

	(*shellglobals.settextmoderoutine) (false); /*make sure editing changes are accepted*/

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

	//we have to call this first before we can get the graphics context
	theErr = PMSessionBeginDocument(
							shellprintinfo.printhandle,
							shellprintinfo.printsettings,
							shellprintinfo.pageformat);

	if(theErr != noErr)
		goto exit;

	/*prepares for printing*/
	(*shellglobals.beginprintroutine) ();

	/*fills in fields of printinfo record*/
	// (*shellglobals.setprintinfoproutine) ();
	//this only counts the number of pages.
	
	//limit this to the number of pages the user asked for. 

	theErr = PMGetFirstPage(shellprintinfo.printsettings, &firstPage);
	if (theErr == noErr)
	{
		theErr = PMGetLastPage(shellprintinfo.printsettings, &lastPage);
	}

	//sanity checks
	if(theErr != noErr)
		goto exit;

	if(lastPage > shellprintinfo.ctpages)
		lastPage = shellprintinfo.ctpages;

	for (i = 1; i <= lastPage; i++) { /*print one page*/

		theErr = PMSessionBeginPage(
							 shellprintinfo.printhandle,
							 shellprintinfo.pageformat,
							&shellprintinfo.pagerect);

		if (theErr != noErr)
			break;
		
		//now set the graphics context.
		//in carbon we have to set up the port each time
		//to make sure it is in the printer port. Since other things can be going on
		//during the printing, the port may have changed.
		{
			GrafPtr thePort, oldPort;

			GetPort(&oldPort);

			//the second parameter is currently ignored.
			theErr = PMSessionGetGraphicsContext(
									shellprintinfo.printhandle,
									kPMGraphicsContextQuickdraw,
									(void **) &thePort);

			shellprintinfo.printport = thePort;

			SetPort(thePort);

			// Draw the page
			SetFractEnable (true);

			fl = (*shellglobals.printroutine) (i);

			//reset back to the old port.
			SetPort(oldPort);
		}

		theErr = PMSessionEndPage(shellprintinfo.printhandle);

		if (!fl)
			break;

		if (keyboardescape ())
		{
			theErr = PMSessionSetError(shellprintinfo.printhandle, iPrAbort);
		}
	} /*for*/

	theErr = PMSessionEndDocument(shellprintinfo.printhandle);

	if(theErr !=  noErr)
		goto exit;

	//everything worked and printing is done. so set fl to true.
	fl = true;

	//the PrPicFile is not supported in carbon.

	popport ();

	(*shellglobals.endprintroutine) ();

	shellpopglobals ();

exit:

	carbonKillPrintVars();
//	PMRelease(shellprintinfo.printhandle);
//	PMRelease(pageformat);
//	PMRelease(printsettings);

	currentprintport = NULL;

	return (fl);

	} /*shellprint*/

#	else
#		pragma mark shellprint classic
boolean shellprint (WindowPtr w, boolean fldialog) {

	/*
	9/5/90 dmb:  open and close print resources each time we're called.  also, 
	make sure we close each page opened, even when errors occur
	
	9/28/91 dmb: for exit of edit mode before printing
	
	4/24/92 dmb: make sure bJDocLoop is bSpoolLoop before calling PrPicFile, as 
	per IM II-155.  otherwise, we can get bogus PrErrors
	*/

	// classic mac
	TPPrPort printport;
	TPrStatus printstatus;
	THPrint hp = shellprintinfo.printhandle;

	SInt32	firstPage = 1;
	SInt32	lastPage = 9999;

	short i;
	boolean fl = false;
	
	if (w == nil) /*defensive driving*/
		return (false);

	PrOpen ();
	if (!shellcheckprinterror (true))
		return (false);

	fl = false; /*until sucessfull print, this is return value*/

	if (fldialog) {
		if (!PrJobDialog (hp))
			goto exit;
	}
	else
		PrValidate (hp);

	setcursortype (cursoriswatch);

	shellupdateallnow (); /*update all windows that were dirtied by the print dialog*/

	shellpushglobals (w);

	(*shellglobals.settextmoderoutine) (false); /*make sure editing changes are accepted*/

	pushport (nil); /*save current port on stack*/

	shellprintinfo.printport = printport = PrOpenDoc (hp, nil, nil);
	currentprintport = w;

	/*prepares for printing*/
	(*shellglobals.beginprintroutine) ();

	/*fills in fields of printinfo record*/
	(*shellglobals.setprintinfoproutine) ();
	//this only counts the number of pages.

	// wird in classic lastPage gesetzt?
	if(lastPage > shellprintinfo.ctpages)
		lastPage = shellprintinfo.ctpages;

	for (i = 1; i <= lastPage; i++) { /*print one page*/

		if (PrError () != noErr)
			break;

		PrOpenPage (printport, nil);

		if (PrError () == noErr) {

			SetFractEnable (true);

			fl = (*shellglobals.printroutine) (i);

			SetFractEnable (false);
		}

		PrClosePage (printport);

		if (!fl)
			break;

		if (keyboardescape ()) {

			PrSetError (iPrAbort);

		}
	} /*for*/

	PrCloseDoc (printport);

	if (fl) {

		if (	(PrError () == noErr)
			&& ((**hp).prJob.bJDocLoop == bSpoolLoop))
		{
			PrPicFile (hp, nil, nil, nil, &printstatus);
		}

		fl = shellcheckprinterror (false);
	}

	popport ();
	
	(*shellglobals.endprintroutine) ();

	shellpopglobals ();

exit:

	PrClose ();

	currentprintport = NULL;

	return (fl);

	} /*shellprint*/

#	endif
#endif

