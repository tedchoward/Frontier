
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"
#include "appletquickdraw.h"
#include "appletops.h"
#include "appletresource.h"
#include "appletmain.h"
#include "appletdialogs.h"


/*
7/25/94 dmb: updated for Universal Headers, PowerPC
*/


/*
flag to control whether the user is interacting, or we're responding to an
interapplication message. the error string is saved -- to be accessed by the
GetErrorString verb.
*/
	boolean fldialogsenabled = true;
	bigstring bsalertstring;
	



void setdialogtext (DialogPtr pdialog, short itemnumber, bigstring bs) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
		
	SetIText (itemhandle, bs); 
	} /*setdialogtext*/
	
	
void getdialogtext (DialogPtr pdialog, short itemnumber, bigstring bs) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
				
	GetIText (itemhandle, bs); 
	} /*getdialogtext*/


void selectdialogtext (DialogPtr pdialog, short itemnumber) {
	
	SelIText (pdialog, itemnumber, 0, infinity); /*select all text*/
	} /*selectdialogtext*/		


void disabledialogitem (DialogPtr pdialog, short itemnumber) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;

	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
	
	if (itemtype < itemDisable) { /*it is enabled, disable it*/
	
		SetDItem (pdialog, itemnumber, itemtype + itemDisable, itemhandle, &itemrect);
		
		grayrect (itemrect);
		}
	} /*disabledialogitem*/
	

void enabledialogitem (DialogPtr pdialog, short itemnumber) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;

	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
	
	if (itemtype >= itemDisable) /*it is disabled, enable it*/
	
		SetDItem (pdialog, itemnumber, itemtype - itemDisable, itemhandle, &itemrect);
	} /*disabledialogitem*/
	

void dialoggetobjectrect (DialogPtr pdialog, short objectnumber, Rect *r) {

	short itemtype;
	Handle itemhandle;
	
	GetDItem (pdialog, objectnumber, &itemtype, &itemhandle, r);
	} /*dialoggetobjectrect*/
	
	
void parsedialogstring (bigstring bs) {
	
	register short i;
	
	for (i = 1; i <= stringlength (bs); i++) {
		
		if (bs [i] == '®')
			bs [i] = (char) 13;
		} /*for*/
	} /*parsedialogstring*/
	
	
static short dialogcountitems (DialogPtr pdialog) {
	
	/*
	amazingly we have to klooge around to find out how many items there are
	in a dialog's item list.
	*/
	
	register Handle h;
	
	h = (*(DialogPeek) pdialog).items;
	
	return ((**(short **) h) + 1); /*magic incantation -- Boozer says this works*/
	} /*dialogcountitems*/


static boolean dialogitemisedittext (DialogPtr pdialog, short item) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	return (itemtype & editText); 
	} /*dialogitemisedittext*/
	

static dialoghasedititems (DialogPtr pdialog) {
	
	register short i;
	register short ctitems;
	
	ctitems = dialogcountitems (pdialog);
	
	for (i = 1; i <= ctitems; i++) 
		if (dialogitemisedittext (pdialog, i))
			return (true);
	
	return (false);
	} /*dialoghasedititems*/


static void dialoggetbuttonstring (DialogPtr pdialog, short item, bigstring bs) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;

	GetDItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	copystring ((**(ControlHandle) itemhandle).contrlTitle, bs);
	} /*dialoggetbuttonstring*/


static void dialogsetbuttonstring (DialogPtr pdialog, short item, bigstring bs) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;

	GetDItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	SetCTitle ((ControlHandle) itemhandle, bs);
	} /*dialogsetbuttonstring*/


static void setdefaultitem (DialogPtr pdialog, short defaultitem) {
	
	(*(DialogPeek) pdialog).aDefItem = defaultitem; /*filter will bolden this*/
	} /*setdefaultitem*/


void boldenbutton (DialogPtr pdialog, short itemnumber) {

	/*
	draw a thick black ring around the OK button in the dialog.  
	*/
	
	PenState savePen;
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	pushmacport (pdialog);
	
	GetPenState (&savePen); /*save the old pen state*/
	
	GetDItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the item’s rect*/
	
	InsetRect (&itemrect, -4, -4);
	
	PenSize (3, 3); /*make the pen fatter*/
	
	FrameRoundRect (&itemrect, 16, 16); /*draw the ring*/

	SetPenState (&savePen); /*restore the pen state*/
	
	popmacport ();
	} /*boldenbutton*/


