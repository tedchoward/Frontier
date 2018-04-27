
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


CGrafPtr currentprintport = NULL;


#		define iPrAbort kPMCancel




typrintinfo shellprintinfo;

/*
static shelldisposeprintinfo (void) {

	disposehandle ((Handle) shellprintinfo.printhandle);
	
	clearbytes (&shellprintinfo, longsizeof (shellprintinfo));
	} /%shelldisposeprintinfo%/
*/



static boolean
carbonKillPrintVars(void)
{
	
	
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
	
	
	return(true);
}	


static boolean
carbonValidSession(void)
{
	return (nil != shellprintinfo.printhandle);
}


static boolean
carbonSessionPrintSession()
{
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
	return(true);
}


static boolean
carbonSessionDefaultPageAndSettingValidate(void)
{
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

	
	return false;
	
}


static boolean
carbonCreateFormatAndSetting(void)
{
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
	
	//typrintinfo *dbgprintinfo = &shellprintinfo;
	
	
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
	
	
	currentprintport = NULL;
	
	clearbytes (&shellprintinfo, longsizeof (shellprintinfo));
	
	setrect (&shellprintinfo.margins, 36, 36, 36, 36);
	
	setrect (&shellprintinfo.paperrect, 5, 6, 725, 546); /*defaults in case PrOpen fails*/
	
	shellprintinfo.scaleMult = 1;
	shellprintinfo.scaleDiv = 1;
	
	
	
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
	
	
	
	
	//#if !TARGET_API_MAC_CARBON
	
	shellcopyprintinfo (); /*copies fields from handle into record*/
	
	//#endif
	
	return (true);
	
		
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
	
	
	//Code change by Timothy Paustian Friday, June 16, 2000 10:08:37 PM
	//new code for the new print manager.
	
	
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

	theErr = PMGetFirstPage(shellprintinfo.printsettings, (UInt32 *) (&firstPage));
	if (theErr == noErr)
	{
		theErr = PMGetLastPage(shellprintinfo.printsettings, (UInt32 *) (&lastPage));
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


