
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

#include "frontier.h"
#include "standard.h"

#include "quickdraw.h"
#include <ioa.h>

#if TARGET_API_MAC_CARBON == 1

	#include "memory.h"
	
#endif


#define textvertinset 2


static boolean cleanbutton (hdlobject h, short height, short width, Rect *r) {
	
	/*
	dmb 1.0b20 -- don't alter font-based measurements for standard system 
	font buttons. i.e. use the height that is passed to us.
	*/
	
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
	} /*cleanbutton*/
	

static boolean canreplicatebutton (hdlobject h) {
	
	return (true); /*it can be replicated*/
	} /*canreplicatebutton*/
	

static boolean getbuttoneditrect (hdlobject h, Rect *r) {
	
	short height, width, extrapixels, offtop;
	
	*r = (**h).objectrect;
		
	IOAgetobjectsize (h, &height, &width);
	
	extrapixels = ((*r).bottom - (*r).top) - height;
	
	offtop = extrapixels / 2;
	
	(*r).top += offtop;
	
	(*r).bottom -= extrapixels - offtop;
	
	return (true); /*can be edited*/
	} /*getbuttoneditrect*/
	

static boolean getbuttonvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getbuttonvalue*/
	

static boolean debugbutton (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugbutton*/
	

#if TARGET_API_MAC_CARBON == 1

	static void MyThemeButtonDrawCallback (const Rect *bounds, ThemeButtonKind kind, const ThemeButtonDrawInfo *info,
		UInt32 refcon, SInt16 depth, Boolean isColorDev) {
		
		/*
		7.0b48 PBS: draw the label for a popup menu.
		*/
		
		Handle htext = (Handle) refcon;
		ThemeFontID idfont;
		bigstring fontname, bs;
		short fontsize;
		Style fontstyle;
		Rect r = *bounds;
		
		texthandletostring (htext, bs);
		
		GetThemeFont (kThemePushButtonFont, smSystemScript, fontname, &fontsize, &fontstyle);
		
		fontgetnumber (fontname, &idfont);
		
		pushstyle (geneva, 9, bold);

		//movepento ((*bounds).left, (*bounds).top + 10);
		
		r = *bounds;
		r.top = r.top + 3;
		
		centerbuttonstring (&r, bs, false);
		
		popstyle ();
		} /*MyThemeButtonDrawCallback*/
		
#endif

static boolean drawbutton (hdlobject h) { 
	
	/*
	DW 8/15/93: commented the pushing of the frame color before drawing
	the button outline. use the text color as set by the shell. makes 
	disabled buttons look right.
	
	2002-10-13 AR: Reorganized variable declarations
	to eliminate compiler warnings about unused variables.
	*/
	
	hdlcard hc = (**h).owningcard;
	Rect r;
	boolean flbold;
	Handle htext;
	
	htext = (**h).objectvalue;
	
	r = (**h).objectrect;
	
	flbold = (**h).objectflag;
	
	#if TARGET_API_MAC_CARBON == 1
	
		{
		ThemeButtonDrawInfo drawinfo;
		ThemeButtonDrawUPP drawupp;
		
		InsetRect (&r, -2, 0);
		
		drawupp = NewThemeButtonDrawUPP (MyThemeButtonDrawCallback);

		drawinfo.state = kThemeButtonOn;
		
		drawinfo.value = kThemeStateActive;
		
		if ((**hc).tracking && (**hc).trackerpressed)
			drawinfo.value = kThemeStatePressed;
		
		drawinfo.adornment = 0;
		
		if (flbold) /*default item*/
			drawinfo.adornment = kThemeAdornmentDefault;
		
		DrawThemeButton (&r, kThemePushButton, &drawinfo, nil, nil, drawupp, (unsigned long) htext); 
		
		//DisposeThemeButtonDrawUPP (MyThemeButtonDrawCallback);
		DisposeThemeButtonDrawUPP (drawupp);		
		}

	#else
		
		{
		Rect routset;

		if (!(**h).objecttransparent)
			EraseRoundRect (&r, 9, 9);
				
		routset = r;
		
		if (flbold)
			InsetRect (&routset, -4, -4);
			
		if (!(**hc).flskiptext) {
		
			Rect redit;
			
			getbuttoneditrect (h, &redit);
			
			IOAeditdrawtexthandle (htext, redit, (**h).objectjustification);
			}
				
		/*IOApushforecolor (&(**h).objectframecolor);*/
		
		FrameRoundRect (&r, 9, 9); /*must be after text drawing, 4/27/92 DW*/
		
		if (flbold) {
		
			PenState x;
			
			GetPenState (&x); /*save the old pen state*/
			
			PenSize (3, 3); /*make the pen fatter*/
			
			FrameRoundRect (&routset, 16, 16); /*draw the ring*/
			
			SetPenState (&x); /*restore the pen state*/
			}
			
		/*IOApopforecolor ();*/
			
		if ((**hc).tracking && (**hc).trackerpressed) {
		
			Rect rinvert = r;
			
			InsetRect (&rinvert, 1, 1);
			
			InvertRoundRect (&rinvert, 9, 9);
			}
		}
		
	#endif
	
	return (true);
	} /*drawbutton*/
	

static boolean flbold;


static boolean initbuttonvisit (hdlobject h) {
	
	if ((**h).objectflag) {
		
		flbold = false; /*already is a bold button*/
		
		return (false); /*stop the traversal*/
		}
		
	return (true); /*keep looking*/
	} /*initbuttonvisit*/
	
	
static boolean initbutton (tyobject *obj) {
	
	hdlcard hc = (*obj).owningcard;
	
	flbold = true;
	
	IOAvisitobjects ((**hc).objectlist, &initbuttonvisit);
	
	(*obj).objectflag = flbold; 
			
	(*obj).objectjustification = centerjustified;
	
	return (true); /*we do want to edit it*/
	} /*initbutton*/
	
	
static boolean getbuttoninvalrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	if ((**h).objectflag)
		InsetRect (r, -4, -4);
	
	return (true);
	} /*getbuttoninvalrect*/
	
	