static boolean dialogitemtypeiscontrol (short itemtype) {
	
	register short x;
	
	x = itemtype % itemDisable; /*ignore enabledness*/
	
	return ((x >= ctrlItem) && (x <= (ctrlItem + resCtrl))); 
	} /*dialogitemtypeiscontrol*/


static boolean dialogitemisbutton (DialogPtr pdialog, short item) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	if (item <= 0)
		return (false);
	
	GetDItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	return (dialogitemtypeiscontrol (itemtype));
	} /*dialogitemisbutton*/
	

static void highlightdialogbutton (DialogPtr pdialog, short itemnumber, boolean flon) {
	
	register DialogPtr p = pdialog;
	register short val;
	short itemtype;
	Handle itemhandle;
	Rect itembox;
	
	if (pdialog == nil) /*defensive driving*/
		return;
		
	GetDItem (p, itemnumber, &itemtype, &itemhandle, &itembox);
	
	if (flon)
		val = inButton;
	else
		val = 0;
	
	HiliteControl ((ControlHandle) itemhandle, val); 
	} /*highlightdialogbutton*/


static boolean dialogitemisenabled (DialogPtr pdialog, short item) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	return ((itemtype & itemDisable) == 0);
	} /*dialogitemisenabled*/


static boolean dialogsimulatehit (DialogPtr pdialog, short item) {
	
	if (!(dialogitemisbutton (pdialog, item) && dialogitemisenabled (pdialog, item)))
		return (false);
	
	highlightdialogbutton (pdialog, item, true);
	
	delayticks (8);
	
	highlightdialogbutton (pdialog, item, false);
	
	return (true);
	} /*dialogsimulatehit*/


static boolean dialogmapkeystroke (DialogPtr pdialog, bigstring bsmap, short *item) {
	
	/*
	map a keystroke onto a dialog item.  if no match, return false.
	
	for buttons, if the keystroke matches the first character of the button name,
	we return true with that item.  makes your choice of button names strategic.
	
	to get the number of items in the list, pull the first two bytes out of the
	dialog's itemhandle and add one.  an obvious problem if Apple changes the format
	of an item list handle.
	
	10/10/91 dmb: now take string to map instead of a character.  if bsmap is 
	longer than a single character, we look for an exact match (still ignoring case).
	
	2/7/92 dmb: use dialoghasedititems instead of looping through item list.
	*/
	
	register short i;
	register boolean flcmdkeyrequired;
	short ctitems;
	bigstring bs;
	boolean flsinglechar;
	
	alllower (bsmap); /*search is unicase*/
	
	ctitems = dialogcountitems (pdialog);
	
	flsinglechar = stringlength (bsmap) == 1;
	
	if (flsinglechar) { /*mapping a single character -- check command key*/
		
		flcmdkeyrequired = dialoghasedititems (pdialog);
		
		if (flcmdkeyrequired && !cmdkeydown ())
			return (false);
		}
	
	for (i = 1; i <= ctitems; i++) {
		
		if (dialogitemisbutton (pdialog, i)) {
			
			dialoggetbuttonstring (pdialog, i, bs);
			
			if (stringlength (bs) > 0) {
				
				if (flsinglechar)
					setstringlength (bs, 1);
				
				alllower (bs);
				
				if (equalstrings (bs, bsmap)) {
					
					*item = i;
					
					return (true);
					}
				}	
			}
		} /*for*/
	
	return (false); /*no mapping for the character*/
	} /*dialogmapkeystroke*/


DialogPtr newmodaldialog (short id, short defaultitem) {
	
	register DialogPtr pdialog;
	
	cometofront ();
	
	arrowcursor ();
	
	pdialog = GetNewDialog (id, nil, (DialogPtr) -1L);
	
	if (pdialog == nil) 
		return (nil);
		
	centerwindow (pdialog, quickdrawglobal (screenBits).bounds);
	
	ShowWindow (pdialog);	
	
	if (defaultitem > 0)
		boldenbutton (pdialog, defaultitem); 
	
	setdefaultitem (pdialog, defaultitem);
	
	return (pdialog);
	} /*newmodaldialog*/
	

