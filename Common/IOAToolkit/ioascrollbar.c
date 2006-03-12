
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

//Code change by Timothy Paustian Sunday, May 7, 2000 8:55:04 PM
//Changed to Opaque call for Carbon

typedef struct tyscrollbardata {
	
	short versionnumber;
	
	short controlresourceid;
	
	short controlmin;
	
	short controlmax;
	
	short controlinitialvalue;
	
	ControlHandle hcontrol;
	
	char waste [16];
	} tyscrollbardata, **hdlscrollbardata;


static boolean checkcontrolexists (hdlobject h) {

	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	hdlcard hcard = (**h).owningcard;
	ControlHandle hcontrol = (**hdata).hcontrol;
	
	if (hcontrol != nil)
		return (true);
		
	hcontrol = GetNewControl ((**hdata).controlresourceid, (**hcard).drawwindow);
		
	(**hdata).hcontrol = hcontrol;
	//Code change by Timothy Paustian Sunday, May 7, 2000 8:46:46 PM
	//Changed to Opaque call for Carbon
	SetControlMinimum(hcontrol, (**hdata).controlmin);
	SetControlMaximum(hcontrol, (**hdata).controlmax);
	SetControlValue(hcontrol, (**hdata).controlinitialvalue);
	
	/*(**hcontrol).contrlMin = (**hdata).controlmin;
	
	(**hcontrol).contrlMax = (**hdata).controlmax;
	
	(**hcontrol).contrlValue = (**hdata).controlinitialvalue;*/
	
	return (hcontrol != nil);
	} /*checkcontrolexists*/
	

static boolean cleanscrollbar (hdlobject h, short textheight, short textwidth, Rect *r) {
	
	short height, width;
	
	if (!checkcontrolexists (h))
		return (false);
		
	height = (*r).bottom - (*r).top;
	
	width = (*r).right - (*r).left;
	
	if (height > width) { /*it's a vertical scrollbar*/
		
		width = IOAclosestmultiple (width, 16);
		
		(*r).right = (*r).left + width;
		}
	else { /*it's a horizontal scrollbar*/
		
		height = IOAclosestmultiple (height, 16);
		
		(*r).bottom = (*r).top + height;
		}
		
	return (true);
	} /*cleanscrollbar*/
	

static pascal void trackscrollarrow (hdlscrollbar sb, short part) {
	
	short val = GetControlValue(sb);
	short minval = GetControlMinimum(sb);
	short maxval = GetControlMaximum(sb);
	
	/*short val = (**sb).contrlValue;
	short minval = (**sb).contrlMin;
	short maxval = (**sb).contrlMax;*/
	short pageamount = (maxval - minval) / 10;
	
	switch (part) {
	
		case kControlUpButtonPart:
			val--;
			
			break;
			
		case kControlDownButtonPart:
			val++;
			
			break;
			
		case kControlPageUpPart:
			val -= pageamount;
			
			break;
						
		case kControlPageDownPart:
			val += pageamount;
			
			break;
		} /*switch*/
	
	if (val < minval)
		val = minval;
		
	if (val > maxval)
		val = maxval;
		
	SetControlValue (sb, val);
	} /*trackscrollarrow*/


	
#if TARGET_RT_MAC_CFM
	
	#if TARGET_API_MAC_CARBON == 1
	//Code change by Timothy Paustian Friday, July 21, 2000 9:55:14 PM
	//Is this the right UPP? I think so. I had to move this here so I could create
	//the UPP in IOAregistercomponents ioa.c
	#define trackscrollarrowUPP (trackscrollarrow)
	#else
	
	static RoutineDescriptor trackscrollarrowDesc = BUILD_ROUTINE_DESCRIPTOR (uppControlActionProcInfo, trackscrollarrow);
	
	#define trackscrollarrowUPP (&trackscrollarrowDesc)
	#endif
		

#else

	#define trackscrollarrowUPP (&trackscrollarrow)
	
#endif
		
