
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <iac.h>
#include <ioa.h>
#include <ioaedittext.h>
#include <appletkb.h>



typedef struct typackedtextdata {
	
	short versionnumber;
	
	boolean bullets;
	
	long lenrealtext;
	
	boolean numericonly: 1; /*if true, only allow 0-9 to be entered*/
	
	boolean reservedbit1: 1;
	
	boolean reservedbit2: 1;
	
	boolean reservedbit3: 1;
	
	boolean reservedbit4: 1;
	
	boolean reservedbit5: 1;
	
	boolean reservedbit6: 1;
	
	boolean reservedbit7: 1;
	
	boolean reservedbit8: 1;
	
	boolean reservedbit9: 1;
	
	boolean reservedbit10: 1;
	
	boolean reservedbit11: 1;
	
	boolean reservedbit12: 1;
	
	boolean reservedbit13: 1;
	
	boolean reservedbit14: 1;
	
	boolean reservedbit15: 1;
	
	char waste [8];
	} typackedtextdata;
	


static void disposehandle (Handle h) {
	
	if (h != nil)
		DisposHandle (h);
	} /*disposehandle*/
	
	
static boolean newtextdata (hdltextdata *hdata) {

	hdltextdata h;
	
	h = (hdltextdata) NewHandleClear (longsizeof (tytextdata));
	
	if (h == nil)
		return (false);
	
	(**h).bullets = false;
	
	(**h).hrealtext = nil;
	
	*hdata = h;
	
	return (true);
	} /*newtextdata*/
	
	
static boolean cleanedittext (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;
	short origheight = height, origwidth = width;
	
	width = IOAmakemultiple ((*r).right - (*r).left, gridunits);
	
	if (width < origwidth)
		width = origwidth;
	
	height = IOAclosestmultiple ((*r).bottom - (*r).top - 4, height);
	
	if (height < origheight)
		height = origheight;
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + height;
	
	return (true);
	} /*cleanedittext*/
	

static boolean canreplicateedittext (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	boolean flediting = (**hc).activetextobject != nil;
	
	return (!flediting); /*only replicate if we're not in edit mode*/
	} /*canreplicateedittext*/
	

static boolean getedittexteditrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
		
	return (true); /*can be edited, edit rect is the same as the object's rect*/
	} /*getedittexteditrect*/
	

