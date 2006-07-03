
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletinternal.h"
#include "appletops.h"
#include "appletquickdraw.h"
#include "appletmain.h"


static boolean flprintinit = false;


static void printcomputewindowinfo (WindowPtr w, hdlappwindow appwindow) {
	
	Rect r;
	
	r = (*w).portRect;
	
	(**appwindow).windowrect = r;
	
	(**appwindow).contentrect = r;
	
	zerorect (&(**appwindow).statusrect);
	
	(**appwindow).windowvertpixels = r.bottom - r.top;
	
	(**appwindow).windowhorizpixels = r.right - r.left;
	
	(**appwindow).macwindow = w;
	} /*printcomputewindowinfo*/
	

static void disposeprintinfo (void) {
	
	/*
	2/18/92 dmb: don't clear all fields; just nil the handle.  that way, a 
	hard-coded paper area can be maintained.
	*/
	
	disposehandle ((Handle) app.printinfo.printhandle);
	
	app.printinfo.printhandle = nil;
	} /*disposeprintinfo*/


static boolean checkprinterror (void) {
	
	/*
	returns false if there was a print error.
	*/
	
	short x;
	
	x = PrError ();
	
	if (x == noErr) /*no error, keep going*/
		return (true);
	
	if (x != iPrAbort) 
		alertdialog ("\pPrinting error.");
	
	return (false); /*there was an error*/
	} /*checkprinterror*/


static void copyprintinfo (void) {
	
	Rect r;
	
	r = app.printinfo.paperrect = (**(THPrint) app.printinfo.printhandle).prInfo.rPage;
	
	app.printinfo.vpagepixels = r.bottom - r.top; 
	} /*copyprintinfo*/


boolean initprint (void) {
	
	/*
	2/18/92 dmb: set paperrect to a hard-coded default, in case the 
	print manager can't be opened.  also, manage flprintinit here, instead of 
	having every caller do so.
	*/
	
	Handle h;
	
	if (flprintinit) /*already initted*/
		return (true);
	
	clearbytes (&app.printinfo, longsizeof (app.printinfo));
	
	SetRect (&app.printinfo.paperrect, 5, 6, 725, 546); /*defaults in case PrOpen fails*/
	
	if (!newclearhandle (longsizeof (TPrint), &h))
		return (false);
	
	app.printinfo.printhandle = h; /*copy into print record*/
	
	PrOpen (); /*initialize the Mac print manager*/
	
	if (!checkprinterror ())
		goto error;
	
	PrintDefault ((THPrint) app.printinfo.printhandle); /*set default print record*/
	
	PrValidate ((THPrint) app.printinfo.printhandle);
	
	PrClose (); /*shouldn't leave print resources open all the time*/
	
	if (!checkprinterror ()) 
		goto error;
	
	copyprintinfo (); /*copies fields from handle into record*/
	
	flprintinit = true;
	
	return (true);
	
	error:
	
	disposeprintinfo ();
	
	return (false);
	} /*initprint*/
	
	
boolean getprintinfo (void) {
	
	/*
	after calling this you can safely refer to fields of the 
	app.printinfo record.
	*/
	
	if (!initprint ()) 
		return (false);
	
	return (true);
	} /*getprintinfo*/


static boolean pagesetupvisit (hdlappwindow appwindow) {

	setappwindow (appwindow);
	
	(*app.pagesetupcallback) ();
	
	return (true);
	} /*pagesetupvisit*/


boolean pagesetup (void) {

	if (!initprint ()) 
		return (false);
	
	PrOpen ();
	
	if (!checkprinterror ()) 
		return (false);
	
	PrValidate ((THPrint) app.printinfo.printhandle);
	
	PrStlDialog ((THPrint) app.printinfo.printhandle);
	
	PrClose ();
	
	if (!checkprinterror ())
		return (false);
	
	copyprintinfo (); /*copies fields from handle into record*/
	
	visitappwindows (&pagesetupvisit);
	
	return (true);
	} /*pagesetup*/


boolean printappwindow (hdlappwindow appwindow, boolean fldialog) {
	
	TPPrPort printport;
	TPrStatus printstatus;
	short i;
	WindowPtr origmacwindow;
	boolean fl;
	
	if (!initprint ()) 
		return (false);
	
	setappwindow (appwindow);
	
	PrOpen ();
	
	if (!checkprinterror ())
		return (false);
	
	fl = false; /*until sucessful print, this is return value*/
	
	if (fldialog) {
		
		if (!PrJobDialog ((THPrint) app.printinfo.printhandle)) {
			
			PrClose ();
			
			return (false);
			}
			
		appserviceeventqueue (); /*update all dirtied windows*/
		
		setappwindow (appwindow); /*9/3/92 DW: app.appwindow could have changed*/
		}
	else
		PrValidate ((THPrint) app.printinfo.printhandle);
	
	watchcursor ();
	
	copyprintinfo ();
	
	(**app.appwindow).flprinting = true;
	
	(*app.openprintcallback) (); /*fills in fields of printinfo record*/
	
	pushmacport (nil); /*save current port on stack*/
	
	printport = PrOpenDoc ((THPrint) app.printinfo.printhandle, nil, nil);
	
	for (i = 1; i <= app.printinfo.ctpages; i++) { /*print one page*/
		
		if (!appserviceeventqueue ()) /*user must have selected quit or something like that*/
			PrSetError (iPrAbort);
			
		if (PrError () != noErr)
			break;
		
		PrOpenPage (printport, nil);
		
		if (PrError () == noErr) {
			
			origmacwindow = (**app.appwindow).macwindow;
			
			printcomputewindowinfo ((WindowPtr) printport, app.appwindow);
			
			SetFractEnable (true);
			
			fl = (*app.printpagecallback) (i);
			
			SetFractEnable (false);
			
			computewindowinfo (origmacwindow, app.appwindow);
			}
		
		PrClosePage (printport);
		
		if (!fl)
			break;
		} /*for*/
	
	PrCloseDoc (printport);
	
	if (fl) {
		
		THPrint hp = (THPrint) app.printinfo.printhandle;
		
		if ((PrError () == noErr) && ((**hp).prJob.bJDocLoop == bSpoolLoop))
			PrPicFile (hp, nil, nil, nil, &printstatus);
		
		fl = checkprinterror ();
		}
	
	popmacport ();
	
	(*app.closeprintcallback) ();
	
	(**app.appwindow).flprinting = false;
	
	PrClose ();
	
	return (fl);
	} /*printappwindow*/


