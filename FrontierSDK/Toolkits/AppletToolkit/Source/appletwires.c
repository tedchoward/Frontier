
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <GestaltEqu.h>
#include <iac.h>
#include "appletinternal.h"
#include "appletmain.h"
#include "appletmenu.h"
#include "appletscrollbar.h"
#include "appletmenuops.h"
#include "appletresource.h"
#include "appletmsg.h"
#include "appletcardops.h"
#include "appletwires.h"


/*
IAC messages implemented by the Applet Toolkit layer.
*/
	#define alertdialogtoken	'alrt'
	#define confirmdialogtoken	'cnfm'
	#define askdialogtoken		'askd'
		
	#define enabledialogtoken	'enbd'
	#define geterrorstringtoken	'gers'
	
	#define gettargettoken		'gtrg'
	#define settargettoken 		'strg'
	
	#define newwindowtoken 		'nwin'
	#define openwindowtoken		'owin'
	#define editwindowtoken		'ewin'
	#define closewindowtoken	'cwin'
	#define savewindowtoken		'swin'
	#define revertwindowtoken	'rwin'
	#define movewindowtoken		'mwin'
	#define printwindowtoken	'pwin'
	#define selectwindowtoken	'xwin'
	#define zoomwindowtoken		'zwin'
	
	#define getfilepathtoken	'gpth'
	#define getwindowpostoken	'gwps'
	#define madechangestoken	'chgs'

	#define getpicttoken		'gpic'
	#define gettexttoken		'gtex'
	#define putpicttoken		'ppic'
	#define puttexttoken		'ptex'
	#define selectalltoken		'sela'
	#define haveselectiontoken	'hsel'
	
	#define countwindowstoken	'twin'
	#define nthwindowtoken		'nthw'
	#define quittoken			'quit'
	
	#define setfonttoken		'sfon'
	#define setfontsizetoken	'sfsz'

	#define getpagerecttoken	'gprc'
	#define getwindowrecttoken	'gwrc'
	#define getbinarytoken		'gbin'
	#define putbinarytoken		'pbin'
	#define scrollwindowtoken	'lwin'
	#define perftesttoken 		'perf'
	
	#define runcardtoken		'runc'
	#define windowmessagetoken	'wmsg'
	
	#define getmemavailtoken	'mema'
	
	#define getfiletoken		'gfil'

	
bigstring bstargetwindowname; /*title of window that IAC verbs apply to*/

extern bigstring bsvisit; /*use appletmain's string for visiting windows*/



static boolean settargetvisit (hdlappwindow appwindow) {
	
	bigstring bs;
	
	getappwindowtitle (appwindow, bs);
	
	if (!equalstrings (bsvisit, bs)) /*no match -- keep traversing*/
		return (true);
	
	/*found a match, stop the traversal*/
	
	setappwindow (appwindow);
	
	return (false); /*stop the traversal*/
	} /*settargetvisit*/
	
	
static boolean setapptarget (bigstring bs) {
	
	/*
	set the globals for the window with the indicated name without bringing
	the window to the front.
	*/
	
	setappwindow (nil);
	
	copystring (bs, bsvisit); /*copy into global so visit routine can access*/
	
	visitappwindows (&settargetvisit);
	
	return (app.appwindow != nil);
	} /*setapptarget*/	
	

boolean settargetglobals (void) {
	
	/*
	get ready to do an operation on the target window.  if the name is empty,
	we do the operation on the frontmost window.
	
	the target window name is set using the 'sett' IAC call.
	
	return an error to the script if the target window doesn't exist.
	
	DW 11/3/93: handle case where target window was closed.
	*/
	
	if (stringlength (bstargetwindowname) == 0) { /*no target has been established*/
		
		if (!setfrontglobals ()) /*no windows are open*/
			goto error;
			
		return (true);
		}
		
	if (!setapptarget (bstargetwindowname)) {
		
		setstringlength (bstargetwindowname, 0); /*DW 11/3/93*/
		
		if (!setfrontglobals ()) /*no windows are open*/
			goto error;
		}
		
	return (true);
	
	error:
	
	IACreturnerror (1, "\pNo window is open.");
	
	return (false); /*no window open, stop processing the message*/
	} /*settargetglobals*/
	
	
