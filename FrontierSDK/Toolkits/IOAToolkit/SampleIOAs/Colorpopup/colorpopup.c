
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


/*original code by Dave Shaver, Quark, 303-849-3311, adapted by DW 2/9/93. Thanks Dave!*/

//#include <SetUpA4.h>
#include <GestaltEqu.h>
#include <iac.h>
#include <ioa.h>
#include "ioacolorpopup.h"

#define colorpopuptype -3

#define mPopupMsg 3 /*this MDEF message isn't in the headers*/

#define outsetsize 1

#define itemsize 10

#define boxinset 5 /*the number of pixels to skip between box and text*/

#define textvertinset 2

#define gestalttrap 0xA1AD

#define unimplementedtrap 0xA09F

static RGBColor greencolor = {0, 32767, 0};

static RGBColor whitecolor = {65535, 65535, 65535};

static RGBColor blackcolor = {0, 0, 0};

static RGBColor lightbluecolor = {52428, 52428, 65535};


static boolean gestaltavailable (void) {

	UniversalProcPtr gestaltaddr;
	UniversalProcPtr unimplementedaddr;

	gestaltaddr = NGetTrapAddress (gestalttrap, ToolTrap);
	
	unimplementedaddr = NGetTrapAddress (unimplementedtrap, ToolTrap);
	
	return (unimplementedaddr != gestaltaddr);	
	} /*gestaltavailable*/
	
	
static boolean getgestaltattr (OSType selector, long *response) {
	
	OSErr errcode;
	
	if (!gestaltavailable ())
		return (false);
		
	errcode = Gestalt (selector, response);
	
	return (errcode == noErr);
	} /*getgestaltattr*/
	
	
static boolean systemhascolor (void) {

	long templong;
	
	if (!getgestaltattr (gestaltQuickdrawVersion, &templong))
		return (false);
	
	return (templong >= gestalt8BitQD);
	} /*systemhascolor*/


static void getitemrect (short item, Rect *ritem) {

	short x, y;
	Rect r;
	
	item--;
	
	x = item % 32;
	
	y = item / 32;
	
	r.top = 1 + y * itemsize;
	
	r.left = 1 + x * itemsize;
	
	r.bottom = r.top + itemsize;
	
	r.right = r.left + itemsize;
	
	*ritem = r;
	} /*getitemrect*/
	
	
static void frameitem (short item, Rect menurect) {

	Rect r;
	PenState ps;
	
	if (item <= 0)
		return;
		
	IOApushforecolor (&blackcolor);
	
	GetPenState (&ps);
	
	PenMode (patXor);
	
	getitemrect (item, &r);
	
	OffsetRect (&r, menurect.left, menurect.top);
	
	FrameRect (&r);
	
	SetPenState (&ps);
	
	IOApopforecolor ();
	} /*frameitem*/
	
	
static pascal void colormenuhandler (short msg, MenuHandle hmenu, Rect *menurect, Point pt, short *item) {

	#ifndef IOAinsideApp

		SetUpA4 ();
	
	#endif

	switch (msg) {
	
		case mDrawMsg: {
			short i, j;
			Rect r;	
			CSpecArray *ctable;
		
			ctable = &(**GetCTable (8)).ctTable;
			
			IOApushforecolor (&lightbluecolor);
			
			PaintRect ((const Rect *) &menurect);
			
			for (i = 0; i < 8; i++) {
			
				for (j = 0; j < 32; j++) {
				
					getitemrect (i * 32 + j + 1, &r);
					
					OffsetRect (&r, (*menurect).left, (*menurect).top);
					
					RGBForeColor (&blackcolor);
					
					InsetRect (&r, 1, 1);
					
					PaintRect (&r);
					
					InsetRect (&r, 1, 1);
					
					RGBForeColor (&(*ctable) [i * 32 + j].rgb);
					
					PaintRect (&r);
					} /*for*/
				} /*for*/
			
			IOApopforecolor ();
			
			break;
			}
			
		case mChooseMsg: {
			short nextitem;
			Rect rinset;
			
			rinset = *menurect;
			
			InsetRect (&rinset, outsetsize, outsetsize);
			
			if (!PtInRect (pt, &rinset)) 
				nextitem = 0;
			else {
				nextitem = 
					32L * ((pt.v - (*menurect).top - outsetsize) / itemsize) +
					(pt.h - (*menurect).left - outsetsize) / itemsize + 1;
				
				if (nextitem < 0) 
					nextitem = 0;
				}
				
			if (*item == nextitem) /*already highlighted*/
				break;
				
			frameitem (*item, *menurect); /*unhighlight the original item*/
		
			frameitem (nextitem, *menurect); /*highlight the newly selected item*/
				
			*item = nextitem;
			
			break;
			}
			
		case mSizeMsg:
			(**hmenu).menuHeight = 8L * itemsize + 2;
			
			(**hmenu).menuWidth = 32L * itemsize + 2;
			
			break;
			
		case mPopupMsg:
			(*menurect).top = pt.h;
			
			(*menurect).left = pt.v;
			
			(*menurect).bottom = (*menurect).top + (**hmenu).menuHeight;
			
			(*menurect).right = (*menurect).left + (**hmenu).menuWidth;
			
			break;
		} /*switch*/

	#ifndef IOAinsideApp

		RestoreA4 ();
	
	#endif
	} /*colormenuhandler*/
	
	
