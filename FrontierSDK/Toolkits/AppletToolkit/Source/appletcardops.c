
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <iowacore.h>
#include "applet.h"
#include "appletmenu.h"
#include "appletmenuops.h"
#include "appletdate.h"
#include "appletasm.h"
#include "appletcardops.h"
#include "appletmain.h"
#include "appletfolder.h"
#include <ioapopup.h>
#include <ioacolorpopup.h>
#include <ioaedittext.h>



hdlcard currentcard = nil;

tydialoginfo appdialoginfo [ctdialogs];

static boolean flglobal; /*for general use*/

static short fontsizemap [] = {9, 10, 12, 14, 18, 24, 36, 48, 72};
	


boolean cardobjectexists (bigstring name) {
	
	hdlobject hobj;
	
	return (getObjectHandle ((hdlcard) currentcard, name, &hobj));
	} /*cardobjectexists*/
	
	
short getcardpopup (bigstring name) {
	
	hdlobject hobj;
	hdlpopupdata hp;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (0);
			
	hp = (hdlpopupdata) (**hobj).objectdata;
		
	return ((**hp).checkeditem);
	} /*getcardpopup*/
	
	
boolean getcardcolorpopup (bigstring name, RGBColor *rgb) {
	
	hdlobject hobj;
	hdlcolordata hdata;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
			
	hdata = (hdlcolordata) (**hobj).objectdata;
	
	*rgb = (**hdata).rgb;
	
	return (true);
	} /*getcardpopup*/
	
	
boolean getcardstring (bigstring name, void *pstring) {
	
	char *bs = (char *) pstring;
	Handle hvalue;
	
	setstringlength (bs, 0);
	
	if (!getObjectValue (currentcard, name, &hvalue))
		return (false);
	
	texthandletostring (hvalue, bs);
	
	return (true);
	} /*getcardstring*/


boolean getcardpassword (bigstring name, void *pstring) {
	
	bigstring bs;
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
	
	if (!gettextbehindbullets (hobj, bs))
		return (false);
	
	copystring (bs, pstring);
	
	return (true);
	} /*getcardpassword*/
	

boolean getcardflag (bigstring name, boolean *fl) {
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
		
	*fl = (**hobj).objectflag;
	
	return (true);
	} /*getcardflag*/
	
		
boolean getcardbit (bigstring name) {
	
	/*
	a special entrypoint that makes it easy to work with booleans that 
	are stored as bits
	*/
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
		
	return ((**hobj).objectflag);
	} /*getcardbit*/
	
	
static void invalobjecthandle (hdlobject hobj) {
	
	hdlcard hc = (hdlcard) (**hobj).owningcard;
	
	(**hobj).objectinval = true;
		
	(**hc).needsupdate = true;
	} /*invalobjecthandle*/
	
	
static boolean invalcardobject (bigstring name, boolean fl) {
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
		
	invalobjecthandle (hobj);
		
	return (true);
	} /*invalcardobject*/
	
	
boolean setradiotext (bigstring name, Handle htext) {
	
	/*
	radio buttons and checkboxes set their values differently, so we
	need to hack to change the text that's displayed next to them.
	
	works for checkboxes too.
	*/
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
	
	disposehandle ((**hobj).objectvalue);
	
	(**hobj).objectvalue = htext;
	
	invalobjecthandle (hobj);
	
	return (true);
	} /*setradiotext*/
	

boolean setcardstring (bigstring name, void *pstring) {
	
	char *value = (char *) pstring;
	bigstring currentvalue;
	Handle htext;
	
	getcardstring (name, currentvalue);
	
	if (equalstrings (value, currentvalue)) /*nothing to do*/
		return (true);
	
	if (!newtexthandle (value, &htext))
		return (false);
		
	return (setObjectValue (currentcard, name, htext));
	} /*setcardstring*/
	
	
boolean emptycardpassword (bigstring name) {
	
	hdlobject hobj;
	
	if (!setcardstring (name, "\p"))
		return (false);
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
	
	return (emptytextbehindbullets (hobj));
	} /*emptycardpassword*/
	

boolean setcardpassword (bigstring name, void *pstring) {
	
	char *value = (char *) pstring;
	bigstring currentvalue;
	hdlobject hobj;
	bigstring bsbullets;
	
	getcardpassword (name, currentvalue);
	
	if (equalstrings (value, currentvalue)) /*nothing to do*/
		return (true);
		
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
		
	if (!settextbehindbullets (hobj, value))
		return (false);
	
	filledstring ('¥', stringlength (value), bsbullets);
	
	return (setcardstring (name, bsbullets));
	} /*setcardpassword*/
	
	