static boolean setwindowtitleverb (void) {
	
	/*
	verb that sets the window title of the target window.
	*/
	
	bigstring bs;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
		
	setappwindowtitle (app.appwindow, bs);
	
	copystring (bs, bstargetwindowname);
		
	return (IACreturnboolean (true));
	} /*setwindowtitleverb*/
	
	
static boolean getwindowrectverb (void) { 
	
	hdlappwindow ha = app.appwindow;
	Rect r = (**app.appwindow).contentrect;
	
	OffsetRect (&r, (**ha).scrollorigin.h, (**ha).scrollorigin.v);
	
	return (IACreturnrect (&r));
	} /*getwindowrectverb*/
	
	
static boolean closewindowverb (void) {
	
	/*
	verb that closes an existing window.  it takes no parameters, the
	current target window is closed.
	
	8/25/92 DW: if we close the target window, reset the target to the
	frontmost window.
	*/
	
	boolean fl = false;
	hdlappwindow ha;
	boolean flresettarget;
	
	flresettarget = stringlength (bstargetwindowname) > 0;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
	
	ha = app.appwindow;
	
	if (ha != nil) { /*a window is open*/
	
		fl = closeappwindow (ha, fldialogsenabled);
		
		if (fl && flresettarget)
			setstringlength (bstargetwindowname, 0); 
		}
		
	return (IACreturnboolean (fl));
	} /*closewindowverb*/
	
	
static boolean revertverb (void) {
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	return (IACreturnboolean (revertfrontwindow ()));
	} /*revertverb*/
	
	
static boolean zoomwindowverb (void) {
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	zoomappwindow (app.appwindow);
		
	return (IACreturnboolean (true));
	} /*zoomwindowverb*/
	

static boolean scrollwindowverb (void) {
	
	short x;
	tydirection dir;
	short ct;

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	if (!IACgetshortparam ('dire', &x))
		return (false);
	
	dir = (tydirection) x;
	
	if (!IACgetshortparam ('coun', &ct))
		return (false);
		
	scrollappwindow (dir, false, ct);
		
	return (IACreturnboolean (true));
	} /*scrollwindowverb*/
	

static boolean verbmakefilespec (bigstring path, short vnum, ptrfilespec pfs) {
	
	bigstring bserror;
	
	if (filemakespec (0, 0, path, pfs))
		return (true);
		
	copystring ("\pCanÕt create a filespec for Ò", bserror);
	
	pushstring (path, bserror);
	
	pushstring ("\pÓ.", bserror);
	
	alertdialog (bserror);
	
	return (false);
	} /*verbmakefilespec*/
	
	
static boolean saveasverb (void) {
	
	boolean fl = false;
	bigstring path;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	if (!IACgetstringparam (keyDirectObject, path))
		return (false);
		
	if (stringlength (path) == 0) {
	
		fl = savefrontwindow ();
		}
	else {
		tyfilespec fs;
		
		if (verbmakefilespec (path, 0, &fs))
			fl = saveaswindow (&fs);
		}
		
	return (IACreturnboolean (fl));
	} /*saveasverb*/
	
	
static boolean openverb (void) {
	
	boolean fl = false;
	bigstring path;
	tyfilespec fs;
	
	setstringlength (bsalertstring, 0);
	
	if (!IACgetstringparam (keyDirectObject, path))
		return (false);
	
	if (verbmakefilespec (path, 0, &fs))
		fl = openappwindow (&fs);
		
	return (IACreturnboolean (fl));
	} /*openverb*/


static boolean editverb (void) {
	
	/*
	4.1b1 dmb: new verb to open a new window and edit caller's data
	*/
	
	boolean fl = false;
	Handle hpacked = nil;
	OSType binarytype;
	AEDesc refdesc = {typeNull, nil};
	bigstring bs;
	OSType sender;
	
	if (!IACgetbinaryparam (keyDirectObject, &hpacked, &binarytype))
		return (false);
	
	if (!IACgetbinaryparam ('refc', &refdesc.dataHandle, &refdesc.descriptorType))
		goto error;
	
	IACglobals.nextparamoptional = true;
	
	if (!IACgetstringparam ('titl', bs))
		setemptystring (bs);
	
	sender = IACgetsender ();
	
	if (!editappwindow (hpacked, sender, &refdesc, bs))
		goto error;
	
	return (IACreturnboolean (true));
	
	error:
	
		AEDisposeDesc (&refdesc);
		
		disposehandle (hpacked);
		
		return (IACreturnboolean (false));
	} /*editverb*/


