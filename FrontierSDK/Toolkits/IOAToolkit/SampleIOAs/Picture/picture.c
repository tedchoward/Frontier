
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>


#define dummystring "\p(paste picture here)"



static boolean cleanpicture (hdlobject h, short height, short width, Rect *r) {

	PicHandle hpict = (PicHandle) (**h).objectdata;
	
	if (hpict == nil) {
		
		hdlcard hc = (**h).owningcard;
		short gridunits = (**hc).gridunits;
		
		width = IOAmakemultiple (StringWidth (dummystring) + 4, gridunits);
		
		height = IOAmakemultiple ((*r).bottom - (*r).top, gridunits);
		}
	else {
		Rect rframe = (**hpict).picFrame;
		
		width = rframe.right - rframe.left;
		
		height = (rframe.bottom - rframe.top);
		}
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + height;
	
	return (true);
	} /*cleanpicture*/
	

static boolean canreplicatepicture (hdlobject h) {
	
	return (true); /*it can be replicated*/
	} /*canreplicatepicture*/
	

static boolean getpictureeditrect (hdlobject h, Rect *r) {

	return (false); /*text of object can't be edited*/
	} /*getpictureeditrect*/
	

static boolean getpicturevalue (hdlobject h, Handle *hvalue) {
	
	return (false); /*picture doesn't have a value for a script*/
	} /*getpicturevalue*/
	

static boolean debugpicture (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugpicture*/
	

static boolean drawpicture (hdlobject h) {
	
	Rect robject;
	PicHandle hpict;
	
	robject = (**h).objectrect;
	
	if (!(**h).objecttransparent)		
		EraseRect (&robject);
	
	hpict = (PicHandle) (**h).objectdata;
	
	if (hpict == nil) {
		
		bigstring bs;
		
		IOAcopystring (dummystring, bs);
		
		IOAcenterstring (robject, bs);
		}
	else {
		Rect rdraw = (**hpict).picFrame;
	
		IOAcenterrect (&rdraw, robject);
	
		DrawPicture (hpict, &rdraw);
		}
	
	return (true);
	} /*drawpicture*/
	

static boolean initpicture (tyobject *obj) {
	
	(*obj).objecthasframe = true;
	
	(*obj).objectfont = geneva;

	(*obj).objectfontsize = 9;

	(*obj).objectstyle = bold;

	return (false); /*we do not want to edit it*/
	} /*initpicture*/
	
	
void setuppicture (tyconfigrecord *);


void setuppicture (tyconfigrecord *config) {

	IOAcopystring ("\pPicture", (*config).objectTypeName);
	
	(*config).objectTypeID = picturetype;
	
	(*config).frameWhenEditing = false;
	
	(*config).canEditValue = false;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).isFontAware = false;
	
	(*config).initObjectCallback = initpicture;
	
	(*config).drawObjectCallback = drawpicture;
	
	(*config).cleanupObjectCallback = cleanpicture;
	
	(*config).canReplicateObjectCallback = canreplicatepicture;
	
	(*config).getObjectEditRectCallback = getpictureeditrect;
	
	(*config).getValueForScriptCallback = getpicturevalue;
	
	(*config).debugObjectCallback = debugpicture;
	} /*setuppicture*/
	
	
