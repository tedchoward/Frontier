
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

#include "frontierwindows.h"
#include "dialogs.h"
#include "shellhooks.h"
#include "lang.h"
#include "langinternal.h"
#include "process.h"




#define userevents (mDownMask + mUpMask + keyDownMask + keyUpMask + autoKeyMask)

#define maxnesteddialogs 3

typedef struct tydialogglobals {
	
	hdltreenode hcode; /*itemhit callback function call code tree*/
	
	DialogPtr pdialog; /*the dialog itself*/
	} tydialogglobals;


DialogPtr langmodaldialog = nil;


static tydialogglobals dialogstack [maxnesteddialogs];

static short topdialog = 0;

static hdltreenode hitemhitcallback;




static void pushdialogcall (void) {
	
	dialogstack [topdialog].hcode = hitemhitcallback;
	
	dialogstack [topdialog++].pdialog = langmodaldialog;
	} /*pushdialogcall*/


static void popdialogcall (void) {
	
	hitemhitcallback = dialogstack [--topdialog].hcode;
	
	langmodaldialog = dialogstack [topdialog].pdialog;
	} /*popdialogcall*/


boolean langdialogrunning (void) {
	
	return (langmodaldialog != nil);
	} /*langdialogrunning*/


static boolean langvaliddialogitem (short itemnum) {
	
	if ((itemnum < 1) || (itemnum > dialogcountitems (langmodaldialog))) {
		
		langlongparamerror (dialogitemnumerror, (long) itemnum);
		
		return (false);
		}
	
	return (true);
	} /*langvaliddialogitem*/


static boolean langdialoggetitemparam (hdltreenode hfirst, short pnum, short *itemnum) {
	
	if (!langdialogrunning ()) { /*factored run-time check*/
		
		langparamerror (dialognotrunningerror, bsfunctionname);
		
		return (false);
		}
	
	if (!getintvalue (hfirst, pnum, itemnum))
		return (false);
	
	return (langvaliddialogitem (*itemnum));
	} /*langdialoggetitemparam*/


boolean langgetdialogvalue (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	8/12/91 dmb: fixed bug getting value out of resCtrls
	
	3.0.2 dmb: return the title of buttons instead of zero
	*/
	
	register tyvaluerecord *v = vreturned;
	short itemnum;
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	bigstring bs;
	
	flnextparamislast = true;
	
	if (!langdialoggetitemparam (hparam1, 1, &itemnum))
		return (false);
	
#if MACVERSION
	GetDialogItem (langmodaldialog, itemnum, &itemtype, &itemhandle, &itemrect);
	
	switch (itemtype % itemDisable) { /*ignore enabledness*/
		
		case ctrlItem + chkCtrl:
		case ctrlItem + radCtrl:
			
			setbooleanvalue (bitboolean (GetControlValue ((ControlHandle) itemhandle)), v);
			
			break;
		
		case ctrlItem + btnCtrl: /*3.0.2*/
			
			GetControlTitle ((ControlHandle) itemhandle, bs);
			
			setstringvalue (bs, v);
			
			break;
		
		case ctrlItem + resCtrl:
			
			setintvalue (GetControlValue ((ControlHandle) itemhandle), v);
			
			break;
		
		case statText:
		case editText:
			
			GetDialogItemText (itemhandle, bs);
			
			setstringvalue (bs, v);
			
			break;
		
		default:
			setlongvalue (0, v);
			
			break;
		}
#endif

	// *** WIN95 not implemented

	return (true);
	} /*langgetdialogvalue*/