static boolean nthwindowverb (void) {
	
	bigstring bs;
	short n;
	hdlappwindow appwindow;
	
	if (!IACgetshortparam (keyDirectObject, &n))
		return (false);
		
	findnthwindow (n, &appwindow);
	
	getappwindowtitle (appwindow, bs);
	
	return (IACreturnstring (bs));
	} /*nthwindowverb*/
	
	
static boolean selectwindowverb (void) {
	
	/*
	verb that brings a window to the front, you provide the name of the window in 
	a string parameter.  side effect -- it also sets the target to that window.
	*/
	
	boolean fl = false;
	bigstring bs;
	
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
		
	if (selectwindowbytitle (bs)) {
		
		fl = true;
		
		copystring (bs, bstargetwindowname);
		}
		
	return (IACreturnboolean (fl));
	} /*selectwindowverb*/
	
	
static boolean perftestverb (void) {
	
	/*
	this verb supports a performance benchmark script written in Frontier.
	
	6/28/91 DW: perform rectangle subtraction on two rectangle params,
	return the difference.
	*/
	
	Rect r1, r2, r;
	
	if (!IACgetrectparam ('prm1', &r1))
		return (false);
		
	if (!IACgetrectparam ('prm2', &r2))
		return (false);
		
	r.top = r1.top - r2.top;
	
	r.left = r1.left - r2.left;
	
	r.bottom = r1.bottom - r2.bottom;
	
	r.right = r1.right - r2.right;
	
	return (IACreturnrect (&r));
	} /*perftestverb*/


static boolean windowmessageverb (void) {

	bigstring bs;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
		
	if (app.hasmessagearea)
		appsetmessage (app.appwindow, bs);
		
	return (IACreturnboolean (app.hasmessagearea));
	} /*windowmessageverb*/
	

static void getmemavailverb (void) {

	IACreturnlong (FreeMem ());
	} /*getmemavailverb*/


static boolean alertdialogverb (void) {
	
	/*
	opens up a modal dialog box with the string parameter displayed, wait for
	the user to click on OK before returning the value.	

	to support runtime menus, you wire your dialog boxes to IAC events if you want 
	script writers to be able to transparently talk to the user without Frontier 
	coming to the front.
	*/
	
	bigstring bs;
	
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
		
	return (IACreturnboolean (alertdialog (bs)));
	} /*alertdialogverb*/
	
	
static boolean confirmdialogverb (void) {
	
	/*
	opens up a modal dialog box with the string parameter displayed, wait for
	the user to click on OK before returning the value.	

	to support runtime menus, you wire your dialog boxes to IAC events if you want 
	script writers to be able to transparently talk to the user without Frontier 
	coming to the front.
	*/
	
	bigstring bs;
	
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
	
	return (IACreturnboolean (confirmdialog (bs)));
	} /*confirmdialogverb*/
	
	
static boolean askdialogverb (void) {
	
	/*
	to support runtime menus, you wire your dialog boxes to IAC events if you want 
	script writers to be able to transparently talk to the user without Frontier 
	coming to the front.
	*/
	
	bigstring prompt, answer;
	boolean fl;
	
	if (!IACgetstringparam (keyDirectObject, prompt))
		return (false);
		
	if (!IACgetstringparam ('dflt', answer))
		return (false);
		
	fl = askdialog (prompt, answer);
		
	IACglobals.event = IACglobals.reply; /*push the params onto the reply record*/
	
	if (!IACpushstringparam (answer, 'ansr'))
		return (false);
		
	return (IACreturnboolean (fl));
	} /*askdialogverb*/

	
static boolean settargetverb (void) {
	
	/*
	set the target of all subsequent verbs to the window named by the string
	parameter.  
	
	special case: set the target string to the empty string if you want verbs 
	to apply to the frontmost window.
	
	returns the title of the target window.
	*/
	
	boolean fl = true;
	bigstring bs;
	
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
	
	if (stringlength (bs) == 0) { /*special case*/
	
		if (setfrontglobals ())
			getappwindowtitle (app.appwindow, bs);
		}
		
	else {
		if (!setapptarget (bs)) 
			setstringlength (bs, 0);
		}
	
	copystring (bs, bstargetwindowname);
	
	return (IACreturnstring (bs));
	} /*settargetverb*/


