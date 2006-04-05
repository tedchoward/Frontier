
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
#pragma unused(h)

	return (true); /*it can be replicated*/
	} /*canreplicatepicture*/
	

static boolean getpictureeditrect (hdlobject h, Rect *r) {
#pragma unused(h, r)

	return (false); /*text of object can't be edited*/
	} /*getpictureeditrect*/
	

static boolean getpicturevalue (hdlobject h, Handle *hvalue) {
#pragma unused(h, hvalue)

	return (false); /*picture doesn't have a value for a script*/
	} /*getpicturevalue*/
	

static boolean debugpicture (hdlobject h, bigstring errorstring) {
#pragma unused(h)

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
	
	
void setuppicture (tyioaconfigrecord *);


void setuppicture (tyioaconfigrecord *config) {

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
	
	