static boolean getedittextvalue (hdlobject h, Handle *hvalue) {
	
	hdltextdata hdata = (hdltextdata) (**h).objectdata;
	
	if ((**hdata).bullets) {
		
		char q = '"';
		
		if (PtrToHand (&q, hvalue, 1) != noErr)
			return (false);
		
		if ((**hdata).hrealtext != nil) {
		
			if (HandAndHand ((**hdata).hrealtext, *hvalue) != noErr)
				return (false);
			}
		
		if (PtrAndHand (&q, *hvalue, 1) != noErr)
			return (false);
			
		return (true);
		}
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getedittextvalue*/
	
	
boolean gettextbehindbullets (hdlobject h, bigstring bs) {
	
	hdltextdata hdata = (hdltextdata) (**h).objectdata;
	
	IOAtexthandletostring ((**hdata).hrealtext, bs);
	
	return (true);
	} /*gettextbehindbullets*/


boolean emptytextbehindbullets (hdlobject h) {
	
	hdltextdata hdata = (hdltextdata) (**h).objectdata;

	if ((**hdata).hrealtext != nil)
		SetHandleSize ((**hdata).hrealtext, 0);
	
	return (true);
	} /*emptytextbehindbullets*/
	
	
boolean settextbehindbullets (hdlobject h, void *pstring) {

	unsigned char *value = (unsigned char *) pstring;
	hdltextdata hdata = (hdltextdata) (**h).objectdata;
	Handle htext;
	
	if (!IOAnewtexthandle (value, &htext))
		return (false);
	
	disposehandle ((**hdata).hrealtext);
	
	(**hdata).hrealtext = htext;

	return (true);
	} /*settextbehindbullets*/
	

static boolean debugedittext (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugedittext*/
	

static boolean drawedittext (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	Rect r;
	
	r = (**h).objectrect;
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	FrameRect (&r);
	
	if (!(**hc).flskiptext) 
		IOAeditdrawtexthandle ((**h).objectvalue, r, (**h).objectjustification);
	
	return (true);
	} /*drawedittext*/
	

static boolean initedittext (tyobject *obj) {
	
	newtextdata ((hdltextdata *) &(*obj).objectdata);
	
	return (true); /*we do want to edit it*/
	} /*initedittext*/
	
	
static boolean recalcedittext (hdlobject h, boolean flmajorrecalc) {
	
	hdltextdata hdata = (hdltextdata) (**h).objectdata;
	
	if (!(**hdata).bullets) 
		return (IOArecalcobjectvalue (h));
		
	disposehandle ((**hdata).hrealtext); /*recalcing bulleted text turned off -- 5/5/93 DW*/
			
	(**hdata).hrealtext = nil; 
		
	IOAsetobjectvalue (h, nil);
	
	return (true); 
	
	/*recalc bulleted text*/ {
		
		/*
		bigstring errorstring;
		Handle hrealtext, hbullets;
		long ctchars, i;
		
		if (!IOAevalscript ((**h).objectrecalcscript, &hrealtext, (**h).objectlanguage, errorstring))
			return (false);
		
		ctchars = GetHandleSize (hrealtext);
		
		hbullets = NewHandleClear (ctchars);
		
		if (hbullets == nil)
			return (false);
		
		for (i = 0; i < ctchars; i++)
			(*hbullets) [i] = '¥';
		
		disposehandle ((**hdata).hrealtext);
			
		(**hdata).hrealtext = hrealtext; 
		
		IOAsetobjectvalue (h, hbullets);
		*/
		}
	} /*recalcedittext*/
	
	
static boolean clickedittext (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	hdlcard hc = (**h).owningcard;
	
	if (h != (**hc).activetextobject) {

		IOAclearactivetextobject ();
		
		IOAsetactivetextobject (h);
		}
		
	IOAeditclick (pt, flshiftkey, IOAgetactiveeditrecord ());

	return (false); /*no recalc needed*/
	} /*clickedittext*/
	
	
static boolean setcursoredittext (hdlobject h, Point pt) {
	
	CursHandle hcursor;
	
	hcursor = GetCursor (iBeamCursor);
	
	if (hcursor != nil) 
		SetCursor (*hcursor);
	
	return (true); /*we don't want the default cursor*/
	} /*setcursoredittext*/
	
	
static boolean editedittext (hdlobject h, boolean fledit) {
	
	/*
	DW 8/26/93: allow app to set the app bit to avoid selecting
	all of the text. the comment window in Clay Basket needs
	this feature.
	*/
	
	IOAclearactivetextobject ();
	
	if (fledit) {
	
		IOAsetactivetextobject (h);
		
		if (!(**h).appbit1)
			IOAeditselectall (IOAgetactiveeditrecord ());
		}
	
	(**h).appbit1 = false;
		
	return (true);
	} /*editedittext*/
	
	
static boolean idleedittext (hdlobject h) {

	IOAeditidle (IOAgetactiveeditrecord ());
	
	return (true);
	} /*idleedittext*/
	
	
static boolean keystrokeedittext (hdlobject h, char ch) {
	
	/*
	1.0b20 dmb: allow cursor keys in numberic-only fields
	*/
	
	hdltextdata hdata = (hdltextdata) (**h).objectdata;
	Handle hrealtext = (**hdata).hrealtext;
	Handle hedit = IOAgetactiveeditrecord ();
	hdlcard hc = (**h).owningcard;
	
	if ((**hdata).numericonly) {
		
		if (((ch < '0') || (ch > '9')) && (ch != chbackspace) && (ch != '-') && (keystroketodirection (ch) == nodirection))
			return (true);
		}
	
	(**hc).runmodechanges = true; /*indicate that one of the objects in the card changed*/
	
	(**(**h).owningcard).runtimevaluechanged = true; /*DW 10/17/95*/
	
	if (!(**hdata).bullets) {
	
		IOAeditkeystroke (ch, hedit);
		
		return (true);
		}
		
	if (ch == chbackspace) {
		
		disposehandle (hrealtext);
			
		(**hdata).hrealtext = nil; 
		
		IOAeditselectall (hedit);
		
		IOAeditkeystroke (chbackspace, hedit);
		
		return (true);
		}
	
	if (hrealtext == nil) {
		
		hrealtext = NewHandle (1);
		
		(*hrealtext) [0] = ch;
		
		(**hdata).hrealtext = hrealtext;
		}
	else {
		long len = GetHandleSize (hrealtext);
		
		SetHandleSize (hrealtext, len + 1);
		
		(*hrealtext) [len] = ch;
		}
	
	IOAeditkeystroke ('¥', hedit);
	
	return (true);
	} /*keystrokeedittext*/
		
	
static boolean packtextdata (hdlobject hobject, Handle *hpackeddata) {
	
	hdltextdata hdata = (hdltextdata) (**hobject).objectdata;
	Handle hrealtext = (**hdata).hrealtext;
	short lenrealtext;
	typackedtextdata info;
	Handle hpacked;
	Ptr p;
	
	*hpackeddata = nil;
	
	IOAclearbytes (&info, longsizeof (info));
	
	info.versionnumber = 1;
	
	info.bullets = (**hdata).bullets;
	
	info.numericonly = (**hdata).numericonly;
	
	if (hrealtext == nil)
		lenrealtext = 0;
	else
		lenrealtext = GetHandleSize (hrealtext);
		
	info.lenrealtext = lenrealtext;
		
	hpacked = NewHandle (longsizeof (info) + lenrealtext);
	
	if (hpacked == nil)
		return (false);
	
	HLock (hpacked);
	
	p = *hpacked;
	
	BlockMove (&info, p, longsizeof (info));
	
	p += longsizeof (info);
	
	if (lenrealtext > 0) 
		BlockMove (*hrealtext, p, lenrealtext);
	
	HUnlock (hpacked);
	
	*hpackeddata = hpacked;
	
	return (true);
	} /*packtextdata*/
	
	
static boolean disposetextdata (hdlobject h) {
	
	hdltextdata hdata = (hdltextdata) (**h).objectdata;
	
	disposehandle ((**hdata).hrealtext);
	
	return (true);
	} /*disposetextdata*/
	
	
static boolean unpacktextdata (hdlobject h) {
	
	hdltextdata hdata;
	Handle hpackeddata;
	typackedtextdata info;
	Handle hrealtext;
	Ptr p;
	OSErr ec;
		
	hpackeddata = (**h).objectdata; 
	
	if (!newtextdata (&hdata)) 
		return (false);
		
	(**h).objectdata = (Handle) hdata;
	
	if (hpackeddata == nil) /*it's a pre 1.0b15 edittext object*/
		return (true);
	
	HLock (hpackeddata);
	
	p = *hpackeddata;
	
	BlockMove (p, &info, longsizeof (info));
	
	p += longsizeof (info);
	
	ec = PtrToHand (p, &hrealtext, info.lenrealtext);
		
	HUnlock (hpackeddata);
	
	if (ec != noErr)
		return (false);
		
	(**hdata).hrealtext = hrealtext;
	
	(**hdata).bullets = info.bullets;
	
	(**hdata).numericonly = info.numericonly;
	
	disposehandle (hpackeddata);
	
	return (true);
	} /*unpacktextdata*/
	
	
static boolean gettextattributes (hdlobject h, AppleEvent *event) {
	
	hdltextdata hdata = (hdltextdata) (**h).objectdata;
	Handle hrealtext;
	
	IACglobals.event = event; 
	
	if (!IACpushbooleanparam ((**hdata).bullets, 'pswd'))
		return (false);
	
	if (!IACpushbooleanparam ((**hdata).numericonly, 'nmrc'))
		return (false);
	
	hrealtext = (**hdata).hrealtext;
	
	if (hrealtext == nil)
		hrealtext = NewHandle (0);
	else
		HandToHand (&hrealtext);
	
	if (!IACpushtextparam (hrealtext, 'rtxt'))
		return (false);
		
	return (true);
	} /*gettextattributes*/
	
	
static boolean settextattributes (hdlobject h, AppleEvent *event) {
	
	hdltextdata hdata = (hdltextdata) (**h).objectdata;	
	Boolean fl;
	Handle hrealtext;
	
	IACglobals.event = event; 
	
	IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('pswd', &fl))
		(**hdata).bullets = fl;
	
	if (IACgetbooleanparam ('nmrc', &fl))
		(**hdata).numericonly = fl;
	
	if (IACgettextparam ('rtxt', &hrealtext)) {
		
		disposehandle ((**hdata).hrealtext);
		
		(**hdata).hrealtext = hrealtext;
		}
		
	return (true);
	} /*settextattributes*/
	

void setupedittext (tyconfigrecord *);

	
void setupedittext (tyconfigrecord *config) {

	IOAcopystring ("\pEditable text", (*config).objectTypeName);
	
	(*config).objectTypeID = edittexttype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).handlesMouseTrack = true; /*allow TextEdit to track the mouse*/
	
	(*config).editableInRunMode = true;
	
	(*config).hasSpecialCard = true;
	
	(*config).initObjectCallback = initedittext;
	
	(*config).drawObjectCallback = drawedittext;
	
	(*config).editObjectCallback = editedittext;
	
	(*config).idleObjectCallback = idleedittext;
	
	(*config).keystrokeObjectCallback = keystrokeedittext;
	
	(*config).clickObjectCallback = clickedittext;
	
	(*config).cleanupObjectCallback = cleanedittext;
	
	(*config).recalcObjectCallback = recalcedittext;
	
	(*config).canReplicateObjectCallback = canreplicateedittext;
	
	(*config).getObjectEditRectCallback = getedittexteditrect;
	
	(*config).getValueForScriptCallback = getedittextvalue;
	
	(*config).debugObjectCallback = debugedittext;
	
	(*config).setObjectCursorCallback = setcursoredittext;
	
	(*config).getAttributesCallback = gettextattributes;
	
	(*config).setAttributesCallback = settextattributes;
		
	(*config).unpackDataCallback = unpacktextdata;
	
	(*config).packDataCallback = packtextdata;
	
	(*config).disposeDataCallback = disposetextdata;
	} /*setupedittext*/
	
	