static boolean gettargetverb (void) {
	
	/*
	returns the title of the target window.
	*/
	
	bigstring bs;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	getappwindowtitle (app.appwindow, bs);
		
	return (IACreturnstring (bs));
	} /*gettargetverb*/
	
	
static boolean getfileverb (void) {
	
	/*
	DW 11/7/95 -- a low-level verb that gets you the file attached to
	the current window, useful for calling back to in Clay Basket
	macros.
	*/
	
	tyfilespec fs;
	
	if (app.appwindow == nil) {
	
		IACreturnerror (1, "\pNo window is open.");
		
		return (false);
		}

	fs = (**app.appwindow).filespec;
 		
	return (IACreturnfilespec (&fs));
	} /*getfileverb*/
	

static boolean newwindowverb (void) {
	
	boolean fl = false;
	hdlappwindow appwindow;
	bigstring bs;
	
	setstringlength (bsalertstring, 0);
	
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
		
	if (stringlength (bs) == 0)
		getuntitledtitle (bs);
		
	if (findbywindowtitle (bs, &appwindow)) {
		
		alertdialog ("\pA window with that name is already open.");
		
		setstringlength (bs, 0); /*return the empty string to indicate error*/
		}
	else {
		if (newappwindow (bs, true)) {
			
			fl = true;
			
			copystring (bs, bstargetwindowname);
			}
		}
		
	return (IACreturnstring (bs));
	} /*newwindowverb*/
	
	
static boolean madechangesverb (void) {

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	return (IACreturnboolean ((**app.appwindow).flmadechanges));
	} /*madechangesverb*/


static boolean countwindowsverb (void) {

	return (IACreturnshort (countwindows ()));
	} /*countwindowsverb*/


static boolean selectallverb (void) {
	
	boolean fl;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	fl = selectallcommand ();
	
	return (IACreturnboolean (fl));
	} /*selectallverb*/


static boolean haveselectionverb (void) {
	
	boolean fl;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	fl = (*app.haveselectioncallback) ();
	
	return (IACreturnboolean (fl));
	} /*haveselectionverb*/


static boolean setfontverb (void) {
	
	bigstring bs;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	if (!IACgetstringparam (keyDirectObject, bs))
		return (false);
		
	appsetfont (bs);
	
	return (IACreturnboolean (true));
	} /*setfontverb*/
	
	
static boolean setfontsizeverb (void) {
	
	short fontsize;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	if (!IACgetshortparam (keyDirectObject, &fontsize))
		return (false);
		
	appsetfontsize (fontsize);
	
	return (IACreturnboolean (true));
	} /*setfontsizeverb*/
	
	
static boolean printwindowverb (void) {

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	return (IACreturnboolean (printappwindow (app.appwindow, false)));
	} /*printwindowverb*/


static boolean enablealertverb (void) {
	
	boolean fl;
	Boolean flenabled;
	
	if (!IACgetbooleanparam (keyDirectObject, &flenabled))
		return (false);
		
	fl = fldialogsenabled; /*return the original value, per spec*/
	
	fldialogsenabled = flenabled;
	
	return (IACreturnboolean (fl));
	} /*enablealertverb*/


static boolean quitverb (void) {
	
	return (IACreturnboolean (exitmainloop ()));
	} /*quitverb*/


static boolean geterrorstringverb (void) {
	
	return (IACreturnstring (bsalertstring));
	} /*geterrorstringverb*/
	
	
static boolean movewindowverb (void) {
	
	Rect r;
	WindowPtr w;
	Rect oldportrect;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	w = (**app.appwindow).macwindow;

	oldportrect = (*w).portRect;
	
	if (!IACgetrectparam (keyDirectObject, &r))
		return (false);
	
	moveappwindow (app.appwindow, r);
	
	adjustaftergrow (w, oldportrect); 
		
	return (IACreturnboolean (true));
	} /*movewindowverb*/
	
	
static boolean getfilepathverb (void) {
	
	tyfilespec fs;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
	
	fs = (**app.appwindow).filespec;
	
	return (IACreturnfilespec (&fs));
	} /*getfilepathverb*/
	
	