static pascal Boolean modaldialogcallback (DialogPtr pdialog, EventRecord *ev, short *item) {
 	
 	/*
 	standard dialog & alert event filtering.
 	
 	10/3/90 dmb: check result of dialogsimulatehit (which now checks enabledness). 
 	only process background tasks on null events; dialog events have priority. 
 	also, get default item instead of assuming item 1, and handle boldening.
 	
 	8/26/91 dmb: after backgrounding, make sure we're still in front.  otherwise, 
 	the dialog manager will crash
 	
 	10/14/91 dmb: set shellevent.when to deter overzealous agent processing
 	*/
 	
 	short defaultitem = (*(DialogPeek) pdialog).aDefItem; /*was set by newmodaldialog*/
 	short whatevent = (*ev).what;
 	bigstring bsbutton;
 	
 	/*put callback here*/
 	
	switch (whatevent) {
	
		case keyDown: case autoKey: {
			
 			char chkb;
 			boolean flcmdkey;
 	
			chkb = (*ev).message & charCodeMask;
			
			flcmdkey = ((*ev).modifiers & cmdKey) == cmdKey;
			
			if ((chkb == chreturn) || (chkb == chenter)) { /*user hit return or enter*/
				
				if (defaultitem == 0)
					return (0);
				
				if (!dialogsimulatehit (pdialog, defaultitem))
					return (0);
				
				*item = defaultitem;
				
				return (-1); /*the dialog manager's version of true*/
				}
			
			if ((chkb == chescape) || ((chkb == '.') && flcmdkey)) { /*escape or cmd-period*/
				
				getresourcestring (cancelbuttonstring, bsbutton);
				}
			else
				setstringwithchar (chkb, bsbutton);
			
			if (dialogmapkeystroke (pdialog, bsbutton, item)) {
				
				if (MenuKey ((*ev).message & charCodeMask)) { /*will be handled by system*/
					
					HiliteMenu (0);
					
					break;
					}
				
				if (!dialogsimulatehit (pdialog, *item))
					return (0);
				
				return (-1);
				}
			
			break; /*keydown or autokey*/
			}
		
		case updateEvt:
		
			if ((defaultitem > 0) && ((*ev).message == (long) pdialog))
				boldenbutton (pdialog, defaultitem);
			
			break;
		
		case nullEvent:
			SelectWindow (pdialog); /*make sure no one has screwed around with us*/
			
			break;
		
		} /*switch*/
	
	return (0); /*the dialog manager's version of false*/
	} /*modaldialogcallback*/


#if GENERATINGCFM

	static RoutineDescriptor modaldialogcallbackDesc = BUILD_ROUTINE_DESCRIPTOR (uppModalFilterProcInfo, modaldialogcallback);
	
	#define modaldialogcallbackUPP &modaldialogcallbackDesc
	
#else

	#define modaldialogcallbackUPP &modaldialogcallback

#endif



short runmodaldialog (void) {
	
	short itemnumber;
	
	ModalDialog (modaldialogcallbackUPP, &itemnumber);
	
	return (itemnumber);
	} /*runmodaldialog*/
	
	
