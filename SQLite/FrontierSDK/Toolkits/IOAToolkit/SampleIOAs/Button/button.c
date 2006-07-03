
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>


#define textvertinset 2


static boolean cleanbutton (hdlobject h, short height, short width, Rect *r) {
	
	/*
	dmb 1.0b20 -- don't alter font-based measurements for standard system 
	font buttons. i.e. use the height that is passed to us.
	*/
	
	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;

	width = IOAmakemultiple (width + 6, gridunits);
	
	width = max (width, 60); /*apply minimum button width*/
	
	(*r).right = (*r).left + width;
	
	if (((**h).objectfont == systemFont) && ((**h).objectfontsize == 12))
		(*r).bottom = (*r).top + height;
	else
		(*r).bottom = (*r).top + IOAmakemultiple (height - (2 * textvertinset) + 6, gridunits);
	
	return (true);
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
	

static boolean drawbutton (hdlobject h) { 
	
	/*
	DW 8/15/93: commented the pushing of the frame color before drawing
	the button outline. use the text color as set by the shell. makes 
	disabled buttons look right.
	*/
	
	hdlcard hc = (**h).owningcard;
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
			
	/*IOApushforecolor (&(**h).objectframecolor);*/
	
	FrameRoundRect (&r, 9, 9); /*must be after text drawing, 4/27/92 DW*/
	
	if (flbold) {
	
		PenState x;
		
		GetPenState (&x); /*save the old pen state*/
		
		PenSize (3, 3); /*make the pen fatter*/
		
		FrameRoundRect (&routset, 16, 16); /*draw the ring*/
		
		SetPenState (&x); /*restore the pen state*/
		}
		
	/*IOApopforecolor ();*/
		
	if ((**hc).tracking && (**hc).trackerpressed) {
	
		Rect rinvert = r;
		
		InsetRect (&rinvert, 1, 1);
		
		InvertRoundRect (&rinvert, 9, 9);
		}
	
	return (true);
	} /*drawbutton*/
	

static boolean flbold;


static boolean initbuttonvisit (hdlobject h) {
	
	if ((**h).objectflag) {
		
		flbold = false; /*already is a bold button*/
		
		return (false); /*stop the traversal*/
		}
		
	return (true); /*keep looking*/
	} /*initbuttonvisit*/
	
	
static boolean initbutton (tyobject *obj) {
	
	hdlcard hc = (*obj).owningcard;
	
	flbold = true;
	
	IOAvisitobjects ((**hc).objectlist, &initbuttonvisit);
	
	(*obj).objectflag = flbold; 
			
	(*obj).objectjustification = centerjustified;
	
	return (true); /*we do want to edit it*/
	} /*initbutton*/
	
	
static boolean getbuttoninvalrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	if ((**h).objectflag)
		InsetRect (r, -4, -4);
	
	return (true);
	} /*getbuttoninvalrect*/
	
	
static boolean recalcbutton (hdlobject h, boolean flmajorrecalc) {
	
	bigstring errorstring;
	Handle hvalue;
	
	if (!IOAevalscript (h, (**h).objectrecalcscript, &hvalue, (**h).objectlanguage, errorstring))
		return (false);
		
	IOAsetobjectvalue (h, hvalue); 
	
	IOAinvalobject (h);
	
	return (true); 
	} /*recalcbutton*/


static boolean clickbutton (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	IOArunbuttonscript (h);
	
	return (true); /*do a minor recalc*/
	} /*clickbutton*/
	
	
static boolean catchreturnbutton (hdlobject h) {
	
	return ((**h).objectflag); /*if the button is bold, we want the Return key*/
	} /*catchreturnbutton*/
	

void setupbutton (tyconfigrecord *);


void setupbutton (tyconfigrecord *config) {
	
	IOAcopystring ("\pButton", (*config).objectTypeName);
	
	IOAcopystring ("\pDefault Button", (*config).objectFlagName);
	
	(*config).objectTypeID = buttontype;
	
	(*config).frameWhenEditing = false;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).hasSpecialCard = true;

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
	
	
