
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>



static boolean cleanstatic (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;
	Handle hvalue;
	boolean flmultiline = false;
	long i, ct;
	
	hvalue = (**h).objectvalue;
	
	ct = GetHandleSize (hvalue);
	
	for (i = 0; i < ct; i++) {
	
		if ((*hvalue) [i] == chreturn) { /*it has a hard return in it*/
		
			flmultiline = true;
			
			break;
			}
		} /*for*/

	if (flmultiline) {
	
		short origheight = height;
		
		width = IOAmakemultiple ((*r).right - (*r).left, gridunits);
		
		height = IOAclosestmultiple ((*r).bottom - (*r).top - 4, height);
		
		if (height < origheight)
			height = origheight;
		
		(*r).right = (*r).left + width;
		
		(*r).bottom = (*r).top + height;
		}
	else {
		width = IOAmakemultiple (width, gridunits);
		
		(*r).right = (*r).left + width;
		
		(*r).bottom = (*r).top + height;
		}
	
	return (true);
	} /*cleanstatic*/
	

static boolean canreplicatestatic (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	boolean flediting = (**hc).activetextobject != nil;
	
	return (!flediting); /*only replicate if we're not in edit mode*/
	} /*canreplicatestatic*/
	

static boolean getstaticeditrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
		
	return (true); /*can be edited, edit rect is the same as the object's rect*/
	} /*getstaticeditrect*/
	

static boolean getstaticvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getstaticvalue*/
	

static boolean debugstatic (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugstatic*/
	

static boolean drawstatic (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	Handle htext;
	Rect r;
	
	htext = (**h).objectvalue;
	
	r = (**h).objectrect;
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	if (!(**hc).flskiptext)
		IOAeditdrawtexthandle (htext, r, (**h).objectjustification);
	
	return (true);
	} /*drawstatic*/
	

static boolean initstatic (tyobject *obj) {
	
	return (true); /*we do want to edit it*/
	} /*initstatic*/
	
	
static boolean recalcstatic (hdlobject h, boolean flmajorrecalc) {
	
	return (IOArecalcobjectvalue (h));
	} /*recalcstatic*/
	
	
void setupstatic (tyconfigrecord *);


void setupstatic (tyconfigrecord *config) {

	IOAcopystring ("\pText", (*config).objectTypeName);
	
	(*config).objectTypeID = statictexttype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).initObjectCallback = initstatic;
	
	(*config).drawObjectCallback = drawstatic;
	
	(*config).cleanupObjectCallback = cleanstatic;
	
	(*config).recalcObjectCallback = recalcstatic;
	
	(*config).canReplicateObjectCallback = canreplicatestatic;
	
	(*config).getObjectEditRectCallback = getstaticeditrect;
	
	(*config).getValueForScriptCallback = getstaticvalue;
	
	(*config).debugObjectCallback = debugstatic;
	} /*setupstatic*/
	
	
