
/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

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

	
#if TARGET_API_MAC_CARBON == 1

	static void MyThemeButtonDrawCallback (const Rect *bounds, ThemeButtonKind kind, const ThemeButtonDrawInfo *info,
		bigstring bs, SInt16 depth, Boolean isColorDev) {
		
		//pushstyle (geneva, 10, 0);

		//movepento ((*bounds).left, (*bounds).top + 10);
		
		//pendrawstring (bs);
		
		//popstyle ();
		} /*MyThemeButtonDrawCallback*/

#endif


static boolean drawradio (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	Rect rbox;
	Handle htext;
	Rect r;
	
	htext = (**h).objectvalue;
	
	r = (**h).objectrect;
	
	getradiorect (h, r, &rbox);
	
	#if TARGET_API_MAC_CARBON == 1
	
		{
		ThemeButtonDrawUPP drawupp;
		ThemeButtonDrawInfo drawinfo;
		
		drawupp = NewThemeButtonDrawUPP (MyThemeButtonDrawCallback);
		
		if ((**h).objectflag)		
			drawinfo.state = kThemeButtonOn;
		else
			drawinfo.state = kThemeButtonOff;
		
		drawinfo.value = kThemeStateActive;
		
		drawinfo.adornment = 0;
				
		DrawThemeButton (&r, kThemeRadioButton, &drawinfo, nil, nil, drawupp, (unsigned long) nil);
		
		//DisposeThemeButtonDrawUPP (MyThemeButtonDrawCallback);
		DisposeThemeButtonDrawUPP (drawupp);
		}
		
	#else
	
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
		#if TARGET_API_MAC_CARBON == 1
		{
		Pattern black;
		GetQDGlobalsBlack(&black);
		FillOval (&rblack, &black);
		}
		#else
		FillOval (&rblack, &quickdrawglobal (black));
		#endif
		
		}
	
	#endif
	
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
	

void setupradio (tyioaconfigrecord *);

	
void setupradio (tyioaconfigrecord *config) {

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
	
	
