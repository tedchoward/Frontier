
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>



static boolean cleanformula (hdlobject h, short height, short width, Rect *r) {

	register hdlcard hc = (**h).owningcard;
	register short gridunits = (**hc).gridunits;
	
	width = IOAmakemultiple (width, gridunits);
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + height;
	} /*cleanformula*/
	

static boolean canreplicateformula (hdlobject h) {
	
	return (true); 
	} /*canreplicateformula*/
	

static boolean getformulaeditrect (hdlobject h, Rect *r) {
	
	return (false); /*can't be edited*/
	} /*getformulaeditrect*/
	

static boolean getformulavalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getformulavalue*/
	

static boolean debugformula (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugformula*/
	

static boolean drawformula (hdlobject h) {
	
	register hdlcard hc = (**h).owningcard;
	Rect r;
	
	r = (**h).objectrect;
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	if (!(**hc).flskiptext)
		IOAeditdrawtexthandle ((**h).objectvalue, r, (**h).objectjustification);
	} /*drawformula*/
	

static boolean initformula (tyobject *obj) {
	
	(*obj).objecthasframe = true;
	
	return (false); /*we don't want to edit it*/
	} /*initformula*/
	
	
static boolean recalcformulaobject (hdlobject h, boolean flmajorrecalc) {
	
	return (IOArecalcobjectvalue (h));
	} /*recalcformulaobject*/
	
	
void setupconfig (tyconfigrecord *config) {

	IOAcopystring ("\pFormula", (*config).objectTypeName);
	
	(*config).objectTypeID = formulatype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = false;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).initObjectCallback = initformula;
	
	(*config).drawObjectCallback = drawformula;
	
	(*config).cleanupObjectCallback = cleanformula;
	
	(*config).recalcObjectCallback = recalcformulaobject;
	
	(*config).canReplicateObjectCallback = canreplicateformula;
	
	(*config).getObjectEditRectCallback = getformulaeditrect;
	
	(*config).getValueForScriptCallback = getformulavalue;
	
	(*config).debugObjectCallback = debugformula;
	} /*setupconfig*/
	
	
