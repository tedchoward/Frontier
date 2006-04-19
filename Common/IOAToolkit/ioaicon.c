
/*	$Id$    */

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

#include <iac.h>
#include <ioa.h>
#include <appleticons.h>
#include "ioaicon.h"


#define iconheight 32
#define iconwidth 32

#define iconvertinset 3

#define textvertinset 2


typedef struct tyversion1icondata {
	
	short versionnumber;
	
	short resnum;
	
	unsigned short haslabel;
	
	unsigned short align: 4; /*DW 8/25/93*/
	
	unsigned short ixlabel: 3; /*DW 8/25/93*/
	
	/*1 more bit is available here*/
	
	Handle iconsuite; /*DW 8/25/93 -- if non-nil, it overrides resnum*/
	
	char waste [2];
	} tyversion1icondata, **hdlversion1icondata;




static boolean cleanicon (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	short gridunits = (**hc).gridunits;
	
	if ((**hdata).haslabel) {
	
		width = IOAmakemultiple (width, gridunits);
		
		if (width < iconwidth)
			width = iconwidth;
		
		height += iconvertinset + iconheight;
		}
	else {
		width = iconwidth;
		
		height = iconheight;
		}
	
	(*r).right = (*r).left + width;
	
	(*r).bottom = (*r).top + height;
	
	return (true);
	} /*cleanicon*/
	

static boolean canreplicateicon (hdlobject h) {
#pragma unused(h)

	return (true); /*it can be replicated*/
	} /*canreplicateicon*/
	

static boolean geticonvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*geticonvalue*/
	