boolean setcardflag (bigstring name, boolean fl) {
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
		
	if (fl) /*limit the possible values to 1 and 0*/
		fl = 1;
	else
		fl = 0;
	
	if ((**hobj).objectflag != fl) {
	
		(**hobj).objectflag = fl;
	
		invalobjecthandle (hobj);
		}
	
	return (true);
	} /*setcardflag*/
	
	
boolean setcardtmpbit (bigstring name, boolean fl) {
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
	
	(**hobj).objecttmpbit = fl;
	
	return (true);
	} /*setcardflag*/
	
	
boolean setcardappbit (bigstring name, short bitnum, boolean fl) {
	
	/*
	there are 4 appbits in each object. they can be used by the app
	to communicate between disparate modules.
	
	for example, when clayinfowindow.c sets up the comment editor,
	it sets appbit1, which edittext.c watches for. when it's set
	it doesn't select all the text when activating the object.
	*/
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
	
	switch (bitnum) {
		
		case 1:
			(**hobj).appbit1 = fl; 
			
			break;
			
		case 2:
			(**hobj).appbit2 = fl; 
			
			break;
			
		case 3:
			(**hobj).appbit3 = fl; 
			
			break;
			
		case 4:
			(**hobj).appbit4 = fl; 
			
			break;
		} /*switch*/
	
	return (true);
	} /*setcardflag*/
	
	
boolean setcardtextcolor (bigstring name, RGBColor *rgb) {
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
		
	if (!equalcolors (&(**hobj).objecttextcolor, rgb)) {
	
		(**hobj).objecttextcolor = *rgb;
	
		invalobjecthandle (hobj);
		}
	
	return (true);
	} /*setcardtextcolor*/
	
	
boolean setcardenable (bigstring name, boolean flenable) {
	
	/*
	DW 8/22/93: return true only if the enabled state changed.
	
	DW 2/15/95: if we're disabling the active text object, we no longer have
	an active text object. if we're enabling a text object, and we have no
	active text object, make it the active text object.
	*/
	
	hdlobject hobj;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
	
	if ((**hobj).objectenabled != flenable) {
	
		(**hobj).objectenabled = flenable;
	
		(**hobj).objectinval = true;
		
		(**currentcard).needsupdate = true;
		
		if ((hobj == (**currentcard).activetextobject) && (!flenable)) { /*DW 2/15/95*/
			
			hdlcard oldiowadata = iowadata;
			
			iowadata = currentcard;
			
			clearactivetextobject ();
			
			iowadata = oldiowadata;
			}
			
		if ((flenable) && ((**currentcard).activetextobject == nil)) { /*DW 2/15/95*/
			
			if ((**hobj).objecttype == edittexttype) {
			
				hdlcard oldiowadata = iowadata;
			
				iowadata = currentcard;
			
				setactivetextobject (hobj);
			
				iowadata = oldiowadata;
				}
			}
		
		return (true);
		}
	
	return (false);
	} /*setcardenable*/
	
	
static boolean disablevisit (hdlobject h) {
	
	if ((**h).objectenabled != flglobal) {
	
		(**h).objectenabled = flglobal;
	
		(**h).objectinval = true;
		}
		
	return (true);
	} /*disablevisit*/
	
	
boolean enableallcardobjects (boolean flenabled) {
	
	/*
	should go thru the component interface. DW 9/12/93
	*/
	
	flglobal = flenabled;
	
	visitobjects ((**currentcard).objectlist, &disablevisit);
	
	(**currentcard).needsupdate = true;
	
	return (true);
	} /*enableallcardobjects*/
	
	
boolean setcardpopup (bigstring name, bigstring displaystring, Handle semicolonstring, short checkeditem) {
	
	hdlobject hobj;
	hdlpopupdata hp;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
			
	hp = (hdlpopupdata) (**hobj).objectdata;
		
	disposehandle ((**hp).hmenuitems);
	
	(**hp).hmenuitems = semicolonstring;
	
	copystring (displaystring, (**hp).bspopup);
	
	(**hp).checkeditem = checkeditem;
	
	return (true);
	} /*setcardpopup*/
	
	
