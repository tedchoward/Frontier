
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ioa.h>


#define radioinset 5 /*the number of pixels to skip between oval and text*/


#define textvertinset 2


static void getradiorect (hdlobject h, Rect r, Rect *rbox) {
	
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
	} /*getradiorect*/
	

static boolean cleanradio (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;
	
	width = IOAmakemultiple (width + 3 + (**h).objectfontsize + radioinset, gridunits);
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + height;
	
	return (true);
	} /*cleanradio*/
	

static boolean canreplicateradio (hdlobject h) {
	
	return (true); /*it can be replicated*/
	} /*canreplicateradio*/
	

static boolean getradioeditrect (hdlobject h, Rect *r) {

	*r = (**h).objectrect;
		
	(*r).left += 3 + (**h).objectfontsize + radioinset;
	
	return (true); /*it can be edited*/
	} /*getradioeditrect*/
	

static boolean getradiovalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetbooleanvalue (h, hvalue)); 
	} /*getradiovalue*/
	

static boolean setradiovalue (hdlobject h, Handle hvalue) {
	
	return (IOAsetbooleanvalue (h, hvalue));
	} /*setradiovalue*/
	
	
static boolean debugradio (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugradio*/
	

static boolean drawradio (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	Rect rbox;
	Handle htext;
	Rect r;
	
	htext = (**h).objectvalue;
	
	r = (**h).objectrect;
	
	getradiorect (h, r, &rbox);
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	FrameOval (&rbox);
	
	if ((**hc).tracking && (**hc).trackerpressed) {
		
		Rect rinset = rbox;
		
		InsetRect (&rinset, 1, 1);
		
		FrameOval (&rinset);
		}
	
	if ((**h).objectflag) {
		
		Rect rblack = rbox;
		
		InsetRect (&rblack, 3, 3);
		
		FillOval (&rblack, &quickdrawglobal (black));
		}
	
	if (!(**hc).flskiptext) {
	
		Rect rtext;
	
		getradioeditrect (h, &rtext);
		
		IOAeditdrawtexthandle (htext, rtext, (**h).objectjustification);
		}
	
	return (true);
	} /*drawradio*/
	

static boolean initradio (tyobject *obj) {
	
	return (true); /*nothing special, we do want to edit it*/
	} /*initradio*/
	
	
static boolean recalcradio (hdlobject h, boolean flmajorrecalc) {
	
	bigstring errorstring;
	Handle hboolvalue;
	
	if (!IOAevalscript (h, (**h).objectrecalcscript, &hboolvalue, (**h).objectlanguage, errorstring))
		return (false);
		
	setradiovalue (h, hboolvalue); /*also disposes of hboolvalue*/
	
	return (true); 
	} /*recalcradio*/
	

static boolean clickradio (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	/*
	the one you hit goes on, all others go off
	*/
					
	hdlobject nomad = listhead;
	boolean newflag;
	
	while (nomad != nil) {
		
		if ((**nomad).objecttype == radiobuttontype) {
		
			newflag = (nomad == h) && (**nomad).objectenabled;
			
			if ((**nomad).objectflag != newflag) {
				
				(**nomad).objectflag = newflag;
				
				IOAinvalobject (nomad);
				
				(**(**nomad).owningcard).runtimevaluechanged = true; /*DW 9/19/95*/
				
				/*runmodeupdate ();*/
				}
			}
		
		nomad = (**nomad).nextobject;
		} /*while*/

	IOArunbuttonscript (h); /*1.0b15 -- run the action script*/
	
	return (true); /*cause a minor recalc*/
	} /*clickradio*/
	

void setupradio (tyconfigrecord *);

	
void setupradio (tyconfigrecord *config) {

	IOAcopystring ("\pRadio button", (*config).objectTypeName);
	
	IOAcopystring ("\pSelected", (*config).objectFlagName);
	
	(*config).objectTypeID = radiobuttontype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = true;
	
	(*config).mutuallyExclusive = true;
	
	(*config).speaksForGroup = true;
	
	(*config).initObjectCallback = initradio;
	
	(*config).drawObjectCallback = drawradio;
	
	(*config).clickObjectCallback = clickradio;
	
	(*config).cleanupObjectCallback = cleanradio;
	
	(*config).recalcObjectCallback = recalcradio;
	
	(*config).canReplicateObjectCallback = canreplicateradio;
	
	(*config).getObjectEditRectCallback = getradioeditrect;
	
	(*config).getValueForScriptCallback = getradiovalue;
	
	(*config).setValueFromScriptCallback = setradiovalue;
	
	(*config).debugObjectCallback = debugradio;
	} /*setupradio*/
	
	
