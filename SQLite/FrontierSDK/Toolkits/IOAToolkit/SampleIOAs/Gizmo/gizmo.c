
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>
#include <appletstrings.h>


#define textvertinset 2


static boolean cleangizmo (hdlobject h, short height, short width, Rect *r) {
	
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
	} /*cleangizmo*/
	

static boolean canreplicategizmo (hdlobject h) {
	
	return (true); /*it can be replicated*/
	} /*canreplicategizmo*/
	

static boolean getgizmoeditrect (hdlobject h, Rect *r) {
	
	short height, width, extrapixels, offtop;
	
	*r = (**h).objectrect;
		
	IOAgetobjectsize (h, &height, &width);
	
	extrapixels = ((*r).bottom - (*r).top) - height;
	
	offtop = extrapixels / 2;
	
	(*r).top += offtop;
	
	(*r).bottom -= extrapixels - offtop;
	
	return (true); /*can be edited*/
	} /*getgizmoeditrect*/
	

static boolean getgizmovalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getgizmovalue*/
	

static boolean debuggizmo (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debuggizmo*/
	

static boolean drawgizmo (hdlobject h) { 
	
	/*
	DW 8/15/93: commented the pushing of the frame color before drawing
	the gizmo outline. use the text color as set by the shell. makes 
	disabled gizmos look right.
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
		EraseRect (&r);
	
	routset = r;
	
	if (flbold)
		InsetRect (&routset, -4, -4);
		
	if (!(**hc).flskiptext) {
	
		Rect redit;
		
		getgizmoeditrect (h, &redit);
		
		IOAeditdrawtexthandle (htext, redit, (**h).objectjustification);
		}
			
	/*IOApushforecolor (&(**h).objectframecolor);*/
	
	FrameRect (&r); /*must be after text drawing, 4/27/92 DW*/
	
	if (flbold) {
	
		PenState x;
		
		GetPenState (&x); /*save the old pen state*/
		
		PenSize (3, 3); /*make the pen fatter*/
		
		FrameRect (&routset); /*draw the ring*/
		
		SetPenState (&x); /*restore the pen state*/
		}
	
	/*IOApopforecolor ();*/
		
	if ((**hc).tracking && (**hc).trackerpressed) {
	
		Rect rinvert = r;
		
		InsetRect (&rinvert, 1, 1);
		
		InvertRect (&rinvert);
		}
	
	return (true);
	} /*drawgizmo*/
	

static boolean flbold;


static boolean initgizmovisit (hdlobject h) {
	
	if ((**h).objectflag) {
		
		flbold = false; /*already is a bold gizmo*/
		
		return (false); /*stop the traversal*/
		}
		
	return (true); /*keep looking*/
	} /*initgizmovisit*/
	
	
static boolean initgizmo (tyobject *obj) {
	
	hdlcard hc = (*obj).owningcard;
	
	flbold = true;
	
	IOAvisitobjects ((**hc).objectlist, &initgizmovisit);
	
	(*obj).objectflag = flbold; 
			
	(*obj).objectjustification = centerjustified;
	
	return (true); /*we do want to edit it*/
	} /*initgizmo*/
	
	
static boolean getgizmoinvalrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	if ((**h).objectflag)
		InsetRect (r, -4, -4);
	
	return (true);
	} /*getgizmoinvalrect*/
	
	
static boolean recalcgizmo (hdlobject h, boolean flmajorrecalc) {
	
	bigstring errorstring;
	Handle hvalue;
	
	if (!IOAevalscript (h, (**h).objectrecalcscript, &hvalue, (**h).objectlanguage, errorstring))
		return (false);
		
	IOAsetobjectvalue (h, hvalue); 
	
	IOAinvalobject (h);
	
	return (true); 
	} /*recalcgizmo*/


static boolean clickgizmo (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	bigstring bs;
	bigstring num;
	Handle hvalue;
	
	IOAcopystring ("\phit at (", bs);
	
	NumToString (pt.h, num);
	
	IOApushstring (num, bs);
	
	IOApushstring ("\p, ", bs);
	
	NumToString (pt.v, num);
	
	IOApushstring (num, bs);
	
	IOApushstring ("\p)", bs);
	
	IOAnewtexthandle (bs, &hvalue);
	
	IOAsetobjectvalue (h, hvalue); 
	
	DisposeHandle (hvalue);
	
	return (true); /*do a minor recalc*/
	} /*clickgizmo*/
	
	
static boolean catchreturngizmo (hdlobject h) {
	
	return ((**h).objectflag); /*if the gizmo is bold, we want the Return key*/
	} /*catchreturngizmo*/


void setupgizmo (tyconfigrecord *config);
	
void setupgizmo (tyconfigrecord *config) {
	
	IOAcopystring ("\pGizmo", (*config).objectTypeName);
	
	IOAcopystring ("\pGizmo Flag", (*config).objectFlagName);
	
	(*config).objectTypeID = 101;
	
	(*config).frameWhenEditing = false;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = true;
	
	(*config).speaksForGroup = false;
	
	(*config).hasSpecialCard = true;

	(*config).initObjectCallback = initgizmo;
	
	(*config).drawObjectCallback = drawgizmo;
	
	(*config).clickObjectCallback = clickgizmo;
	
	(*config).cleanupObjectCallback = cleangizmo;
	
	(*config).recalcObjectCallback = recalcgizmo;
	
	(*config).canReplicateObjectCallback = canreplicategizmo;
	
	(*config).catchReturnCallback = catchreturngizmo;
	
	(*config).getObjectInvalRectCallback = getgizmoinvalrect;
	
	(*config).getObjectEditRectCallback = getgizmoeditrect;
	
	(*config).getValueForScriptCallback = getgizmovalue;
	
	(*config).debugObjectCallback = debuggizmo;
	} /*setupgizmo*/