static boolean clickscrollbar (hdlobject listhead, hdlobject h, Point pt, boolean flshiftkey, boolean fl2click) {
	
	hdlcard hc = (**h).owningcard;
	
	if (!checkcontrolexists (h))
		return (false);
			
	(**hc).runmodechanges = true; /*indicate that one of the objects in the card changed*/
	
	(**hc).recalcneeded = true; /*force recalc of all objects that calc when another changes*/
	
	(**hc).recalctickcount = TickCount (); /*do it asap*/
	
	/*do the actual tracking*/ {
		short scrollbarpart;
		ControlHandle sb;
		
		scrollbarpart = FindControl (pt, (**hc).drawwindow, &sb);
		
		assert (sb == (**((hdlscrollbardata) (**h).objectdata)).hcontrol);
		
		if (scrollbarpart == kControlIndicatorPart) 
			TrackControl (sb, pt, nil);
		else
			TrackControl (sb, pt, trackscrollarrowUPP);
		}
	
	return (true);
	} /*clickscrollbar*/


static boolean drawscrollbar (hdlobject h) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	ControlHandle hcontrol;
	Rect r = (**h).objectrect;
	
	if (!checkcontrolexists (h))
		return (false);
		
	hcontrol = (**hdata).hcontrol;
		
	if (hcontrol == nil) {
		//Code change by Timothy Paustian Sunday, June 25, 2000 10:54:09 PM
		//
		#if TARGET_API_MAC_CARBON == 1
		Pattern gray;
		GetQDGlobalsBlack(&gray);
		FillRect (&r, &gray);
		#else
		FillRect (&r, &quickdrawglobal(gray));
		#endif
		return (true);
		}
	//Code change by Timothy Paustian Sunday, May 7, 2000 8:52:54 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	SetControlBounds(hcontrol, &r);
	#else
	(**hcontrol).contrlRect = r;
	#endif
	//Code change by Timothy Paustian Sunday, May 7, 2000 8:53:47 PM
	//Changed to Opaque call for Carbon
	{
		boolean	visControl;
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		visControl = IsControlVisible(hcontrol);
		#else
		visControl = (**hcontrol).contrlVis;
		#endif
		
		if(visControl)
		{
			Draw1Control (hcontrol);
		}
		else
		{
			ShowControl (hcontrol);
		}	
	}
	return (true);
	} /*drawscrollbar*/
	
	
static boolean newscrollbardata (hdlscrollbardata *hdata) {

	hdlscrollbardata h;
	
	h = (hdlscrollbardata) NewHandleClear (longsizeof (tyscrollbardata));
	
	if (h == nil)
		return (false);
	
	(**h).versionnumber = 1;
	
	(**h).controlresourceid = 1024;
	
	(**h).controlmin = 0;
	
	(**h).controlinitialvalue = 0;
	
	(**h).controlmax = 99;
	
	*hdata = h;
	
	return (true);
	} /*newscrollbardata*/
	
	
static boolean initscrollbar (tyobject *obj) {
	
	hdlscrollbardata hdata;
	
	if (!newscrollbardata (&hdata))
		return (false);
	
	(*obj).objectdata = (Handle) hdata;
	
	return (false); /*we don't want to edit it*/
	} /*initscrollbar*/
	
	
static boolean recalcscrollbar (hdlobject h, boolean flmajorrecalc) {
	
	return (IOArecalcobjectvalue (h));
	} /*recalcscrollbar*/
	

static boolean getscrollbarinvalrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
	
	return (true);
	} /*getscrollbarinvalrect*/
	
	
static boolean canreplicatescrollbar (hdlobject h) {
	
	return (true); 
	} /*canreplicatescrollbar*/
	

static boolean getscrollbareditrect (hdlobject h, Rect *r) {
	
	return (false); /*can't be edited*/
	} /*getscrollbareditrect*/
	

static boolean getscrollbarvalue (hdlobject h, Handle *hvalue) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	bigstring bs, bscontrolvalue;
	
	if (!checkcontrolexists (h))
		return (false);
		
	IOAcopystring ("\p\"", bs);

	NumToString (GetControlValue ((**hdata).hcontrol), bscontrolvalue);
	
	IOApushstring (bscontrolvalue, bs);
	
	IOApushstring ("\p\"", bs);
	
	return (IOAnewtexthandle (bs, hvalue));
	} /*getscrollbarvalue*/
	