static boolean getpictverb (void) {

	Handle hpict;

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
	
	hpict = nil;
	
	(*app.getpictcallback) (&hpict); /*hpict is nil if it failed*/
	
	return (IACreturnbinary (hpict, 'BINA'));
	} /*getpictverb*/


static boolean gettextverb (void) {

	Handle htext;

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
	
	htext = nil;
		
	(*app.gettextcallback) (&htext); /*htext is nil if it failed*/
	
	return (IACreturntext (htext));
	} /*gettextverb*/


static boolean putpictverb (void) {

	Handle hpict;
	boolean fl;
	OSType binarytype;

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	if (!IACgetbinaryparam (keyDirectObject, &hpict, &binarytype))
		return (false);
		
	fl = (*app.putpictcallback) (hpict); 
	
	return (IACreturnboolean (fl));
	} /*putpictverb*/


static boolean puttextverb (void) {

	Handle htext;
	boolean fl;

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	if (!IACgettextparam (keyDirectObject, &htext))
		return (false);
		
	fl = (*app.puttextcallback) (htext);
	
	return (IACreturnboolean (fl));
	} /*puttextverb*/


static boolean getbinaryverb (void) {

	AEDesc desc;
	OSErr ec;

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
	
	desc.dataHandle = nil;
	
	desc.descriptorType = app.filetype;
	
	(*app.packcallback) (&desc.dataHandle);
	
	ec = AEPutParamDesc (IACglobals.reply, keyDirectObject, &desc);
	
	AEDisposeDesc (&desc);
	
	return (true);
	} /*getbinaryverb*/


static boolean putbinaryverb (void) {

	AEDesc result;
	OSErr ec;
	boolean fl;

	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	ec = AEGetParamDesc (IACglobals.event, keyDirectObject, app.filetype, &result);
	
	fl = (*app.unpackcallback) (result.dataHandle); 
	
	AEDisposeDesc (&result);
	
	return (IACreturnboolean (fl));
	} /*putbinaryverb*/
	
	
static void modelessfilter (EventRecord *ev) {
	
	switch ((*ev).what) {
		
		case iowaButtonHitEvent:
			(*ev).what = iowaRunScriptEvent;
			
			break;
		} /*switch*/
	} /*modelessfilter*/


boolean appruncard (Handle hpackedcard) { 
	
	tycardeventcallback cb;
	
	if (cardIsModal (hpackedcard))
		cb = &appscriptedmodalcardfilter;
	else
		cb = &modelessfilter;
	
	return (runCard (hpackedcard, true, 0, 0, cb));
	} /*appruncard*/


#ifdef appRunsCards

	static boolean runcardverb (void) {
	
		Handle hpackedcard;
		OSType binarytype;
	
		if (!IACgetbinaryparam (keyDirectObject, &hpackedcard, &binarytype))
			return (false);
			
		return (IACreturnboolean (appruncard (hpackedcard)));
		} /*runcardverb*/
	
#endif


static boolean getpagerectverb (void) {
	
	getprintinfo ();
	
	return (IACreturnrect (&app.printinfo.paperrect));
	} /*getpagerectverb*/
	

static boolean getwindowposverb (void) {
	
	Rect r;
	
	if (!settargetglobals ()) /*this verb requires an open window*/
		return (false);
		
	r = (**app.appwindow).windowrect;
	
	localtoglobalrect (&r);
		
	return (IACreturnrect (&r));
	} /*getwindowposverb*/
	
	
static pascal OSErr handlecustomverb (AppleEvent *event, AppleEvent *reply, long refcon) {

	IACglobals.event = event; 
	
	IACglobals.reply = reply;
	
	IACglobals.refcon = refcon;
	
	appprecallback ();
	
	(*app.iacmessagecallback) ();
	
	apppostcallback ();
	
	return (noErr);
	} /*handlecustomverb*/


static pascal OSErr handlefastverb (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	IACglobals.event = event; 
	
	IACglobals.reply = reply;
	
	IACglobals.refcon = refcon;
	
	appprecallback ();
	
	app.insyseventhandler = true;
	
	(*app.iacfastmessagecallback) ();
	
	app.insyseventhandler = false;
		
	apppostcallback ();
	
	return (noErr);
	} /*handlefastverb*/
	
	