static boolean getnthpopupitem (hdlobject h, short n, bigstring bs) {
	
	/*
	get the nth string from the popup menu object. it's a bit slower to load
	up the menu using calls to this guy, but it's simpler, and it's nice to
	have this mess confined to one routine.
	
	n is 1-based.
	*/
	
	hdlpopupdata hdata = (hdlpopupdata) (**h).objectdata;
	Handle htext = (**hdata).hmenuitems;
	short i;
	short ctchars;
	short itemnum = 1;
	
	ctchars = GetHandleSize (htext);
	
	setstringlength (bs, 0);
		
	for (i = 0; i < ctchars; i++) {
		
		char ch = (*htext) [i];
		
		if (ch == ';') {
			
			if (itemnum == n)
				return (true);
				
			itemnum++;
			
			setstringlength (bs, 0);
			}
		else
			pushchar (ch, bs);
		} /*for*/
	
	return ((itemnum == n) && (stringlength (bs) > 0));
	} /*getnthpopupitem*/
	
		
boolean setcardpopupcheckeditem (bigstring name, short checkeditem) {

	hdlobject hobj;
	hdlpopupdata hp;

	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
			
	hp = (hdlpopupdata) (**hobj).objectdata;
	
	(**hp).checkeditem = checkeditem;
	
	getnthpopupitem (hobj, checkeditem, (**hp).bspopup);
	
	invalobjecthandle (hobj);
	
	return (true);
	} /*setcardpopupcheckeditem*/
	
	
boolean getcardpopupstring (bigstring name, bigstring popupstring) {

	hdlobject hobj;
	hdlpopupdata hp;

	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
			
	hp = (hdlpopupdata) (**hobj).objectdata;
	
	copystring ((**hp).bspopup, popupstring);
	
	return (true);
	} /*getcardpopupstring*/
	
	
boolean setcardpopupfrommenu (bigstring name, MenuHandle hmenu, short checkeditem) {
	
	short ct = countmenuitems (hmenu), i;
	Handle semicolonstring;
	bigstring bsdisplay;
	
	setstringlength (bsdisplay, 0);
	
	newclearhandle (0, &semicolonstring);
	
	for (i = 1; i <= ct; i++) {
		
		bigstring bs;
		
		getmenuitem (hmenu, i, bs);
		
		if (i == checkeditem)
			copystring (bs, bsdisplay);
			
		pushchar (';', bs);
		
		pushtexthandle (bs, semicolonstring);
		} /*for*/
		
	return (setcardpopup (name, bsdisplay, semicolonstring, checkeditem));
	} /*setcardpopupfrommenu*/
	
	
typedef struct tysetcardpopupfromfolderrecord {
	
	Handle hsemistring;
	
	short checkeditem;
	
	bigstring bsdisplay;
	
	bigstring fnamechecked;
	
	short ix;
	} tysetcardpopupfromfolderrecord;
	
	
static boolean setpopupfoldervisit (bigstring fname, tyfileinfo *info, long refcon) {
	
	tysetcardpopupfromfolderrecord *rec = (tysetcardpopupfromfolderrecord *) refcon;
	
	if (!pushtexthandle (fname, (*rec).hsemistring))
		return (false);
		
	if (!pushtexthandle ("\p;", (*rec).hsemistring))
		return (false);
	
	if (unicaseequalstrings (fname, (*rec).fnamechecked)) {
		
		copystring (fname, (*rec).bsdisplay);
		
		(*rec).checkeditem = (*rec).ix;
		}
		
	(*rec).ix++;
		
	return (true);
	} /*setpopupfoldervisit*/


boolean setcardpopupfromfolder (bigstring name, ptrfilespec pfsfolder, bigstring fnamechecked) {

	tysetcardpopupfromfolderrecord rec; 
	
	if (!newtexthandle ("\p", &rec.hsemistring))
		return (false);
		
	copystring (fnamechecked, rec.fnamechecked);
	
	rec.checkeditem = 1; /*default, in case there's no match*/
	
	if (!folderloop (pfsfolder, false, &setpopupfoldervisit, (long) &rec)) {
		
		disposehandle (rec.hsemistring);
		
		return (false);
		}
		
	return (setcardpopup (name, rec.bsdisplay, rec.hsemistring, rec.checkeditem));
	} /*setcardpopupfromfolder*/
	
	
boolean setcardicon (bigstring name, short align, short ixlabel, Handle iconsuite, tyiconclickcallback iconclick) {
	
	/*
	allows you to set the customizable features of an IOA icon object. if iconsuite
	is nil, no problem, then the component uses the resnum to get the icon.
	*/
	
	hdlobject hobj;
	hdlicondata hdata;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
			
	hdata = (hdlicondata) (**hobj).objectdata;
	
	(**hdata).align = align;
	
	(**hdata).ixlabel = ixlabel;
	
	(**hdata).iconsuite = iconsuite;
	
	(**hdata).iconclick = iconclick;
	
	invalobjecthandle (hobj);
	
	return (true);
	} /*setcardicon*/
	
	