static void DebugNum (long num) {
	
	bigstring bs;
	
	NumToString (num, bs);
	
	DebugStr (bs);
	} /*DebugNum*/

#if !GENERATINGCFM

typedef struct Jump68kDescriptor {
	
	short jmp;
	
	ProcPtr adr;
	} Jump68kDescriptor;

#define BUILD_JUMP68K_DESCRIPTOR(procedure)  \
	{								\
	0x4EF9,							\
	(ProcPtr)(procedure),			\
	}

#endif

static boolean colormenuclick (Point pt, RGBColor *rgb) {
	
	/*
	4.1b3 dmb: updated for ppc compatibility
	*/
	
	long result;
	MenuHandle colormenu;
	#if GENERATINGCFM
		RoutineDescriptor mdefstub = BUILD_ROUTINE_DESCRIPTOR (uppMenuDefProcInfo, colormenuhandler);
	#else
		Jump68kDescriptor mdefstub = BUILD_JUMP68K_DESCRIPTOR (colormenuhandler);
	#endif
	Handle hmdefstub;
	short lo, hi;
	
	#ifndef IOAinsideApp

		RememberA4 ();
	
	#endif

	if (!systemhascolor ()) 
		return (false);
		
	IOAopenresfile ();
	
	colormenu = GetMenu (256);

	hmdefstub = NewHandle (sizeof (mdefstub));
	
	#if GENERATINGCFM
		**(RoutineDescriptor **) hmdefstub = mdefstub;
	#else
		**(Jump68kDescriptor **) hmdefstub = mdefstub;
	#endif
	
	(**colormenu).menuProc = hmdefstub;
	
	CalcMenuSize (colormenu);
		
	InsertMenu (colormenu, hierMenu);

	LocalToGlobal (&pt);
	
	result = PopUpMenuSelect (colormenu, pt.v, pt.h, 1);
	
	DisposeHandle (hmdefstub);
	
	DeleteMenu ((**colormenu).menuID);
	
	IOAcloseresfile ();
	
	lo = LoWord (result);
	
	hi = HiWord (result);
	
	if (hi > 0) { /*something was selected*/
	
		CSpecArray *ctable;
		
		ctable = &(**GetCTable (8)).ctTable;

		*rgb = (*ctable) [lo - 1].rgb;
		
		return (true);
		}
	
	return (false);
	} /*colormenuclick*/
	

static void getcolorrect (hdlobject h, Rect r, Rect *rbox) {
	
	hdlcard hc = (**h).owningcard;
	short objectfontsize = (**h).objectfontsize;
	FontInfo fi = (**hc).fontinfo;
	short lineheight;
	short extrapixels;
	
	lineheight = fi.ascent + fi.descent + fi.leading;
	
	extrapixels = (lineheight - objectfontsize) / 2;
	
	r.top += textvertinset;
	
	if (extrapixels > 0)
		r.top += (lineheight - objectfontsize) - extrapixels;
	
	r.bottom = r.top + objectfontsize;
	
	r.left += 3;
	
	r.right = r.left + objectfontsize;
	
	*rbox = r;
	} /*getcolorrect*/
	