static pascal OSErr handleapp1verb (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	/*
	called by Apple Event Manager when an 'app1' verb arrives.
	
	we always return noErr to the Apple Event Manager -- each verb processor
	may set the error number and string in the reply record by calling 
	IACreturnerror.
	*/
	
	OSType token;
	
	IACglobals.event = event; 
	
	IACglobals.reply = reply;
	
	IACglobals.refcon = refcon;
	
	token = IACgetverbtoken ();
		
	switch (token) {
	
		case newwindowtoken:
			newwindowverb (); break;
			
		case closewindowtoken:
			closewindowverb (); break;
		
		case savewindowtoken:
			saveasverb (); break;
			
		case revertwindowtoken:
			revertverb (); break;
			
		case openwindowtoken:
			openverb (); break;
		
		case editwindowtoken:
			editverb (); break;
		
		case gettargettoken:
			gettargetverb (); break;
				
		case selectwindowtoken:
			selectwindowverb (); break;
			
		case zoomwindowtoken:
			zoomwindowverb (); break;
			
		case scrollwindowtoken:
			scrollwindowverb (); break;
			
		case perftesttoken:
			perftestverb (); break;
			
		case alertdialogtoken:
			alertdialogverb (); break;
			
		case askdialogtoken:
			askdialogverb (); break;
			
		case confirmdialogtoken:
			confirmdialogverb (); break;
			
		case settargettoken:
			settargetverb (); break;
			
		case madechangestoken:
			madechangesverb (); break;
			
		case enabledialogtoken:
			enablealertverb (); break;
			
		case geterrorstringtoken:
			geterrorstringverb (); break;
			
		case nthwindowtoken:
			nthwindowverb (); break;
			
		case getfilepathtoken:
			getfilepathverb (); break;
			
		case movewindowtoken:
			movewindowverb (); break;
			
		case printwindowtoken:
			printwindowverb (); break;
			
		case getpagerecttoken:
			getpagerectverb (); break;
			
		case quittoken:
			quitverb (); break;
			
		case getwindowpostoken:
			getwindowposverb (); break;
			
		case countwindowstoken:
			countwindowsverb (); break;	
	
		case getpicttoken:
			getpictverb (); break;
		
		case gettexttoken:
			gettextverb (); break;
			
		case putpicttoken:
			putpictverb (); break;
			
		case puttexttoken:
			puttextverb (); break;
			
		case selectalltoken:
			selectallverb (); break;
			
		case haveselectiontoken:
			haveselectionverb (); break;
			
		case getwindowrecttoken:
			getwindowrectverb (); break;
			
		case setfonttoken:
			setfontverb (); break;
			
		case setfontsizetoken:
			setfontsizeverb (); break;
			
		case getbinarytoken:
			getbinaryverb (); break;
			
		case putbinarytoken:
			putbinaryverb (); break;
			
		case windowmessagetoken:
			windowmessageverb (); break;
			
		case getmemavailtoken:
			getmemavailverb (); break;
			
		case getfiletoken:
			getfileverb (); break;
		
		#ifdef appRunsCards 
		
			case runcardtoken:
				runcardverb (); break;
			
		#endif
			
		default:
			IACnothandlederror (); break;
		} /*switch*/
		
	return (noErr);
	} /*handleapp1verb*/
	

static pascal Boolean openfilespec (ptrfilespec pfs) {
	
	if ((*app.opendoccallback) (pfs)) /*the application handled the opendoc event*/
		return (true);
		
	#ifdef appRunsCards
	
		if (app.filetype != 'CARD') { /*it's Card Editor! or another app that opens cards*/
		
			tyfileinfo info;
			
			filegetinfo (pfs, &info);
			
			if (info.filetype == 'CARD') {
				
				short fnum;
				
				if (fileopen (pfs, &fnum)) {
					
					Handle h;
					
					if (filereadwholefile (fnum, &h)) 
						appruncard (h);
					
					fileclose (fnum);
					}
				
				return (true); /*we handled the file-open*/
				}
			}
	#endif
	
	/*DebugStr ((*pfs).name);*/
	
	if (!openappwindow (pfs)) {
		
		IACreturnerror (-1, bsalertstring);
		
		return (false);
		}
		
	return (true);
	} /*openfilespec*/
	

static pascal OSErr handleopen (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	IACglobals.event = event;
	
	IACglobals.reply = reply;
	
	IACglobals.refcon = refcon;
	
	return (IACdrivefilelist (&openfilespec));
	} /*handleopen*/
	
		
