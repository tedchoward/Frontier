
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

#include "quickdraw.h"
#include "icon.h"
#include "op.h" /*7.0b16 PBS*/
#include "opicons.h"
#include "opdisplay.h"
#include "strings.h" /*7.0b9 PBS*/
#include "tablestructure.h"

#define canexpandicon 475
#define cantexpandicon 476
#define canexpandcommenticon 477
#define cantexpandcommenticon 478
#define righticon 479
#define lefticon 480
#define downicon 481
#define upicon 482
#define breakpointicon 483
#define markedicon 484
#define	expandedicon 485
#ifdef WIN95VERSION
	#define refconicon 43 /*7.0d7 PBS: musical note icon.*/
#endif
#ifdef MACVERSION
	#define refconicon 489 /*7.0b2 PBS: musical note icon has different id in Mac version.*/
#endif
#define customicon 490


short opgetheadicon (hdlheadrecord hnode) {
	
	register hdloutlinerecord ho = outlinedata;
	boolean flcanexpand;
	
	if ((**ho).flprinting) /*show all leaders as gray when printing*/
		flcanexpand = false;
	else
		flcanexpand = ophassubheads (hnode) && (!opsubheadsexpanded (hnode));
	
	if (opnestedincomment (hnode)) {
		
		if (flcanexpand)
			return (canexpandcommenticon);
		else
			return (cantexpandcommenticon);
		}
	
	if ((**hnode).flbreakpoint)
		return (breakpointicon);
	
	#if false	//ndef onlineOutliner
	
		if ((**hnode).flmarked)
			return (markedicon);
	
	#endif
		
	if (flcanexpand)
		return (canexpandicon);
	else
		return (cantexpandicon);
	} /*opgetheadicon*/
	
	
void opdrawheadicon (short iconnum, const Rect *r, boolean flselected) {
	/*
	 This function draws the triangle icons inside outlines (but not root tables).
	 Root table triangles are in tabledisplay.c, browserdrawnodeicon()
	 */
	
	operaserect (*r);

#ifdef MACVERSION
	short transform = kTransformNone;
	
	if (flselected)
		transform = kTransformSelected; 
	
	ploticonresource ((Rect *) r, kAlignAbsoluteCenter, transform, iconnum);
#endif

#ifdef WIN95VERSION
	ploticonresource (r, 0, 0, iconnum);
#endif
	} /*opdrawheadicon*/


boolean opdrawheadiconcustom (bigstring bsiconname, const Rect *r, boolean flselected) {
	
	short transform = 0;

	if (flselected)
		transform = kTransformSelected; 
	
	operaserect (*r);

	return (ploticoncustom (r, 0, transform, bsiconname));
	} /*opdrawheadiconcustom*/

boolean opdrawheadiconcustomfromodb (bigstring bsadricon, const Rect *r, boolean flselected) {
	
	short transform = 0;
	
	if (flselected)
		transform = kTransformSelected; 
	
	operaserect (*r);
	
	return (ploticonfromodb (r, 0, transform, bsadricon));
} /*opdrawheadiconcustom*/

boolean opgetnodetypetableadr(bigstring bsnodetype, bigstring bsadrnodepath) {
	//user.tools.nodeTypes.[type]
	//Frontier.tools.data.nodeTypes.[type]
	
	bigstring bsname;
	copystring(BIGSTRING("\x15" "user.tools.nodeTypes."), bsadrnodepath);
	pushstring(bsnodetype, bsadrnodepath);
	
	hdlhashtable ht;
	hdlhashnode hn;
	tyvaluerecord iconvalue;
	boolean flexpanded = false;
	boolean fllookup = false;
	
	pushhashtable (roottable);
	
	disablelangerror ();
	
	flexpanded = langexpandtodotparams (bsadrnodepath, &ht, bsname);
	enablelangerror ();
	pophashtable ();
	
	fllookup = hashtablelookup (ht, bsnodetype, &iconvalue, &hn);
	
	if (!fllookup) {
		copystring(BIGSTRING("\x1E" "Frontier.tools.data.nodeTypes."), bsadrnodepath);
		pushstring(bsnodetype, bsadrnodepath);
		
		pushhashtable (roottable);
		
		disablelangerror ();
		
		flexpanded = langexpandtodotparams (bsadrnodepath, &ht, bsname);
		enablelangerror ();
		pophashtable ();
		
		fllookup = hashtablelookup (ht, bsnodetype, &iconvalue, &hn);
	}

	return fllookup;
}

