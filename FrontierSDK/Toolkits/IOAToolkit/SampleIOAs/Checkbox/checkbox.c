
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>


#define checkboxinset 5 /*the number of pixels to skip between checkbox and text*/
#define textvertinset 2



static void getcheckboxrect (hdlobject h, Rect r, Rect *rbox) {
	
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
	} /*getcheckboxrect*/
	

static boolean cleancheckbox (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;
	
	width = IOAmakemultiple (width + 3 + (**h).objectfontsize + checkboxinset, gridunits);
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + height;
	
	return (true);
	} /*cleancheckbox*/
	

static boolean canreplicatecheckbox (hdlobject h) {
	
	return (true); /*it can be replicated*/
	} /*canreplicatecheckbox*/
	

static boolean getcheckboxeditrect (hdlobject h, Rect *r) {

	*r = (**h).objectrect;
		
	(*r).left += 3 + (**h).objectfontsize + checkboxinset;
	
	return (true); /*it can be edited*/
	} /*getcheckboxeditrect*/
	

static boolean getcheckboxvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetbooleanvalue (h, hvalue)); 
	} /*getcheckboxvalue*/
	

static boolean setcheckboxvalue (hdlobject h, Handle hvalue) {
	
	return (IOAsetbooleanvalue (h, hvalue));
	} /*setcheckboxvalue*/
	
	
static boolean debugcheckbox (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugcheckbox*/
	

static boolean drawcheckbox (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	Rect rbox;
	Rect r;
	Handle htext;
	
	htext = (**h).objectvalue; 
	
	r = (**h).objectrect;
	
	getcheckboxrect (h, r, &rbox);
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	FrameRect (&rbox);
	
	if ((**hc).tracking && (**hc).trackerpressed) {
		
		Rect rinset = rbox;
		
		InsetRect (&rinset, 1, 1);
		
		FrameRect (&rinset);
		}
	
	if ((**h).objectflag) {
		
		MoveTo (rbox.left, rbox.top);
		
		LineTo (rbox.right - 1, rbox.bottom - 1);
		
		MoveTo (rbox.right - 1, rbox.top);
		
		LineTo (rbox.left, rbox.bottom - 1);
		}
		
	if (!(**hc).flskiptext) {
		
		Rect rtext;
		
		getcheckboxeditrect (h, &rtext);
		
		IOAeditdrawtexthandle (htext, rtext, (**h).objectjustification);
		}
	
	return (true);
	} /*drawcheckbox*/
	
	
static boolean initcheckbox (tyobject *obj) {
	
	return (true); /*nothing special, we do want to edit it*/
	} /*initcheckbox*/
	
	
static boolean recalccheckbox (hdlobject h, boolean flmajorrecalc) {
	
	bigstring errorstring;
	Handle hboolvalue;
	
	if (!IOAevalscript (h, (**h).objectrecalcscript, &hboolvalue, (**h).objectlanguage, errorstring))
		return (false);
		
	setcheckboxvalue (h, hboolvalue); /*also disposes of hboolvalue*/
	
	return (true); 
	} /*recalccheckbox*/
	

static boolean clickcheckbox (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	(**h).objectflag = !(**h).objectflag;
	
	(**(**h).owningcard).runtimevaluechanged = true; /*DW 9/19/95*/
	
	IOAinvalobject (h);
	
	IOArunbuttonscript (h); /*1.0b15 -- run the action script*/
	
	return (true); /*cause a minor recalc*/
	} /*clickcheckbox*/
	

void setupcheckbox (tyconfigrecord *); /*prototype*/

	
void setupcheckbox (tyconfigrecord *config) {

	IOAcopystring ("\pCheckbox", (*config).objectTypeName);
	
	IOAcopystring ("\pChecked", (*config).objectFlagName);
	
	(*config).objectTypeID = checkboxtype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = true;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).initObjectCallback = initcheckbox;
	
	(*config).drawObjectCallback = drawcheckbox;
	
	(*config).clickObjectCallback = clickcheckbox;
	
	(*config).cleanupObjectCallback = cleancheckbox;
	
	(*config).recalcObjectCallback = recalccheckbox;
	
	(*config).canReplicateObjectCallback = canreplicatecheckbox;
	
	(*config).getObjectEditRectCallback = getcheckboxeditrect;
	
	(*config).getValueForScriptCallback = getcheckboxvalue;
	
	(*config).setValueFromScriptCallback = setcheckboxvalue;
	
	(*config).debugObjectCallback = debugcheckbox;
	} /*setupcheckbox*/
	
	