boolean alertdialog (bigstring bs) {
	
	#define alertdialogid 128 
	#define alertokitem 1
	#define alertmsgitem 3
	register DialogPtr pdialog;
	
	if (!fldialogsenabled) {
		
		copystring (bs, bsalertstring);
		
		return (true);
		}

	sysbeep;
	
	parsedialogstring (bs);
	
	if ((pdialog = newmodaldialog (alertdialogid, alertokitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, alertmsgitem, bs);
	
	ShowWindow (pdialog);
	
	runmodaldialog ();
	
	DisposDialog (pdialog);
	
	#ifdef fldebug
		if (optionkeydown ())
			DebugStr ("\pHello");
	#endif
	
	return (true);
	} /*alertdialog*/
	
	
short savedialog (bigstring fname) {
	
	/*
	as the user to save changes before closing the file.
	
	we return 1 if they chose Yes, 2 for No, 3 for Cancel.
	*/
	
	#define savedialogid 131 
	#define saveyesitem 1
	#define savenoitem 2
	#define savecancelitem 3
	#define savemsgitem 4
	register DialogPtr pdialog;
	bigstring bs;
	short choice;
	
	copystring ("\pSave “", bs);
	
	pushstring (fname, bs);
	
	pushstring ("\p” before closing?", bs);
	
	if ((pdialog = newmodaldialog (savedialogid, saveyesitem)) == nil)
		return (1);
	
	setdialogtext (pdialog, savemsgitem, bs);
	
	ShowWindow (pdialog);
	
	switch (runmodaldialog ()) {
		
		case saveyesitem:
			choice = 1;
			
			break;
			
		case savenoitem:
			choice = 2;
			
			break;
			
		case savecancelitem:
			choice = 3;
			
			break;
			
		} /*switch*/
	
	DisposDialog (pdialog);
	
	return (choice);
	} /*savedialog*/


boolean confirmdialog (bigstring bs) {
	
	#define confirmdialogid 129 
	#define confirmokitem 1
	#define confirmcancelitem 2
	#define confirmmsgitem 3
	register DialogPtr pdialog;
	register short item;
	
	parsedialogstring (bs);
	
	if ((pdialog = newmodaldialog (confirmdialogid, confirmokitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, confirmmsgitem, bs);
	
	ShowWindow (pdialog);
	
	item = runmodaldialog ();
	
	DisposDialog (pdialog);
	
	return (item == confirmokitem);
	} /*confirmdialog*/


boolean yesnodialog (bigstring bs) {
	
	#define yesnodialogid 129 
	#define yesnookitem 1
	#define yesnocancelitem 2
	#define yesnomsgitem 3
	register DialogPtr pdialog;
	register short item;
	
	parsedialogstring (bs);
	
	if ((pdialog = newmodaldialog (yesnodialogid, yesnookitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, yesnomsgitem, bs);
	
	dialogsetbuttonstring (pdialog, yesnookitem, "\pYes");
	
	dialogsetbuttonstring (pdialog, yesnocancelitem, "\pNo");
	
	ShowWindow (pdialog);
	
	item = runmodaldialog ();
	
	DisposDialog (pdialog);
	
	return (item == yesnookitem);
	} /*yesnodialog*/


short threewaydialog (bigstring bsprompt, bigstring leftbutton, bigstring midbutton, bigstring rightbutton) {
	
	#define threewaydialogid 131 
	#define leftbuttonitem 3
	#define midbuttonitem 2
	#define rightbuttonitem 1
	#define messageitem 4
	register DialogPtr pdialog;
	register short item;
	
	parsedialogstring (bsprompt);
	
	if ((pdialog = newmodaldialog (threewaydialogid, rightbuttonitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, messageitem, bsprompt);
	
	dialogsetbuttonstring (pdialog, leftbuttonitem, leftbutton);
	
	dialogsetbuttonstring (pdialog, midbuttonitem, midbutton);
	
	dialogsetbuttonstring (pdialog, rightbuttonitem, rightbutton);
	
	ShowWindow (pdialog);
	
	item = runmodaldialog ();
	
	DisposDialog (pdialog);
	
	switch (item) {
		
		case leftbuttonitem:
			return (3);
			
		case midbuttonitem:
			return (2);
			
		case rightbuttonitem:
		default:
			return (1);
		} /*switch*/
	} /*threewaydialog*/
	
	
boolean askdialog (bigstring bsprompt, bigstring bsanswer) {
	
	/*
	put up the standard "ask" dialog, with the provided prompt and return
	true if the user clicked on ok.  the answer is in bsanswer.
	*/
	
	#define askdialogid 130
	#define askokitem 1
	#define askcancelitem 2
	#define askpromptitem 3
	#define askansweritem 4	
	register DialogPtr pdialog;
	register short itemnumber;
	
	ParamText (bsprompt, emptystring, emptystring, emptystring);
	
	if ((pdialog = newmodaldialog (askdialogid, askokitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, askansweritem, bsanswer);
	
	selectdialogtext (pdialog, askansweritem);
	
	ShowWindow (pdialog);
	
	itemnumber = runmodaldialog ();
	
	getdialogtext (pdialog, askansweritem, bsanswer);
	
	DisposDialog (pdialog);
	
	return (itemnumber == askokitem);
	} /*askdialog*/


boolean oserror (OSErr errcode) {
	
	bigstring bs;
	
	if (errcode == noErr)
		return (false);
	
	copystring ("\pMacintosh OS Error = ", bs);
	
	pushint (errcode, bs);
	
	alertdialog (bs);
	
	return (true);
	} /*oserror*/
	
	
