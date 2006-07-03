
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <iowacore.h>
#include "applet.h"
#include "appletmain.h"
#include "appletwires.h"
#include "appletcardops.h"
#include "appletcard.h"


#define infocardid 157
#define twowaycardid 158
#define threewaycardid 159
#define alertcardid 160
#define askcardid 161
#define passwordcardid 173


static boolean alertshowing = false;

static Handle dialogstring1, dialogstring2, dialogstring3, dialogstring4;

static boolean flconfirmed; /*for confirm dialog*/

static short threewayvalue; /*for threeway dialog*/


/*
flag to control whether the user is interacting, or we're responding to an
interapplication message. the error string is saved -- to be accessed by the
GetErrorString verb.
*/
	boolean fldialogsenabled = true;
	bigstring bsalertstring;
	



static void alertfilter (EventRecord *ev) {
	
	switch ((*ev).what) {
		
		case iowaInitEvent: 
			setObjectValue ((hdlcard) (*ev).message, "\pmessage", dialogstring1);
			
			break;
			
		case iowaButtonHitEvent: {
			hdlobject h = (hdlobject) (*ev).message;
		
			currentcard = (hdlcard) (**h).owningcard;
				
			appsavedialoginfo (alertcardid); /*remember window position*/
			
			(*ev).what = iowaCloseEvent;
			
			break;
			}
			
		default:
			appcarddefaulteventhandler (ev);
			
			break;
		} /*switch*/
	} /*alertfilter*/
	 

boolean alertdialog (bigstring bs) {
	
	if (alertshowing) /*don't allow alerts to nest, only the first error displays*/
		return (false);
		
	copystring (bs, bsalertstring);
	
	if (!fldialogsenabled)
		return (true);
		
	newtexthandle (bs, &dialogstring1);
	
	alertshowing = true;
			
	sysbeep;
	
	apprunmodalresource (alertcardid, &alertfilter);
	
	#ifdef fldebug
	
		if (optionkeydown ())
			DebugStr ("\pHello!");
			
	#endif
	
	alertshowing = false;
	
	return (true);
	} /*alertdialog*/
	

boolean infodialog (bigstring bs) {
	
	copystring (bs, bsalertstring);
	
	if (!fldialogsenabled)
		return (true);
		
	newtexthandle (bs, &dialogstring1);
	
	apprunmodalresource (infocardid, &alertfilter);
	
	return (true);
	} /*infodialog*/
	

static void twowayfilter (EventRecord *ev) {
	
	switch ((*ev).what) {
		
		case iowaInitEvent: 
			setObjectValue ((hdlcard) (*ev).message, "\pmessage", dialogstring1);
			
			setObjectValue ((hdlcard) (*ev).message, "\pdefaultButton", dialogstring2);
			
			setObjectValue ((hdlcard) (*ev).message, "\potherButton", dialogstring3);
			
			break;
			
		case iowaButtonHitEvent: {
			hdlobject h = (hdlobject) (*ev).message;
			bigstring bs;
			
			getobjectnamestring (h, bs);
			
			flconfirmed = equalstrings (bs, "\pdefaultButton");
			
			currentcard = (hdlcard) (**h).owningcard;
				
			appsavedialoginfo (twowaycardid); /*remember window position*/
			
			(*ev).what = iowaCloseEvent; /*either button closes the dialog*/
			
			break;
			}
			
		default:
			appcarddefaulteventhandler (ev);
			
			break;
		} /*switch*/
	} /*twowayfilter*/
	
	
boolean twowaydialog (bigstring bsprompt, bigstring defaultbutton, bigstring otherbutton) {
	
	copystring (bsprompt, bsalertstring);
	
	if (!fldialogsenabled)
		return (true);
		
	newtexthandle (bsprompt, &dialogstring1);
			
	newtexthandle (defaultbutton, &dialogstring2);
			
	newtexthandle (otherbutton, &dialogstring3);
			
	apprunmodalresource (twowaycardid, &twowayfilter);
	
	return (flconfirmed);
	} /*twowaydialog*/
	
	
boolean confirmdialog (bigstring bs) {
	
	return (twowaydialog (bs, "\pOK", "\pCancel"));
	} /*confirmdialog*/
	
	
boolean yesnodialog (bigstring bs) {
	
	return (twowaydialog (bs, "\pYes", "\pNo"));
	} /*yesnodialog*/
	
	
static void threewayfilter (EventRecord *ev) {
	
	switch ((*ev).what) {
		
		case iowaInitEvent: {
			hdlcard context = (hdlcard) (*ev).message;
			
			setObjectValue (context, "\pmessage", dialogstring1);
			
			setObjectValue (context, "\pleftButton", dialogstring2);
			
			setObjectValue (context, "\pmidButton", dialogstring3);
			
			setObjectValue (context, "\prightButton", dialogstring4);
			
			break;
			}
			
		case iowaButtonHitEvent: {
			hdlobject h = (hdlobject) (*ev).message;
			bigstring bs;
			
			getobjectnamestring (h, bs);
			
			if (equalstrings (bs, "\pleftButton"))
				threewayvalue = 3;
			
			if (equalstrings (bs, "\pmidButton"))
				threewayvalue = 2;
			
			if (equalstrings (bs, "\prightButton"))
				threewayvalue = 1;
			
			currentcard = (hdlcard) (**h).owningcard;
				
			appsavedialoginfo (threewaycardid); /*remember window position*/
			
			(*ev).what = iowaCloseEvent; /*either button closes the dialog*/
			
			break;
			}
			
		default:
			appcarddefaulteventhandler (ev);
			
			break;
		} /*switch*/
	} /*threewayfilter*/
	
	
