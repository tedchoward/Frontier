
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


typedef struct tyrectdata {

	short versionnumber;
	
	short subtype; /*1: square corners, 2: rounded corners, 3: oval.*/
	
	short ovalwidth, ovalheight; /*if subtype is 2*/
	
	char waste [10];
	} tyrectdata, **hdlrectdata;
	


static boolean newrectdata (hdlrectdata *hdata) {

	hdlrectdata h;
	
	h = (hdlrectdata) NewHandleClear (longsizeof (tyrectdata));
	
	if (h == nil)
		return (false);
	
	(**h).versionnumber = 1;
	
	(**h).subtype = 1;
	
	*hdata = h;
	
	return (true);
	} /*newrectdata*/
	
	
static boolean cleanrect (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;
	
	width = IOAmakemultiple ((*r).right - (*r).left, gridunits);
	
	height = IOAmakemultiple ((*r).bottom - (*r).top, gridunits);
	
	(*r).right = (*r).left + width;

	(*r).bottom = (*r).top + height;
	
	return (true);
	} /*cleanrect*/
	

static boolean canreplicaterect (hdlobject h) {
	
	return (false); /*it can't be replicated*/
	} /*canreplicaterect*/
	

static boolean getrecteditrect (hdlobject h, Rect *r) {
	
	return (false); /*can't be edited*/
	} /*getrecteditrect*/
	

static boolean getrectvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getrectvalue*/
	

static boolean debugrectobject (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugrectobject*/
	

static boolean drawrect (hdlobject h) {
	
	hdlrectdata hdata = (hdlrectdata) (**h).objectdata;
	boolean fltransparent = (**h).objecttransparent;
	Rect r;
	
	r = (**h).objectrect;
	
	IOApushforecolor (&(**h).objectframecolor);
	
	switch ((**hdata).subtype) {
		
		case 1: /*square corners*/
			if (!fltransparent)
				EraseRect (&r);
	
			FrameRect (&r);
			
			break;
			
		case 2: { /*rounded corners*/
			short width = (**hdata).ovalwidth;
			short height = (**hdata).ovalheight;
			
			if (!fltransparent)
				EraseRoundRect (&r, width, height);
	
			FrameRoundRect (&r, width, height);
			
			break;
			}
			
		case 3: /*oval*/
			if (!fltransparent)
				EraseOval (&r);
	
			FrameOval (&r);
			
			break;
		} /*switch*/
		
	IOApopforecolor ();
	
	return (true);
	} /*drawrect*/
	

static boolean initrect (tyobject *obj) {
	
	newrectdata ((hdlrectdata *) &(*obj).objectdata);
	
	return (false); /*we don't want to edit it*/
	} /*initrect*/
	
	
static boolean unpackrectdata (hdlobject h) {
	
	hdlrectdata hdata = (hdlrectdata) (**h).objectdata;
	
	if (hdata == nil) { /*pre 1.0b15 object, no data handle*/
		
		if (!newrectdata (&hdata))
			return (false);
			
		(**h).objectdata = (Handle) hdata;
		
		return (true);
		}
		
	return (true);
	} /*unpackrectdata*/


static boolean clickrect (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	IOArunbuttonscript (h);
	
	return (true); /*do a minor recalc*/
	} /*clickrect*/
	
	
static boolean getrectattributes (hdlobject h, AppleEvent *event) {
	
	hdlrectdata hdata = (hdlrectdata) (**h).objectdata;
	
	IACglobals.event = event; 
	
	if (!IACpushshortparam ((**hdata).subtype, 'subt'))
		return (false);
	
	if (!IACpushshortparam ((**hdata).ovalwidth, 'owid'))
		return (false);
	
	if (!IACpushshortparam ((**hdata).ovalheight, 'ohei'))
		return (false);
	
	return (true);
	} /*getrectattributes*/
	
	
static boolean setrectattributes (hdlobject h, AppleEvent *event) {
	
	hdlrectdata hdata = (hdlrectdata) (**h).objectdata;
	tyrectdata info = **hdata;
	
	IACglobals.event = event; 
	
	IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('subt', &info.subtype); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('owid', &info.ovalwidth); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('ohei', &info.ovalheight); 
	
	**hdata = info;
	
	return (true);
	} /*setrectattributes*/
	
	
void setuprect (tyioaconfigrecord *);


void setuprect (tyioaconfigrecord *config) {

	IOAcopystring ("\pRectangle", (*config).objectTypeName);
	
	(*config).objectTypeID = recttype;
	
	(*config).frameWhenEditing = false; /*can't be edited*/
	
	(*config).canEditValue = false;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).isFontAware = false;
	
	(*config).hasSpecialCard = true;

	(*config).initObjectCallback = initrect;
	
	(*config).drawObjectCallback = drawrect;
	
	(*config).clickObjectCallback = clickrect;
	
	(*config).cleanupObjectCallback = cleanrect;
	
	(*config).canReplicateObjectCallback = canreplicaterect;
	
	(*config).getObjectEditRectCallback = getrecteditrect;
	
	(*config).getValueForScriptCallback = getrectvalue;
	
	(*config).debugObjectCallback = debugrectobject;
	
	(*config).unpackDataCallback = unpackrectdata;
	
	(*config).getAttributesCallback = getrectattributes;
	
	(*config).setAttributesCallback = setrectattributes;
	} /*setuprect*/
	
	
