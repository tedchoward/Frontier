
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>



static boolean cleanoval (hdlobject h, short height, short width, Rect *r) {

	register hdlcard hc = (**h).owningcard;
	register short gridunits = (**hc).gridunits;
	
	width = IOAmakemultiple ((*r).right - (*r).left, gridunits);
	
	height = IOAmakemultiple ((*r).bottom - (*r).top, gridunits);
	
	(*r).right = (*r).left + width;

	(*r).bottom = (*r).top + height;
	} /*cleanoval*/
	

static boolean canreplicateoval (hdlobject h) {
	
	return (false); /*it can't be replicated*/
	} /*canreplicateoval*/
	

static boolean getovaleditrect (hdlobject h, Rect *r) {
	
	return (false); /*can't be edited*/
	} /*getovaleditrect*/
	

static boolean getovalvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getovalvalue*/
	

static boolean debugoval (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugoval*/
	

static boolean drawoval (hdlobject h) {
	
	Rect r;
	
	r = (**h).objectrect;
	
	if (!(**h).objecttransparent)
		EraseOval (&r);
	
	FrameOval (&r);
	} /*drawoval*/
	

static boolean initoval (tyobject *obj) {
	
	return (false); /*we don't want to edit it*/
	} /*initoval*/
	
	
static boolean clickoval (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	IOArunbuttonscript (h);
	
	return (true); /*do a minor recalc*/
	} /*clickoval*/
	
	
void setupconfig (tyconfigrecord *config) {

	IOAcopystring ("\pOval", (*config).objectTypeName);
	
	(*config).objectTypeID = ovaltype;
	
	(*config).frameWhenEditing = false; /*can't be edited*/
	
	(*config).canEditValue = false;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).isFontAware = false;
	
	(*config).initObjectCallback = initoval;
	
	(*config).drawObjectCallback = drawoval;
	
	(*config).clickObjectCallback = clickoval;
	
	(*config).cleanupObjectCallback = cleanoval;
	
	(*config).canReplicateObjectCallback = canreplicateoval;
	
	(*config).getObjectEditRectCallback = getovaleditrect;
	
	(*config).getValueForScriptCallback = getovalvalue;
	
	(*config).debugObjectCallback = debugoval;
	} /*setupconfig*/
	
	