boolean langsetdialogvalue (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	10/7/91 dmb: use setdialogtext instead of SetIText, so that our 
	dialog routines can munge special characters (specifically, '@')
	*/
	
	short itemnum;
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	boolean flvalue;
	short intvalue;
	bigstring bsvalue;
	
	if (!langdialoggetitemparam (hparam1, 1, &itemnum))
		return (false);
	
	setbooleanvalue (true, vreturned); /*optimism*/
	
#if MACVERSION
	GetDialogItem (langmodaldialog, itemnum, &itemtype, &itemhandle, &itemrect);
	
	flnextparamislast = true;
	
	switch (itemtype % itemDisable) { /*ignore enabledness*/
		
		case ctrlItem + chkCtrl:
		case ctrlItem + radCtrl:
			
			if (!getbooleanvalue (hparam1, 2, &flvalue))
				return (false);
			
			SetControlValue ((ControlHandle) itemhandle, flvalue);
			
			break;
		
		case ctrlItem + btnCtrl:
			
			if (!getstringvalue (hparam1, 2, bsvalue))
				return (false);
			
			SetControlTitle ((ControlHandle) itemhandle, bsvalue);
			
			break;
		
		case ctrlItem + resCtrl:
			
			if (!getintvalue (hparam1, 2, &intvalue))
				return (false);
			
			SetControlValue ((ControlHandle) itemhandle, intvalue);
			
			break;
		
		case statText:
		case editText:
			
			if (!getstringvalue (hparam1, 2, bsvalue))
				return (false);
			
			setdialogtext (langmodaldialog, itemnum, bsvalue); /*so bsvalue gets preprocessed*/
			
			/*
			SetIText (itemhandle, bsvalue);
			*/
			
			break;
		
		default:
			flnextparamislast = false; /*we didn't ask for a parameter*/
			
			(*vreturned).data.flvalue = false; /*didn't work out*/
			
			break;
		}
#endif

	// *** WIN95 not implemented

	return (true);
	} /*langsetdialogvalue*/


boolean langsetdialogitemenable (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	short itemnum;
	boolean fl;
	
	if (!langdialoggetitemparam (hparam1, 1, &itemnum))
		return (false);
	
	flnextparamislast = true;
	
	if (!getbooleanvalue (hparam1, 2, &fl))
		return (false);
	
	if (fl)
		enabledialogitem (langmodaldialog, itemnum);
	else
		disabledialogitem (langmodaldialog, itemnum);
	
	setbooleanvalue (true, vreturned);
	
	return (true);
	} /*langsetdialogitemenable*/


boolean langsetdialogitemvis (hdltreenode hparam1, boolean fl, tyvaluerecord *vreturned) {
	
	short itemnum;
	
	flnextparamislast = true;
	
	if (!langdialoggetitemparam (hparam1, 1, &itemnum))
		return (false);
	
	if (fl)
		showdialogitem (langmodaldialog, itemnum);
	else
		hidedialogitem (langmodaldialog, itemnum);
	
	setbooleanvalue (true, vreturned);
	
	return (true);
	} /*langsetdialogitemvis*/

#if isFrontier && !flruntime

static boolean langdialogeventhook (EventRecord *ev, WindowPtr w) {
	
	/*
	2.1b3 dmb: don't drop activate events
	*/
	
	short item;
	short ix;
	boolean fllangdialog;
	DialogPtr pdialog;
	//Code change by Timothy Paustian Saturday, April 29, 2000 10:54:25 PM
	//Changed to Opaque call for Carbon
	RgnHandle	visRgn;
	
	fllangdialog = (w == (WindowPtr) langmodaldialog);
	
	for (ix = 0; (!fllangdialog) && (ix < topdialog); ++ix)
		if ((WindowPtr) dialogstack [ix].pdialog == w)
			fllangdialog = true;
	
	if (!fllangdialog) /*not one of our dialogs -- don't hook*/
		return (true);
	
	switch ((*ev).what) {
		
		case activateEvt:
			DialogSelect (ev, &pdialog, &item);
			
			break;
		
		case updateEvt:
			BeginUpdate (w);
			//Code change by Timothy Paustian Saturday, April 29, 2000 10:56:01 PM
			//Changed to Opaque call for Carbon
			//not tested yet.
			#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			visRgn = NewRgn();
			GetWindowRegion(w, kWindowUpdateRgn, visRgn);
			UpdateDialog ((DialogRef) w, visRgn);
			DisposeRgn(visRgn);
			#else
			#pragma unused(visRgn)
			//old code
			UpdateDialog (w, (*w).visRgn);
			#endif
			modaldialogcallback ((DialogPtr) w, ev, &item);
			
			EndUpdate (w);
			
			break;
		}
	
	return (false); /*ignore the event*/
	} /*langdialogeventhook*/