boolean opdefaultdrawicon (hdlheadrecord hnode, const Rect *iconrect, boolean flselected, boolean flinverted) {
#pragma unused(flselected, flinverted)

	/*
	the default icon drawing routine, for the script editor and menu
	editor, not for clay basket.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	short iconnum;
	bigstring bsheadlinetype; /*7.0b9 PBS*/
	boolean flcustomicondrawn = false;
	
	iconnum = opgetheadicon (hnode);
	
//#ifdef PIKE

	/*
	7.0b9 PBS: logic for drawing a custom icon.
	If the outline is an outline,
	and the headline has a refcon,
	and the refcon has a type attribute,
	and there's a file on disk [type.bmp],
	use that icon.
	*/

	if ((**ho).outlinetype == outlineisoutline) { /*is this an outline?*/

		if (ophasrefcon (hnode)) { /*does it have a refcon?*/
		
			setemptystring (bsheadlinetype);
			
			if (opattributesgettypestring (hnode, bsheadlinetype)) { /*is there a type att?*/

				/* Look up a custom icon in the odb and draw it if there is one. */
				
				#if defined (MACVERSION)
				
				bigstring bsadrnodepath;

				if (stringlength(bsheadlinetype) > 0 && opgetnodetypetableadr(bsheadlinetype, bsadrnodepath)) {
					pushstring(BIGSTRING("\x09" ".icon.mac"), bsadrnodepath);
					flcustomicondrawn = opdrawheadiconcustomfromodb (bsadrnodepath, iconrect, false);
				}
				
				#endif
				
				#if (defined(MACVERSION) && defined(__ppc__)) || defined (WIN95VERSION)

				/*Draw a custom icon. If it returns false, there was no custom icon.*/
				if (!flcustomicondrawn) {
					flcustomicondrawn = opdrawheadiconcustom (bsheadlinetype, iconrect, false);
				}
				
				#endif
				} /*if*/
			} /*if*/
		} /*if*/

//#endif
//	opdrawheadicon (iconnum, iconrect, flselected);

	if (!flcustomicondrawn) /*Draw a normal icon only if there was no custom icon.*/

		opdrawheadicon (iconnum, iconrect, false);
	
	/*
	if (flselected) {
		
		Rect r = *iconrect;
		
		insetrect (&r, 1, 1);
		
		invertrect (r);
		}
	*/
	
	return (true);
	} /*opdefaultdrawicon*/


boolean opdefaultgeticonrect (hdlheadrecord hnode, const Rect *linerect, Rect *iconrect) {
	
	hdloutlinerecord ho = outlinedata;
	Rect r;
	Rect rcontains = *linerect;
	
	rcontains.bottom = rcontains.top + (**ho).iconheight;
	
	rcontains.left += opnodeindent (hnode);
		
	rcontains.right = rcontains.left + (**ho).iconwidth;
	
	r = rcontains;
	
	if (opisfatheadlines (ho))
		rcontains.bottom = rcontains.top + textvertinset + (**ho).defaultlineheight + textvertinset;
	else
		rcontains.bottom = (*linerect).bottom;
	
	centerrect (&r, rcontains); /*center it vertically within the linerect*/
	
	*iconrect = r;

	return (true);
	} /*opdefaultgeticonrect*/
	
	
void opgeticonrect (hdlheadrecord hnode, const Rect *linerect, Rect *iconrect) {

	(*(**outlinedata).geticonrectcallback) (hnode, linerect, iconrect);
	} /*opgeticonrect*/
	
	
void opdrawarrowicon (hdlheadrecord hnode, long lnum, tydirection arrowdirection) {
	
	/*
	display an arrow as the icon for headline. used in dragging move. 
	
	send us "nodirection" if you want it restored to its normal icon.
	*/
	
	Rect linerect, iconrect;
	short iconnum = 0;
	boolean flinverted, flselected;
	
	if (lnum < 0) /*defensive driving*/
		return;
	
	pushbackcolor (&(**outlinedata).backcolor);
	
	opgetlinerect (lnum, &linerect); 
	
	opgeticonrect (hnode, &linerect, &iconrect);
	
	opgetlineselected (hnode, &flinverted, &flselected);
	
	if (arrowdirection == nodirection) {
		
		operaserect (iconrect);
		
		(*(**outlinedata).drawiconcallback) (hnode, &iconrect, flselected, flinverted);
			
		goto exit;
		}
	
	switch (arrowdirection) {
		
		case up:
			iconnum = upicon; break;
			
		case down:
			iconnum = downicon; break;
			
		case left:
			iconnum = lefticon; break;
			
		case right:
			iconnum = righticon; break;
		
		default:
			/* do nothing*/
			break;
			
		} /*switch*/
		
	opdrawheadicon (iconnum, &iconrect, flselected);

	exit:
	
		popbackcolor ();
	
	} /*opdrawarrowicon*/
	
	
	
	
	
	