static boolean recalcbutton (hdlobject h, boolean flmajorrecalc) {
	
	bigstring errorstring;
	Handle hvalue;
	
	if (!IOAevalscript (h, (**h).objectrecalcscript, &hvalue, (**h).objectlanguage, errorstring))
		return (false);
		
	IOAsetobjectvalue (h, hvalue); 
	
	IOAinvalobject (h);
	
	return (true); 
	} /*recalcbutton*/


static boolean clickbutton (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	IOArunbuttonscript (h);
	
	return (true); /*do a minor recalc*/
	} /*clickbutton*/
	
	
static boolean catchreturnbutton (hdlobject h) {
	
	return ((**h).objectflag); /*if the button is bold, we want the Return key*/
	} /*catchreturnbutton*/
	

void setupbutton (tyioaconfigrecord *);


void setupbutton (tyioaconfigrecord *config) {
	
	IOAcopystring ("\pButton", (*config).objectTypeName);
	
	IOAcopystring ("\pDefault Button", (*config).objectFlagName);
	
	(*config).objectTypeID = buttontype;
	
	(*config).frameWhenEditing = false;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).hasSpecialCard = true;

	(*config).initObjectCallback = initbutton;
	
	(*config).drawObjectCallback = drawbutton;
	
	(*config).clickObjectCallback = clickbutton;
	
	(*config).cleanupObjectCallback = cleanbutton;
	
	(*config).recalcObjectCallback = recalcbutton;
	
	(*config).canReplicateObjectCallback = canreplicatebutton;
	
	(*config).catchReturnCallback = catchreturnbutton;
	
	(*config).getObjectInvalRectCallback = getbuttoninvalrect;
	
	(*config).getObjectEditRectCallback = getbuttoneditrect;
	
	(*config).getValueForScriptCallback = getbuttonvalue;
	
	(*config).debugObjectCallback = debugbutton;
	} /*setupconfig*/
	
	
