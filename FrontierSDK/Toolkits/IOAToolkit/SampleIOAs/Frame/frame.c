
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>


#define frametype -4

#define textvertinset 2
#define texthorizinset 2
#define mintextwidth 25

#define textleftinset ((**(**h).owningcard).gridunits-texthorizinset)
#define textrightinset (3*textleftinset)



static short frametextwidth (hdlobject h) {
	
	Handle htext = (**h).objectvalue;
	short tw;

	tw = TextWidth (*htext, 0, GetHandleSize (htext)) + 2;
	
	if (tw < mintextwidth)
		tw = mintextwidth;
		
	return (tw);
	} /*frametextwidth*/
	
	
static short frametextheight (void) {
	
	FontInfo fi;
	short lh;
	
	GetFontInfo (&fi);
	
	lh = fi.ascent + fi.descent + (2 * textvertinset);
	
	if ((lh % 2) == 1) /*round up*/
		lh++;
	
	return (lh);
	} /*frametextheight*/


typedef struct tycleanframe {
	
	Rect objectrect;
	
	Rect newobjectrect;
	} tycleanframe;
	
tycleanframe cleanup;

	
static boolean cleanframevisit (hdlobject h) {
	
	Rect r = (**h).objectrect;
	Rect rsect;
	
	if (SectRect (&r, &cleanup.objectrect, &rsect))
		UnionRect (&r, &cleanup.newobjectrect, &cleanup.newobjectrect);
		
	return (true);
	} /*cleanframevisit*/
		
	
static boolean cleanframe (hdlobject h, short height, short width, Rect *robject) {
	
	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;
	short minheight, minwidth;
	Rect r;
	
	cleanup.objectrect = (**h).objectrect;
	
	cleanup.newobjectrect = (**h).objectrect;
	
	IOAvisitobjects ((**hc).objectlist, &cleanframevisit);
	
	if (cleanup.newobjectrect.left != cleanup.objectrect.left)	
		cleanup.newobjectrect.left -= textleftinset;
	
	if (cleanup.newobjectrect.right != cleanup.objectrect.right)	
		cleanup.newobjectrect.right += textleftinset;
	
	r = cleanup.newobjectrect;
	
	minheight = frametextheight ();
	
	if ((r.bottom - r.top) < minheight)
		r.bottom = r.top + minheight;
	
	minwidth = width + textleftinset + textrightinset;
	
	if ((r.right - r.left) < minwidth)
		r.right = r.left + minwidth;
		
	width = IOAmakemultiple (r.right - r.left, gridunits);
	
	height = IOAmakemultiple (r.bottom - r.top, gridunits);
	
	r.right = r.left + width;

	r.bottom = r.top + height;

	*robject = r;
	
	return (true);
	} /*cleanframe*/
	

static boolean canreplicateframe (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	boolean flediting = (**hc).activetextobject != nil;
	
	return (!flediting); /*only replicate if we're not in edit mode*/
	} /*canreplicateframe*/
	

static boolean getframeeditrect (hdlobject h, Rect *rframe) {
	
	Rect r = (**h).objectrect;
	short fth = frametextheight ();
	
	r.top -= fth / 2;
	
	r.bottom = r.top + fth;
	
	r.left += textleftinset;
			
	r.right -= textrightinset;
	
	*rframe = r;
			
	return (true); /*can be edited, edit rect is the same as the object's rect*/
	} /*getframeeditrect*/
	

static boolean getframevalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getframevalue*/
	

static boolean debugframe (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugframe*/
	

static boolean drawframe (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	boolean notediting = !(**hc).flskiptext;
	Handle htext = (**h).objectvalue;
	Rect r, rtext;
	PenState ps;
	
	r = (**h).objectrect;
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	getframeeditrect (h, &rtext);
	
	if (notediting) 
		rtext.right = rtext.left + frametextwidth (h) + (2 * texthorizinset);
	
	IOApushforecolor (&(**h).objectframecolor);
	
	GetPenState (&ps);
	
	if ((**h).objectselected)
		PenPat (&quickdrawglobal (black));
	else
		PenPat (&quickdrawglobal (gray));
		
	FrameRect (&r);
	
	/*
	rframe = r;
	
	rframe.right--; rframe.bottom--;
	
	MoveTo (rframe.left, rframe.top);
	
	LineTo (rframe.left + textleftinset, rframe.top);
	
	MoveTo (rtext.right, rframe.top);
	
	LineTo (rframe.right, rframe.top);
	
	LineTo (rframe.right, rframe.bottom);
	
	LineTo (rframe.left, rframe.bottom);
	
	LineTo (rframe.left, rframe.top);
	*/
	
	SetPenState (&ps);
	
	IOApopforecolor ();
	
	if (notediting) 
		IOAeditdrawtexthandle (htext, rtext, (**h).objectjustification);
		
	(**h).objecttmpbit = true; /*tell the framework not to draw the object's frame*/
	
	return (true);
	} /*drawframe*/
	

static boolean initframe (tyobject *obj) {
	
	(*obj).objecttransparent = true;
	
	return (true); /*we do want to edit it*/
	} /*initframe*/
	
	
static boolean recalcframe (hdlobject h, boolean flmajorrecalc) {
	
	return (IOArecalcobjectvalue (h));
	} /*recalcframe*/
	
	
static boolean getframeinvalrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	(*r).top -= frametextheight () / 2;
	
	return (true);
	} /*getframeinvalrect*/
	
	
void setupframe (tyconfigrecord *);


void setupframe (tyconfigrecord *config) {

	IOAcopystring ("\pFrame", (*config).objectTypeName);
	
	(*config).objectTypeID = frametype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).initObjectCallback = initframe;
	
	(*config).drawObjectCallback = drawframe;
	
	(*config).cleanupObjectCallback = cleanframe;
	
	(*config).recalcObjectCallback = recalcframe;
	
	(*config).canReplicateObjectCallback = canreplicateframe;
	
	(*config).getObjectEditRectCallback = getframeeditrect;
	
	(*config).getValueForScriptCallback = getframevalue;
	
	(*config).debugObjectCallback = debugframe;
	
	(*config).getObjectInvalRectCallback = getframeinvalrect;
	} /*setupframe*/
	
	