short threewaydialog (bigstring bsprompt, bigstring leftbutton, bigstring midbutton, bigstring rightbutton) {
	
	copystring (bsprompt, bsalertstring);
	
	if (!fldialogsenabled)
		return (true);
		
	newtexthandle (bsprompt, &dialogstring1);
			
	newtexthandle (leftbutton, &dialogstring2);
			
	newtexthandle (midbutton, &dialogstring3);
			
	newtexthandle (rightbutton, &dialogstring4);
			
	apprunmodalresource (threewaycardid, &threewayfilter);
	
	return (threewayvalue);
	} /*threewaydialog*/
	
	
static void askfilter (EventRecord *ev) {
	
	switch ((*ev).what) {
		
		case iowaInitEvent: 
			setObjectValue ((hdlcard) (*ev).message, "\pmessage", dialogstring1);
			
			setObjectValue ((hdlcard) (*ev).message, "\panswer", dialogstring2);
			
			break;
			
		case iowaButtonHitEvent: {
			hdlobject hobject = (hdlobject) (*ev).message;
			bigstring bs;
			
			getobjectnamestring (hobject, bs);
			
			if (equalstrings (bs, "\pokButton")) {
				
				Handle hvalue;
				
				flconfirmed = true;
				
				getObjectValue ((hdlcard) (**hobject).owningcard, "\panswer", &hvalue);
				
				copyhandle (hvalue, &dialogstring2);
				}
			else {
				flconfirmed = false;
				
				dialogstring2 = nil;
				}
			
			currentcard = (hdlcard) (**hobject).owningcard;
				
			appsavedialoginfo (askcardid); /*remember window position*/
			
			(*ev).what = iowaCloseEvent; /*either button closes the dialog*/
			
			break;
			}
			
		default:
			appcarddefaulteventhandler (ev);
			
			break;
		} /*switch*/
	} /*askfilter*/
	
	
static boolean onestringdialog (short id, bigstring prompt, bigstring answer) {
	
	copystring (prompt, bsalertstring);
	
	if (!fldialogsenabled)
		return (true);
		
	newtexthandle (prompt, &dialogstring1);
			
	newtexthandle (answer, &dialogstring2);
			
	apprunmodalresource (id, &askfilter);
	
	texthandletostring (dialogstring2, answer);
	
	disposehandle (dialogstring2);
	
	return (flconfirmed);
	} /*onestringdialog*/


boolean askdialog (bigstring prompt, bigstring answer) {
	
	return (onestringdialog (askcardid, prompt, answer));
	} /*askdialog*/


boolean passworddialog (bigstring prompt, bigstring answer) {
	
	return (onestringdialog (passwordcardid, prompt, answer));
	} /*passworddialog*/


static void replaceexistingfilter (EventRecord *ev) {
	
	switch ((*ev).what) {
		
		case iowaInitEvent: 
			setObjectValue ((hdlcard) (*ev).message, "\pprompt", dialogstring1);
			
			break;
			
		case iowaButtonHitEvent: {
			hdlobject h = (hdlobject) (*ev).message;
			bigstring bs;
			
			getobjectnamestring (h, bs);
			
			flconfirmed = equalstrings (bs, "\preplace");
			
			currentcard = (hdlcard) (**h).owningcard;
				
			appsavedialoginfo (150); /*remember window position*/
			
			(*ev).what = iowaCloseEvent; /*either button closes the dialog*/
			
			break;
			}
			
		default:
			appcarddefaulteventhandler (ev);
			
			break;
		} /*switch*/
	} /*replaceexistingfilter*/
	
	
boolean replaceexistingdialog (bigstring fname) {
	
	bigstring bs;
	short top, left;
	
	if (!fldialogsenabled)
		return (true);
	
	copystring ("\pReplace existing “", bs);
	
	pushstring (fname, bs);
	
	pushstring ("\p”?", bs);
	
	newtexthandle (bs, &dialogstring1);
	
	/*position of this dialog is relative to the position of the front window*/ {
	
		WindowPtr w = FrontWindow ();
		Rect r;
		
		r = (*w).portRect;
		
		pushmacport (w);
				
		localtoglobalrect (&r);
				
		popmacport ();
		
		top = r.top + 32;
		
		left = r.left + 32;
		}
			
	apprunmodalresource (150, &replaceexistingfilter);
	
	return (flconfirmed);
	} /*replaceexistingdialog*/
	
	
	