static boolean getcoloreditrect (hdlobject h, Rect *r) {

	*r = (**h).objectrect;
		
	(*r).left += 3 + (**h).objectfontsize + boxinset;
	
	return (true); /*it can be edited*/
	} /*getcoloreditrect*/
	

static boolean clickcolorpopup (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	hdlcolordata hdata = (hdlcolordata) (**h).objectdata;
	RGBColor rgb;
	
	if (!systemhascolor ()) 
		return (false);
		
	if (!colormenuclick (pt, &rgb))
		return (false);
		
	(**hdata).rgb = rgb;
		
	(**(**h).owningcard).runtimevaluechanged = true; /*DW 10/13/95*/
	
	IOAinvalobject (h);
	
	IOArunbuttonscript (h); /*1.0b15 -- run the action script*/
	
	return (true); /*do a minor recalc*/
	} /*clickcolorpopup*/
	
	
static boolean cleancolorpopup (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;
	
	width = IOAmakemultiple (width + 3 + (**h).objectfontsize + boxinset, gridunits);
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + height;
	
	return (true);
	} /*cleancolorpopup*/
	

static boolean canreplicatecolorpopup (hdlobject h) {
	
	return (true); 
	} /*canreplicatecolorpopup*/
	

static boolean getcheckboxeditrect (hdlobject h, Rect *r) {

	*r = (**h).objectrect;
		
	(*r).left += 3 + (**h).objectfontsize + boxinset;
	
	return (true); /*it can be edited*/
	} /*getcheckboxeditrect*/
	

static void pushlong (long num, bigstring bs) {
	
	bigstring bsnum;
	
	NumToString (num, bsnum);
	
	IOApushstring (bsnum, bs);
	} /*pushlong*/
	
	
static boolean getcolorpopupvalue (hdlobject h, Handle *hvalue) {
	
	hdlcolordata hdata = (hdlcolordata) (**h).objectdata;
	RGBColor rgb = (**hdata).rgb;
	bigstring bs;
	
	IOAcopystring ("\p\"", bs);
	
	pushlong (rgb.red, bs);
	
	IOApushstring ("\p,", bs);
	
	pushlong (rgb.green, bs);
	
	IOApushstring ("\p,", bs);

	pushlong (rgb.blue, bs);
	
	IOApushstring ("\p\"", bs);
	
	return (IOAnewtexthandle (bs, hvalue));
	} /*getcolorpopupvalue*/
	
	
static void poplong (bigstring bs, unsigned short *x) {
	
	bigstring bsnum;
	long longval;
	
	bsnum [0] = 0;
	
	while (true) {
		
		short len;
		char ch;
		
		len = bs [0];
		
		if (len == 0)
			break;
			
		ch = bs [1];
		
		BlockMove (&bs [2], &bs [1], --len);
		
		bs [0] = len;
		
		if ((ch < '0') || (ch > '9')) 
			break;
			
		len = bsnum [0] + 1;
		
		bsnum [0] = len;
		
		bsnum [len] = ch;
		} /*while*/
	
	StringToNum (bsnum, &longval);
	
	*x = longval;
	} /*poplong*/
	

static boolean setcolorpopupvalue (hdlobject h, Handle hvalue) {

	hdlcolordata hdata = (hdlcolordata) (**h).objectdata;
	bigstring bs;
	RGBColor rgb;
	
	if (hvalue == nil)
		return (true);
		
	IOAtexthandletostring (hvalue, bs);
	
	DisposHandle (hvalue);
	
	poplong (bs, &rgb.red);
	
	poplong (bs, &rgb.green);
	
	poplong (bs, &rgb.blue);
	
	(**hdata).rgb = rgb;
	
	return (true);
	} /*setcolorpopupvalue*/