#endif

static boolean langdialogitemhit (DialogPtr pdialog, short itemnumber) {
	
	/*
	we're going to call the handler code with itemnumber as a parameter.
	
	we'll form the code tree for the parameter list (itemnumber) by hand.
	
	7/29/92 dmb: different event handling for runtime.
	
	1/18/93 dmb: fldisableyield global is now a counter
	
	2.1b11 dmb: don't disable yield if we're debugging
	*/
	
	hdltreenode hparam;
	tyvaluerecord val;
	boolean fl;
	boolean fldebugging;
	
	cleartmpstack ();
	
	setintvalue (itemnumber, &val);
	
	if (!newconstnode (val, &hparam))
		return (false);
	
	pushdialogcall ();
	
	langmodaldialog = pdialog; /*make visible to langdialogeventhook, etc.*/
	
	if (topdialog == 1) { /*only need one event hook for all nested dialogs*/
		
		#if !isFrontier
			++fldisableyield;
		
		#else
			fldebugging = debuggingcurrentprocess ();
			
			if (!fldebugging)
				++fldisableyield;
			
			shellpusheventhook (&langdialogeventhook);
		#endif
		}
	
	fl = langhandlercall (hitemhitcallback, hparam, &val);
	
	if (topdialog == 1) {
	
		#if !isFrontier
			--fldisableyield;
			
		#else
			shellpopeventhook ();
			
			if (!fldebugging)
				--fldisableyield;
		#endif
		}
	
	popdialogcall ();
	
	langdisposetree (hparam);
	
	if ((WindowPtr) pdialog != getfrontwindow ()) /*this can happen under the debugger*/
		windowbringtofront ((WindowPtr) pdialog);
	
	if (!fl)
		return (false);
	
	if (!coercetoboolean (&val))
		return (false);
	
	return (val.data.flvalue);
	} /*langdialogitemhit*/


boolean langrundialog (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	run a modal dialog.  we're expecting to be called with three arguments:
		
		rundialog (dialogid, defaultitem, itemhitcallback)
	
	after extracting these value, we construct a code tree for:
		
		itemhitcallback^
	
	and place the result in hitemhitcallback.  langdialogitemhit will 
	construct a parameter list for each call, and call back to the item 
	hit routine.
	*/
	
	short dialogid;
	short defaultitem;
	tyvaluerecord val;
	hdltreenode htree;
	boolean fl;
	
	if (topdialog == maxnesteddialogs /*langdialogrunning ()*/ ) {
		
		langerror (cantnestdialogserror);
		
		return (false);
		}
	
	if (!getintvalue (hparam1, 1, &dialogid))
		return (false);
	
	if (!getintvalue (hparam1, 2, &defaultitem))
		return (false);
	
	flnextparamislast = true;
	
	if (!getaddressparam (hparam1, 3, &val))
		return (false);
	
	if (!pushfunctionreference (val, &htree))
		return (false);
	
	hitemhitcallback = htree; /*make visible to langdialogitemhit*/
	
	fl = customdialog (dialogid, defaultitem, &langdialogitemhit);
	
	langdisposetree (htree);
	
	if (!fl) {
		
		langlongparamerror (cantloaddialogerror, dialogid);
		
		return (false);
		}
	
	setbooleanvalue (true, vreturned);
	
	return (true);
	} /*langrundialog*/