boolean setcardcolorpopup (bigstring name, RGBColor *rgb) {
	
	hdlobject hobj;
	hdlcolordata hdata;
	
	if (!getObjectHandle ((hdlcard) currentcard, name, &hobj))
		return (false);
			
	hdata = (hdlcolordata) (**hobj).objectdata;
	
	(**hdata).rgb = *rgb;
	
	invalobjecthandle (hobj);
	
	return (true);
	} /*setcardcolorpopup*/
	
	
short getfontsizepopup (bigstring name) {
	
	return (fontsizemap [getcardpopup (name) - 1]);
	} /*getfontsizepopup*/
	
	
void setfontsizepopup (bigstring name, short size) {
	
	short ix = 1, i;
	
	for (i = 0; i < sizeof (fontsizemap); i++) {
	
		if (size == fontsizemap [i]) {
			
			ix = i;
			
			break;
			}
		} /*for*/
		
	setcardpopupcheckeditem (name, ix + 1);
	} /*setfontsizepopup*/
	
	
void setfontpopup (bigstring name, diskfontstring fontname) {
	
	short ct = countmenuitems (hdlfontmenu), i;
	bigstring bsfontname;
	
	copystring (fontname, bsfontname);
	
	for (i = 1; i <= ct; i++) {
		
		bigstring bs;
		
		getmenuitem (hdlfontmenu, i, bs);
		
		if (fastunicaseequalstrings (bsfontname, bs)) {
		
			setcardpopupcheckeditem (name, i);
			
			return;
			}
		} /*for*/
	} /*setfontpopup*/
	
	
void getfontpopup (bigstring name, diskfontstring fontname) {
	
	bigstring bs;
	
	getmenuitem (hdlfontmenu, getcardpopup (name), bs);
	
	copystring (bs, fontname);
	} /*getfontpopup*/
	
	
void updatecard (void) {
	
	/*
	the filter routine has changed the value of some object, but isn't ready
	to return yet (for example, th progress message in the find command).
	*/
	
	updateCard (currentcard);
	} /*updatecard*/
	
	
boolean getcardnumber (bigstring name, long *num) {
	
	bigstring bs;
	boolean flerror;
	
	if (!getcardstring (name, bs))
		return (false);
	
	stringtonumber (bs, num, &flerror);
	
	return (!flerror);
	} /*getcardnumber*/
	

boolean getcardshort (bigstring name, short *num) {
	
	long x;
	
	if (!getcardnumber (name, &x))
		return (false);
		
	*num = (short) x;
	
	return (true);
	} /*getcardshort*/
	
	
boolean setcardnumber (bigstring name, long num) {
	
	bigstring bs;
	
	NumToString (num, bs);
	
	return (setcardstring (name, bs));
	} /*setcardnumber*/
	
		
boolean setcardtexthandle (bigstring name, Handle hvalue) {
	
	Handle h;
	
	if (!copyhandle (hvalue, &h))
		return (false);
		
	return (setObjectValue (currentcard, name, h));
	} /*setcardtexthandle*/
	

boolean replacecardtexthandle (bigstring name, Handle *hvalue) {
	
	/*
	a special entrypoint for Card Editor -- hvalue already contains
	a value, we replace it only if the value changed.
	*/
	
	Handle hnewvalue;
	
	if (!getObjectValue (currentcard, name, &hnewvalue))
		return (false);
		
	if (equalhandles (hnewvalue, *hvalue)) 
		return (true);
	
	disposehandle (*hvalue);
	
	return (copyhandle (hnewvalue, hvalue));
	} /*replacecardtexthandle*/
	
	
boolean getcardtexthandle (bigstring name, Handle *hvalue) {
	
	Handle hnewvalue;
	
	disposehandle (*hvalue); /*DW 9/20/95, does nothing if it's nil, assume all handles are init'd to nil*/
	
	*hvalue = nil;
	
	if (!getObjectValue (currentcard, name, &hnewvalue))
		return (false);
		
	return (copyhandle (hnewvalue, hvalue));
	} /*getcardtexthandle*/
	
	
boolean setcardrect (bigstring name, Rect r) {
	
	bigstring bs;
	
	setstringlength (bs, 0);
	
	pushlong (r.top, bs); pushchar (',', bs);
	
	pushlong (r.left, bs); pushchar (',', bs);
	
	pushlong (r.bottom, bs); pushchar (',', bs);
	
	pushlong (r.right, bs); 
	
	return (setcardstring (name, bs));
	} /*setcardrect*/
	
	