static pascal Boolean printfilespec (ptrfilespec pfs) {

	hdlappwindow appwindow;
	
	if (findbyfile (pfs, &appwindow)) 
		selectappwindow (appwindow);

	else {	
		if (!openappwindow (pfs)) {
			
			IACreturnerror (-1, bsalertstring);
			
			return (false);
			}
		}

	setfrontglobals (); /*event applies to frontmost window*/
	
	return (printappwindow (app.appwindow, false));
	} /*printfilespec*/
	

static pascal OSErr handleprint (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	IACglobals.event = event;
	
	IACglobals.reply = reply;
	
	IACglobals.refcon = refcon;
	
	return (IACdrivefilelist (&printfilespec));
	} /*handleprint*/
	
		
static pascal OSErr handlequit (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	exitmainloop ();
	
	return (noErr);
	} /*handlequit*/
	
	
static pascal OSErr handleopenapp (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	(*app.openappcallback) (); /*the application handled the opendoc event*/
	
	return (noErr);
	} /*handleopenapp*/
	
	
pascal Boolean appscriptcomplete (void) {
	
	bigstring bs;
	
	resetdirtyscrollbars ();
	 
	if (IACgetstringparam ('errs', bs)) {
		
		bigstring bsalert;
		
		copystring ("\pError in shared script. ", bsalert);
		
		pushstring (bs, bsalert);
		
		alertdialog (bsalert);
		}
	
	return (true);
	} /*appscriptcomplete*/
	
	
static long casereverser (long id) {
	
	/*
	used in setting up the system event handler for the "fast" verbs that the applet
	implements. we reverse the case of each of the alpha characters in the id. for
	example, 'DOCS' turns into 'docs'. And 'ÄMnU' turns into 'ÄmNu'. originally we
	had each applet declare its system handler in the class 'fast', but this wouldn't
	work if more than one applet was running at a time.
	*/

	char *p = (char *) &id;
	short i;
	
	for (i = 1; i <= 4; i++) {
	
		char ch = *p;
		
		if ((ch >= 'a') && (ch <= 'z'))
			ch -= 32;
		else
			if ((ch >= 'A') && (ch <= 'Z'))
				ch += 32;
		
		*p++ = ch;
		} /*for*/
		
	return (id);
	} /*casereverser*/
	

pascal short appwaitroutine (EventRecord *ev) {
	
	/*
	we're called by the IAC Toolkit while it's waiting for a reply to an 
	Apple Event. since we may be called while getting new shared menus, we 
	must not call handleevent for null events here to avoid re-entrancy.
	
	DW 12/22/92: we used to call appidle on receipt of a null event. this
	would wreak havoc in IOWA, so instead we just handle non-null events
	while waiting for a reply to the Apple Event.
	*/
	
	if ((*ev).what != nullEvent)
		appleteventdispatcher (ev);
		
	return (0);
	} /*appwaitroutine*/


boolean initappletwires (void) {
	
	setstringlength (bstargetwindowname, 0); /*no target window initially*/
	
	if (!IACinstallhandler (kCoreEventClass, kAEOpenApplication, (ProcPtr) &handleopenapp))
		goto error;
	
	if (!IACinstallhandler (kCoreEventClass, kAEOpenDocuments, (ProcPtr) &handleopen))
		goto error;
	
	if (!IACinstallhandler (kCoreEventClass, kAEPrintDocuments, (ProcPtr) &handleprint))
		goto error;
	
	if (!IACinstallhandler (kCoreEventClass, kAEQuitApplication, (ProcPtr) &handlequit))
		goto error;

	if (!IACinstallhandler (app1class, typeWildCard, (ProcPtr) &handleapp1verb))
		goto error;
	
	if (!IACinstallhandler (app.creator, typeWildCard, (ProcPtr) &handlecustomverb))
		goto error;
	
	if (!IACinstallsystemhandler (casereverser (app.creator), typeWildCard, (ProcPtr) &handlefastverb))
		goto error;

	IACglobals.waitroutine = appwaitroutine;
	
	return (true);
	
	error:
	
	alertdialog ("\pCouldn't install all the Apple Event handlers.");
	
	return (false);
	} /*initappletwires*/
	
	
