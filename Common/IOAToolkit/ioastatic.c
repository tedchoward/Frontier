
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

#include <ioa.h>



static boolean cleanstatic (hdlobject h, short height, short width, Rect *r) {

	hdlcard hc = (**h).owningcard;
	short gridunits = (**hc).gridunits;
	Handle hvalue;
	boolean flmultiline = false;
	long i, ct;
	
	hvalue = (**h).objectvalue;
	
	ct = GetHandleSize (hvalue);
	
	for (i = 0; i < ct; i++) {
	
		if ((*hvalue) [i] == chreturn) { /*it has a hard return in it*/
		
			flmultiline = true;
			
			break;
			}
		} /*for*/

	if (flmultiline) {
	
		short origheight = height;
		
		width = IOAmakemultiple ((*r).right - (*r).left, gridunits);
		
		height = IOAclosestmultiple ((*r).bottom - (*r).top - 4, height);
		
		if (height < origheight)
			height = origheight;
		
		(*r).right = (*r).left + width;
		
		(*r).bottom = (*r).top + height;
		}
	else {
		width = IOAmakemultiple (width, gridunits);
		
		(*r).right = (*r).left + width;
		
		(*r).bottom = (*r).top + height;
		}
	
	return (true);
	} /*cleanstatic*/
	

static boolean canreplicatestatic (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	boolean flediting = (**hc).activetextobject != nil;
	
	return (!flediting); /*only replicate if we're not in edit mode*/
	} /*canreplicatestatic*/
	

static boolean getstaticeditrect (hdlobject h, Rect *r) {
	
	*r = (**h).objectrect;
		
	return (true); /*can be edited, edit rect is the same as the object's rect*/
	} /*getstaticeditrect*/
	

static boolean getstaticvalue (hdlobject h, Handle *hvalue) {
	
	return (IOAgetstringvalue (h, hvalue));
	} /*getstaticvalue*/
	

static boolean debugstatic (hdlobject h, bigstring errorstring) {
	
	setstringlength (errorstring, 0);
	
	return (true);
	} /*debugstatic*/
	

static boolean drawstatic (hdlobject h) {
	
	hdlcard hc = (**h).owningcard;
	Handle htext;
	Rect r;
	
	htext = (**h).objectvalue;
	
	r = (**h).objectrect;
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	if (!(**hc).flskiptext)
		IOAeditdrawtexthandle (htext, r, (**h).objectjustification);
	
	return (true);
	} /*drawstatic*/
	

static boolean initstatic (tyobject *obj) {
	
	return (true); /*we do want to edit it*/
	} /*initstatic*/
	
	
static boolean recalcstatic (hdlobject h, boolean flmajorrecalc) {
	
	return (IOArecalcobjectvalue (h));
	} /*recalcstatic*/
	
	
void setupstatic (tyioaconfigrecord *);


void setupstatic (tyioaconfigrecord *config) {

	IOAcopystring ("\pText", (*config).objectTypeName);
	
	(*config).objectTypeID = statictexttype;
	
	(*config).frameWhenEditing = true;
	
	(*config).canEditValue = true;
	
	(*config).toggleFlagWhenHit = false;
	
	(*config).mutuallyExclusive = false;
	
	(*config).speaksForGroup = false;
	
	(*config).initObjectCallback = initstatic;
	
	(*config).drawObjectCallback = drawstatic;
	
	(*config).cleanupObjectCallback = cleanstatic;
	
	(*config).recalcObjectCallback = recalcstatic;
	
	(*config).canReplicateObjectCallback = canreplicatestatic;
	
	(*config).getObjectEditRectCallback = getstaticeditrect;
	
	(*config).getValueForScriptCallback = getstaticvalue;
	
	(*config).debugObjectCallback = debugstatic;
	} /*setupstatic*/
	
	
