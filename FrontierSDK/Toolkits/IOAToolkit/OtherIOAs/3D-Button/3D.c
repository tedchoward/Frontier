
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>

#define threeDtype -2

#define shadowdepth 3

#define textvertinset 2


static boolean cleanbutton (hdlobject h, short height, short width, Rect *r) {

	register hdlcard hc = (**h).owningcard;
	register short gridunits = (**hc).gridunits;

	width = IOAmakemultiple (width + 6, gridunits);
	
	width = max (width, 60); /*apply minimum button width*/
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + IOAmakemultiple (height - (2 * textvertinset) + 6, gridunits);
	} /*cleanbutton*/
	

static boolean canreplicatebutton (hdlobject h) {
	
	return (true); /*it can be replicated*/
	} /*canreplicatebutton*/
	

static boolean getbuttoneditrect (hdlobject h, Rect *r) {
	
	short height, width, extrapixels, offtop;
	
	*r = (**h).objectrect;
		
	IOAgetobjectsize (h, &height, &width);
	
	extrapixels = ((*r).bottom - (*r).top) - height;
	
	offtop = extrapixels / 2;
	
	(*r).top += offtop;
	
	(*r).bottom -= extrapixels - offtop;
	
	return (true); /*can be edited*/
	} /*getbuttoneditrect*/
	

static boolean getbuttonvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getbuttonvalue*/
	

static boolean debugbutton (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugbutton*/
	

static boolean olddrawbutton (hdlobject h) { 
	
	register hdlcard hc = (**h).owningcard;
	Rect routset;
	Rect r;
	boolean flbold;
	Handle htext;
	
	htext = (**h).objectvalue;
	
	r = (**h).objectrect;
	
	flbold = (**h).objectflag;
	
	if (!(**h).objecttransparent)
		EraseRoundRect (&r, 9, 9);
	
	routset = r;
	
	if (flbold)
		InsetRect (&routset, -4, -4);
		
	if (!(**hc).flskiptext) {
	
		Rect redit;
		
		getbuttoneditrect (h, &redit);
		
		IOAeditdrawtexthandle (htext, redit, (**h).objectjustification);
		}
			
	IOApushforecolor (&(**h).objectframecolor);
		
	FrameRoundRect (&r, 9, 9); /*must be after text drawing, 4/27/92 DW*/
	
	if (flbold) {
	
		PenState x;
		
		GetPenState (&x); /*save the old pen state*/
		
		PenSize (3, 3); /*make the pen fatter*/
		
		FrameRoundRect (&routset, 16, 16); /*draw the ring*/
		
		SetPenState (&x); /*restore the pen state*/
		}
		
	IOApopforecolor ();
		
	if ((**hc).tracking && (**hc).trackerpressed) {
	
		Rect rinvert = r;
		
		InsetRect (&rinvert, 1, 1);
		
		InvertRoundRect (&rinvert, 9, 9);
		}
	} /*olddrawbutton*/
	

static void dropshadowrect (Rect rorig, short width, boolean flerase) {
	
	Rect r = rorig;
	register short h, v;
	register short i;
	PenState ps;
	
	if (flerase) {
		
		GetPenState (&ps);
		
		PenMode (patBic);
		}
		
	for (i = 0; i < width; i++) {
		
		h = r.right + i;
		
		MoveTo (h, r.top + i);
		
		LineTo (h, r.bottom + width - 1);
		
		v = r.bottom + i;
		
		MoveTo (r.left + i, v);
		
		LineTo (r.right + width - 1, v);
		} /*for*/
		
	if (flerase)
		SetPenState (&ps);
	} /*dropshadowrect*/ 
	
	
static boolean drawbutton (hdlobject h) { 
	
	register hdlcard hc = (**h).owningcard;
	boolean flpressed = (**hc).tracking && (**hc).trackerpressed;
	Rect r = (**h).objectrect;
	Rect rframe;
	
	if (!(**h).objecttransparent) 
		EraseRect (&r);
	
	if (flpressed) {
		
		Rect rerase = r;
		
		IOApushbackcolor (&(**hc).backcolor);
		
		rerase.bottom = r.top + shadowdepth;
		
		rerase.right += shadowdepth;
		
		EraseRect (&rerase);
		
		rerase = r;
		
		rerase.right = rerase.left + shadowdepth;
		
		rerase.bottom += shadowdepth;
		
		EraseRect (&rerase);
		
		IOApopbackcolor ();
		}
	
	if (!(**hc).flskiptext) {
	
		Rect redit;
		
		getbuttoneditrect (h, &redit);
		
		if (flpressed) 
			OffsetRect (&redit, shadowdepth, shadowdepth);
		
		IOAeditdrawtexthandle ((**h).objectvalue, redit, (**h).objectjustification);
		}
			
	IOApushforecolor (&(**h).objectframecolor);
	
	rframe = r;
	
	dropshadowrect (rframe, shadowdepth, flpressed);
	
	if (flpressed) 
		OffsetRect (&rframe, shadowdepth, shadowdepth);
		
	FrameRect (&rframe);
	
	IOApopforecolor ();	
	} /*drawbutton*/
	

static boolean initbutton (tyobject *obj) {
	
	(*obj).objectjustification = centerjustified;
	
	return (true); /*we do want to edit it*/
	} /*initbutton*/
	
	
static boolean getbuttoninvalrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	(*r).right += shadowdepth;
	
	(*r).bottom += shadowdepth;
	} /*getbuttoninvalrect*/
	
	
static boolean recalcbutton (hdlobject h, boolean flmajorrecalc) {
	
	return (true); /*the script linked into a button is used for other purposes*/
	} /*recalcbutton*/


static boolean clickbutton (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey) {
	
	IOArunbuttonscript (h);
	
	return (true); /*do a minor recalc*/
	} /*clickbutton*/
	
	
static boolean catchreturnbutton (hdlobject h) {
	
	return ((**h).objectflag); /*if the button is bold, we want the Return key*/
	} /*catchreturnbutton*/
	
	
void setupconfig (tyconfigrecord *config) {
	
	IOAcopystring ("\p3D Button", (*config).objectTypeName);
	
	(*config).objectTypeID = threeDtype;
	
	(*config).frameWhenEditing = false;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).initObjectCallback = initbutton;
	
	(*config).drawObjectCallback = drawbutton;
	
	(*config).clickObjectCallback = clickbutton;
	
	(*config).cleanupObjectCallback = cleanbutton;
	
	(*config).recalcObjectCallback = recalcbutton;
	
	(*config).canReplicateObjectCallback = canreplicatebutton;
	
	(*config).catchReturnCallback = catchreturnbutton;
	
	(*config).getObjectInvalRectCallback = getbuttoninvalrect;
	
	(*config).getObjectEditRectCallback = getbuttoneditrect;
	
	(*config).getValueForScriptCallback = getbuttonvalue;
	
	(*config).debugObjectCallback = debugbutton;
	} /*setupconfig*/
	
	