static boolean debugcolorpopup (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugcolorpopup*/
	
	
static boolean drawcolorobject (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	hdlcolordata hdata = (hdlcolordata) (**h).objectdata;
	RGBColor rgb = (**hdata).rgb;
	Rect rbox;
	Rect r;
	Handle htext;
	
	htext = (**h).objectvalue; 
	
	r = (**h).objectrect;
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
		
	getcolorrect (h, r, &rbox);
	
	if (!(**h).objectenabled)
		rgb.red = rgb.green = rgb.blue = 61166; /*light gray*/
		
	IOApushbackcolor (&rgb);

	EraseRect (&rbox);
	
	IOApopbackcolor ();
		
	FrameRect (&rbox);
	
	if ((**hc).tracking && (**hc).trackerpressed) {
		
		Rect rinset = rbox;
		
		InsetRect (&rinset, 1, 1);
		
		FrameRect (&rinset);
		}
	
	if (!(**hc).flskiptext) {
		
		Rect rtext;
		
		getcoloreditrect (h, &rtext);
		
		IOAeditdrawtexthandle (htext, rtext, (**h).objectjustification);
		}
	
	return (true);
	} /*drawcolorobject*/
	
	
static boolean newcolordata (hdlcolordata *hdata) {

	hdlcolordata h;
	
	h = (hdlcolordata) NewHandleClear (longsizeof (tycolordata));
	
	if (h == nil)
		return (false);
	
	(**h).versionnumber = 1;
	
	(**h).rgb = lightbluecolor;
	
	*hdata = h;
	
	return (true);
	} /*newcolordata*/
	
	
static boolean initcolorpopup (tyobject *obj) {
	
	newcolordata ((hdlcolordata *) &(*obj).objectdata);
	
	return (true); /*nothing special, we do want to edit it*/
	} /*initcolorpopup*/
	
	
static boolean unpackcolordata (hdlobject h) {
	
	hdlcolordata hdata = (hdlcolordata) (**h).objectdata;
	
	if (hdata == nil) { /*pre 1.0b15 object, no data handle*/
		
		if (!newcolordata (&hdata))
			return (false);
			
		(**h).objectdata = (Handle) hdata;
		
		return (true);
		}
		
	return (true);
	} /*unpackcolordata*/


static boolean getcolorattributes (hdlobject h, AppleEvent *event) {
	
	hdlcolordata hdata = (hdlcolordata) (**h).objectdata;
	RGBColor rgb = (**hdata).rgb;
	
	IACglobals.event = event; 
	
	if (!IACpushRGBColorparam (&rgb, 'colr'))
		return (false);
	
	return (true);
	} /*getcolorattributes*/
	
	
static boolean setcolorattributes (hdlobject h, AppleEvent *event) {
	
	hdlcolordata hdata = (hdlcolordata) (**h).objectdata;	
	RGBColor rgb;
	
	IACglobals.event = event; 
	
	IACglobals.nextparamoptional = true;
	
	if (IACgetRGBColorparam ('colr', &rgb))
		(**hdata).rgb = rgb;
	
	return (true);
	} /*setcolorattributes*/
	
	
static boolean recalccolorpopup (hdlobject h, boolean flmajorrecalc) {
	
	bigstring errorstring;
	Handle hrgbvalue;
	
	if (!IOAevalscript (h, (**h).objectrecalcscript, &hrgbvalue, (**h).objectlanguage, errorstring))
		return (false);
		
	setcolorpopupvalue (h, hrgbvalue); /*also disposes of hrgbvalue*/
	
	return (true); 
	} /*recalccolorpopup*/
	

void setupcolorpopup (tyconfigrecord *);


void setupcolorpopup (tyconfigrecord *config) {
	
	IOAcopystring ("\pColor popup", (*config).objectTypeName);
	
	(*config).objectTypeID = colorpopuptype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).handlesMouseTrack = true; /*allow the popup manager to track the mouse*/

	(*config).hasSpecialCard = false; /*dmb 1.0b22 - was true, but there's no card*/

	(*config).initObjectCallback = initcolorpopup;
	
	(*config).drawObjectCallback = drawcolorobject;
	
	(*config).clickObjectCallback = clickcolorpopup;
	
	(*config).cleanupObjectCallback = cleancolorpopup;
	
	(*config).canReplicateObjectCallback = canreplicatecolorpopup;
	
	(*config).getObjectEditRectCallback = getcoloreditrect;
	
	(*config).getValueForScriptCallback = getcolorpopupvalue;
	
	(*config).debugObjectCallback = debugcolorpopup;	
	
	(*config).getAttributesCallback = getcolorattributes;
	
	(*config).setAttributesCallback = setcolorattributes;
	
	(*config).unpackDataCallback = unpackcolordata;
	
	(*config).recalcObjectCallback = recalccolorpopup;
	} /*setupconfig*/
	
	