static boolean debugicon (hdlobject h, bigstring errorstring) {
#pragma unused(h)

	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugicon*/
	

static short icontextheight (void) {
	
	FontInfo fi;
	short lh;
	
	GetFontInfo (&fi);
	
	lh = fi.ascent + fi.descent + (2 * textvertinset);
	
	if ((lh % 2) == 1) /*round up*/
		lh++;
	
	return (lh);
	} /*icontextheight*/


static boolean geticoneditrect (hdlobject h, Rect *redit) {
	
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	Rect r;
	
	if (!(**hdata).haslabel) /*can't be edited*/
		return (false);
	
	r = (**h).objectrect;
	
	r.top = r.bottom - icontextheight ();
	
	*redit = r;
	
	return (true); /*it can be edited*/
	} /*geticoneditrect*/
	

static boolean drawiconobject (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	boolean haslabel = (**hdata).haslabel;
	Rect r, ricon, rcontains, rtext;
	boolean flhot = false;
	short transform = 0;
	
	r = (**h).objectrect;
	
	rcontains = r;
	
	rcontains.bottom -= icontextheight () + iconvertinset;
	
	ricon = r;
	
	ricon.bottom = ricon.top + iconheight;
	
	ricon.right = ricon.left + iconwidth;
	
	IOAcenterrect (&ricon, rcontains);
	
	if (ricon.top < r.top)
		ricon.top = r.top;
		
	if (!haslabel) /*DW 6/15/93*/
		ricon = r;
		
	if ((**hc).tracking && (**hc).trackerpressed) {
	
		transform = 0x4000; /*display it selected*/
		
		flhot = true;
		}
		
	transform += ttLabel [(**hdata).ixlabel]; /*color it according to the label*/
	
	if ((**hdata).iconsuite != nil)
		ploticonsuite (&ricon, (**hdata).align, transform, (**hdata).iconsuite);
	else
		ploticonresource (&ricon, (**hdata).align, transform, (**hdata).resnum);
	
	if (haslabel) {
	
		geticoneditrect (h, &rtext);
	
		IOAeditdrawtexthandle ((**h).objectvalue, rtext, (**h).objectjustification);
	
		if (flhot) {
		
			InsetRect (&r, 2, 2);
		
			InvertRect (&rtext);
			}
		}
		
	return (true);
	} /*drawiconobject*/
	
	
static boolean recalciconobject (hdlobject h, boolean flmajorrecalc) {
#pragma unused(flmajorrecalc)

	return (IOArecalcobjectvalue (h));
	} /*recalciconobject*/
	
	
static boolean newicondata (hdlicondata *hdata) {

	hdlicondata h;
	
	h = (hdlicondata) NewHandleClear (longsizeof (tyicondata));
	
	if (h == nil)
		return (false);
	
	(**h).versionnumber = 2;
	
	(**h).resnum = 128;
	
	(**h).haslabel = true;
	
	(**h).align = kAlignAbsoluteCenter;
	
	*hdata = h;
	
	return (true);
	} /*newicondata*/
	
	
static boolean initicon (tyobject *obj) {
	
	(*obj).objectautosize = true; /*it's kind of senseless to have wierd sized icons*/
	
	(*obj).objectfont = geneva;

	(*obj).objectfontsize = 9;

	(*obj).objectjustification = centerjustified;
	
	newicondata ((hdlicondata *) &(*obj).objectdata);
	
	return (true); /*we do want to edit it*/
	} /*initicon*/
	
	
static boolean geticonattributes (hdlobject h, AppleEvent *event) {
	
	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	
	IACglobals.event = event; 
	
	if (!IACpushshortparam ((**hdata).resnum, 'rsnm'))
		return (false);
		
	if (!IACpushbooleanparam ((**hdata).haslabel, 'hlbl'))
		return (false);
		
	return (true);
	} /*geticonattributes*/
	
	
static boolean seticonattributes (hdlobject h, AppleEvent *event) {
	
	hdlicondata hdata = (hdlicondata) (**h).objectdata;	
	Boolean haslabel;
	short resnum;
	
	IACglobals.event = event; 
	
	IACglobals.nextparamoptional = true;

	if (IACgetbooleanparam ('hlbl', &haslabel)) 
		(**hdata).haslabel = haslabel; 
	
	IACglobals.nextparamoptional = true;
	
	if (IACgetshortparam ('rsnm', &resnum))
		(**hdata).resnum = resnum;
	
	return (true);
	} /*seticonattributes*/
	
	
static boolean unpackicondata (hdlobject h) {

	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	
	if (hdata == nil) {
		
		newicondata (&hdata);
		
		(**h).objectdata = (Handle) hdata;
		
		return (hdata != nil);
		}
	
	if ((**hdata).versionnumber == 1) { /*convert from version 1 to version 2*/
		
		tyversion1icondata x;
		
		BlockMove (*hdata, &x, longsizeof (tyversion1icondata));
		
		DisposeHandle ((Handle) hdata);
		
		hdata = (hdlicondata) NewHandleClear (longsizeof (tyicondata));
		
		BlockMove (&x, *hdata, longsizeof (tyversion1icondata));
		
		(**hdata).versionnumber = 2;
		
		(**h).objectdata = (Handle) hdata;
		}
		
	return (true);
	} /*unpackicondata*/
	
	
static boolean clickicon (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
#pragma unused(listhead, pt, flshiftkey)

	hdlicondata hdata = (hdlicondata) (**h).objectdata;
	tyiconclickcallback lcallback = (**hdata).iconclick;
	
	if (lcallback != nil)
		return ((*lcallback) (h, fl2click));
	
	if (fl2click)
		IOArunbuttonscript (h);
	
	return (true); /*do a minor recalc*/
	} /*clickicon*/
	

void setupicon (tyioaconfigrecord *);

	
void setupicon (tyioaconfigrecord *config) {

	IOAcopystring ("\pIcon", (*config).objectTypeName);
	
	(*config).objectTypeID = icontype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).isFontAware = true;
	
	(*config).hasSpecialCard = true;
	
	(*config).initObjectCallback = initicon;
	
	(*config).drawObjectCallback = drawiconobject;
	
	(*config).clickObjectCallback = clickicon;
	
	(*config).recalcObjectCallback = recalciconobject;
	
	(*config).cleanupObjectCallback = cleanicon;
	
	(*config).canReplicateObjectCallback = canreplicateicon;
	
	(*config).getObjectEditRectCallback = geticoneditrect;
	
	(*config).getValueForScriptCallback = geticonvalue;
	
	(*config).debugObjectCallback = debugicon;
	
	(*config).getAttributesCallback = geticonattributes;
	
	(*config).setAttributesCallback = seticonattributes;
		
	(*config).unpackDataCallback = unpackicondata;
	} /*setupicon*/
	
	
