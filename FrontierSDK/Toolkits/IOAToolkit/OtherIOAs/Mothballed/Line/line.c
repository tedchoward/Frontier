
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>



static boolean cleanline (hdlobject h, short height, short width, Rect *r) {

	register hdlcard hc = (**h).owningcard;
	register short gridunits = (**hc).gridunits;
	
	width = IOAmakemultiple ((*r).right - (*r).left, gridunits);
	
	height = IOAmakemultiple ((*r).bottom - (*r).top, gridunits);
	
	(*r).right = (*r).left + width;

	(*r).bottom = (*r).top + height;
	} /*cleanline*/
	

static boolean canreplicateline (hdlobject h) {
	
	return (false); /*it can't be replicated*/
	} /*canreplicateline*/
	

static boolean getlineeditrect (hdlobject h, Rect *r) {
	
	return (false); /*can't be edited*/
	} /*getlineeditrect*/
	

static boolean getlinevalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getlinevalue*/
	

static boolean debugline (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugline*/
	

static boolean drawline (hdlobject h) {
	
	Rect r;
	
	r = (**h).objectrect;
	
	MoveTo (r.left, r.top);
	
	LineTo (r.right, r.bottom);
	} /*drawline*/
	

static boolean initline (tyobject *obj) {
	
	return (false); /*we don't want to edit it*/
	} /*initline*/
	
	
void setupconfig (tyconfigrecord *config) {

	IOAcopystring ("\pLine", (*config).objectTypeName);
	
	(*config).objectTypeID = linetype;
	
	(*config).frameWhenEditing = false; /*can't be edited*/
	
	(*config).canEditValue = false;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).isFontAware = false;
	
	(*config).initObjectCallback = initline;
	
	(*config).drawObjectCallback = drawline;
	
	(*config).cleanupObjectCallback = cleanline;
	
	(*config).canReplicateObjectCallback = canreplicateline;
	
	(*config).getObjectEditRectCallback = getlineeditrect;
	
	(*config).getValueForScriptCallback = getlinevalue;
	
	(*config).debugObjectCallback = debugline;
	} /*setupconfig*/
	
	