static boolean debugscrollbar (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugscrollbar*/
	

static boolean getscrollbarattributes (hdlobject h, AppleEvent *event) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	
	IACglobals.event = event; 
	
	if (!IACpushshortparam ((**hdata).controlresourceid, 'sbid'))
		return (false);
	
	if (!IACpushshortparam ((**hdata).controlmin, 'sbmn'))
		return (false);
	
	if (!IACpushshortparam ((**hdata).controlmax, 'sbmx'))
		return (false);
	
	if (!IACpushshortparam ((**hdata).controlinitialvalue, 'sbin'))
		return (false);
	
	return (true);
	} /*getscrollbarattributes*/
	
	
static boolean setscrollbarattributes (hdlobject h, AppleEvent *event) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	tyscrollbardata info = **hdata;
	
	IACglobals.event = event; 
	
	IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sbid', &info.controlresourceid); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sbmn', &info.controlmin); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sbmx', &info.controlmax); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sbin', &info.controlinitialvalue); IACglobals.nextparamoptional = true;
	
	**hdata = info;
	
	/*update the control handle to reflect new numbers*/ {
		
		ControlHandle hcontrol = (**hdata).hcontrol;
		//Code change by Timothy Paustian Sunday, May 7, 2000 8:54:51 PM
		//Changed to Opaque call for Carbon
		SetControlMinimum(hcontrol, info.controlmin);
		SetControlMaximum(hcontrol, info.controlmax);
		SetControlValue(hcontrol, info.controlinitialvalue);
		/*(**hcontrol).contrlMin = info.controlmin;
	
		(**hcontrol).contrlMax = info.controlmax;
	
		(**hcontrol).contrlValue = info.controlinitialvalue;*/
		}
	
	return (true);
	} /*setscrollbarattributes*/
	
	
static boolean disposescrollbardata (hdlobject h) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	
	if ((**hdata).hcontrol != nil)
		DisposeControl ((**hdata).hcontrol);
		
	return (true);
	} /*disposescrollbardata*/
	
	
static boolean unpackscrollbardata (hdlobject h) {
	
	hdlscrollbardata hdata = (hdlscrollbardata) (**h).objectdata;
	
	(**hdata).hcontrol = nil;
	
	return (true);
	} /*unpackscrollbardata*/
	
	
void setupscrollbar (tyioaconfigrecord *);


void setupscrollbar (tyioaconfigrecord *config) {

	IOAcopystring ("\pScrollbar", (*config).objectTypeName);
	
	(*config).objectTypeID = scrollbartype;
	
	(*config).frameWhenEditing = false;
	
	(*config).canEditValue = false;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).handlesMouseTrack = true; /*allow the control manager to track the mouse*/
	
	(*config).hasSpecialCard = true; 
	
	(*config).cantDrawInBitmap = true; /*can't use offscreen bitmap to draw a control*/

	(*config).initObjectCallback = initscrollbar;
	
	(*config).drawObjectCallback = drawscrollbar;
	
	(*config).clickObjectCallback = clickscrollbar;
	
	(*config).recalcObjectCallback = recalcscrollbar;
	
	(*config).cleanupObjectCallback = cleanscrollbar;
	
	(*config).canReplicateObjectCallback = canreplicatescrollbar;
	
	(*config).getObjectEditRectCallback = getscrollbareditrect;
	
	(*config).getValueForScriptCallback = getscrollbarvalue;
	
	(*config).debugObjectCallback = debugscrollbar;	
	
	(*config).getObjectInvalRectCallback = getscrollbarinvalrect;
	
	(*config).getAttributesCallback = getscrollbarattributes;
	
	(*config).setAttributesCallback = setscrollbarattributes;
	
	(*config).unpackDataCallback = unpackscrollbardata;
	
	(*config).disposeDataCallback = disposescrollbardata;
	} /*setupscrollbar*/
	
	