boolean getcardrect (bigstring name, Rect *r) {
	
	bigstring bs;
	long nums [4];
	short ixnums = 0;
	short i;
	
	if (!getcardstring (name, bs))
		return (false);
		
	for (i = 1; i <= 4; i++) {
		
		bigstring bsnum;
		
		setstringlength (bsnum, 0);
		
		while (true) {
			
			char ch;
			
			if (stringlength (bs) == 0)
				break;
				
			ch = bs [1];
				
			deletestring (bs, 1, 1);
			
			if (ch == ',')
				break;
				
			pushchar (ch, bsnum);
			} /*while*/
			
		StringToNum (bsnum, &nums [ixnums++]);
		} /*for*/
		
	if (ixnums != 4)
		return (false);
	
	(*r).top = nums [0];
	
	(*r).left = nums [1];
	
	(*r).bottom = nums [2];
	
	(*r).right = nums [3];
	
	return (true);
	} /*getcardrect*/
	
	
boolean setcarddate (bigstring name, tydaterecord date) {
	
	bigstring bs;
	
	appletdatetostring (date, bs);
	
	return (setcardstring (name, bs));
	} /*setcarddate*/
	
	
boolean getcarddate (bigstring name, tydaterecord *date) {
	
	bigstring bs;
	
	if (!getcardstring (name, bs))
		return (false);
	
	return (stringtoappletdate (bs, date));
	} /*getcarddate*/
	
	
static short getindex (short resnum) {
	
	short ix = resnum - 128;
	
	assert (ix < ctdialogs); /*need to increase array size or bad resnum*/
		
	return (ix);
	} /*getindex*/
	
	
static void getdialoginfo (short resnum, tydialoginfo *info) {
	
	*info = appdialoginfo [getindex (resnum)];
	} /*getdialoginfo*/
	
	
static void setdialoginfo (short resnum, tydialoginfo *info) {
	
	appdialoginfo [getindex (resnum)] = *info;
	} /*setdialoginfo*/
	
	
void appcarddefaulteventhandler (EventRecord *ev) {
	
	/*
	a basic event handler for modal cards, scripted or otherwise.
	
	only pass back events to the Applet Toolkit main event loop
	that it has a right to be interested in when a modal card is
	in front of all its windows.
	*/
	
	switch ((*ev).what) {
		
		case updateEvt: case activateEvt: case osEvt: case diskEvt:
			appleteventhandler (ev);
			
			break;
		} /*switch*/
	} /*appcarddefaulteventhandler*/
	

void appscriptedmodalcardfilter (EventRecord *ev) {
	
	/*
	a basic filter routine for a scripted modal card.
	
	at least a couple of place need exactly this filter,
	so it made sense to put it here so they don't have
	to re-invent the wheel.
	*/
	
	switch ((*ev).what) {
		
		case iowaButtonHitEvent:
			(*ev).what = iowaRunScriptEvent;
			
			break;
			
		default:
			appcarddefaulteventhandler (ev);
			
			break;
		} /*switch*/
	} /*appscriptedmodalcardfilter*/
	

static boolean getcardresource (short id, Handle *hpackedcard) {
	
	Handle h;
	
	h = GetResource ('CARD', id);
	
	if (h == nil)
		return (false);
	
	DetachResource (h);
	
	*hpackedcard = h;
	
	return (true);
	} /*getcardresource*/
	
	
void appopencardresource (short id, tycardeventcallback filter) {
	
	Handle hpackedcard;
	tydialoginfo info;
	
	apppostcallback (); 
	
	getdialoginfo (id, &info);
	
	if (getcardresource (id, &hpackedcard)) 
		runModelessCard (hpackedcard, false, info.top, info.left, filter);

	appprecallback ();
	} /*appopencardresource*/
	

void apprunmodalresource (short id, tycardeventcallback filter) {
	
	Handle hpackedcard;
	tydialoginfo info;
	
	dialogmenuenabler (true); /*disable all menus but the Edit menu*/
	
	getdialoginfo (id, &info);
	
	if (getcardresource (id, &hpackedcard)) 
		runModalCard (hpackedcard, false, info.top, info.left, filter);
	
	dialogmenuenabler (false);
	} /*apprunmodalresource*/
	
	
void appsavedialoginfo (short id) {

	WindowPtr w = (**currentcard).drawwindow;
	Rect r = (*w).portRect;
	tydialoginfo info;
	
	pushmacport (w);
	
	localtoglobalrect (&r);
	
	popmacport ();
	
	info.top = r.top;
	
	info.left = r.left;
	
	setdialoginfo (id, &info);
	} /*appsavedialoginfo*/
	

void appcardtofront (hdlcard hcard) {
	
	SelectWindow ((**hcard).drawwindow);
	} /*appcardtofront*/
	
	
	