
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

#include "shelltypes.h"

#ifdef iowaRuntime

	#include <appletdefs.h>
	#include <appletquickdraw.h>
	#include <appletkb.h>
	#include <appletfont.h>
	#include <appletops.h>
	#include <appletmenuops.h>
	#include <appletfrontier.h>

#else

	#include <applet.h>
	#include <appletmenuops.h>
	
#endif

#ifndef iowacoreinclude

	#include "iowacore.h"

#endif

#include "iowascript.h"
#include "iowaparser.h"
#include "iowafrontier.h"


#ifndef iowaRuntime

	#include "iowaattributes.h"
	#include "iowainfowindow.h"
	#include "iowaiac.h"

#endif


boolean displaydebug = false;

hdlcard iowadata = nil; /*points to a card record, the one we're currently editing*/

typedef struct tylookup {
	
	bigstring name; /*the name we're looking for*/
	
	hdlobject h; /*the object we found, nil if not found*/
	} tylookup;
	
tylookup lookup;


tyunpackglobals unpack = {nil, nil};


typedef struct tyautoname { 
	
	char untitledstring [32];
	
	long highestnum; /*dmb 1.0b20 - was short*/
	} tyautoname;
	
static tyautoname autoname;


boolean flalertdialogrunning = false;
		
static unsigned long now;

static boolean specialflag1 = false; /*true when we're drawing objects that can't be drawn in bitmaps*/


boolean unpacklist (hdlobject *);

boolean unpackobject (hdlobject *, boolean *);

boolean unpacksingleobject (Handle, hdlobject *);






static void iowareadclock (void) {
	
	GetDateTime (&now);
	} /*iowareadclock*/
	

void editclickbottleneck (Point pt, boolean flshiftkey, hdleditrecord hbuffer) {
	
	/*
	new in 1.0b15 -- when the user cmd-2clicks on any text, we shoot it thru
	Frontier to open the object or the file.
	*/
	
	#ifndef iowaRuntime
		Handle htext;
	#endif
	
	editclick (pt, flshiftkey, hbuffer);
	
	#ifndef iowaRuntime
	
		if (!keyboardstatus.flcmdkey)
			return;
		
		if (!editgetselectedtexthandle (hbuffer, &htext)) /*error allocating handle*/
			return;
		
		if (gethandlesize (htext) == 0) { /*selection is empty, nothing to jump to*/
			
			disposehandle (htext);
			
			return;
			}
		
		apppostcallback ();
		
		FrontierJump (htext);
		
		appprecallback ();
	
	#endif
	} /*editclickbottleneck*/


boolean runcardscript (Handle hscript, long idlanguage, boolean flgetreturn, bigstring errorstring, Handle *hreturns) {

	/*
	dmb 1.0b20 -- added this wrapper to save iowadata
	*/
	
	boolean fl;
	hdlcard saveiowadata = iowadata;
	
	fl = runlangscript (hscript, (**iowadata).tablename, idlanguage, flgetreturn, errorstring, hreturns);
	
	iowadata = saveiowadata;
	
	return (fl);
	} /*runcardscript*/

	
boolean cardFastScript (bigstring bspre, Handle hmid, bigstring bspost) {
	
	/*
	a jacket for sending a fast do-script message to Frontier.
	
	the three bits of text are concatenated and sent to Frontier for execution.
	
	if an error happens, we display it and return false.
	*/
	
	Handle h;
	bigstring errorstring;
	Handle htoss;
	
	if (flalertdialogrunning)
		return (false);
	
	if (!newtexthandle (bspre, &h))
		return (false);
		
	if (!pushhandleonhandle (hmid, h))
		goto error;
		
	if (!pushtexthandle (bspost, h))
		goto error;
		
	if (!runcardscript (h, idUserTalk, true, errorstring, &htoss)) {
		
		if (stringlength (errorstring) > 0) {
		
			flalertdialogrunning = true;
		
			alertdialog (errorstring); 
		
			flalertdialogrunning = false;
			}
		
		return (false);
		}
	
	return (true);
	
	error:
	
	disposehandle (h);
	
	return (false);
	} /*cardFastScript*/
	

short makemultiple (short val, short factor) {
	
	/*
	return a multiple of factor, round up in all cases -- that is, if
	it's even one more than the nearest multiple, return the next one.
	*/
	
	if ((val % factor) == 0) /*it's already a multiple, nothing to do*/
		return (val);
		
	val = ((val / factor) + 1) * factor;
	
	if (val == 0)
		val = factor;
		
	return (val);
	} /*makemultiple*/
	
	
short closestmultiple (short val, short factor) {
	
	/*
	return a multiple of factor, round to the nearest multiple.
	*/
	
	short x, diff;
	
	if ((val % factor) == 0) /*it's already a multiple, nothing to do*/
		return (val);
	
	x = (val / factor) * factor;
	
	diff = val - x;
	
	if (diff >= (factor / 2)) /*move to the next stop up*/
		x += factor;
		
	return (x);
	} /*closestmultiple*/
	

void pushtemphandle (Handle h) {
	
	hdlcard hc = iowadata;
	short i;
	
	for (i = 0; i < cttemphandles; i++) {
	
		if ((**hc).temphandles [i] == nil) {
			
			(**hc).temphandles [i] = h;
			
			return;
			}
		} /*for*/
		
	DebugStr ("\pDW: increase size of temphandle array.");
	} /*pushtemphandle*/
	
	
void releasetemphandles (void) {
	
	hdlcard hc = iowadata;
	short i;
	
	for (i = 0; i < cttemphandles; i++) {
	
		if ((**hc).temphandles [i] != nil) {
			
			disposehandle ((**hc).temphandles [i]);
			
			(**hc).temphandles [i] = nil;
			}
		} /*for*/
	} /*releasetemphandles*/
	
	
void disposetemphandle (Handle h) {
	
	hdlcard hc = iowadata;
	short i;
	
	disposehandle (h);
	
	for (i = 0; i < cttemphandles; i++) {
	
		if ((**hc).temphandles [i] == h) {
			
			(**hc).temphandles [i] = nil;
			
			return;
			}
		} /*for*/
	} /*disposetemphandle*/
	
	
void pushobjectstyle (hdlobject h) {
	
	RGBColor forecolor;
	
	pushstyle ((**h).objectfont, (**h).objectfontsize, (**h).objectstyle);
	
	if (!(**h).objectenabled)
		forecolor = graycolor;
	else
		forecolor = (**h).objecttextcolor;
		
	pushcolors (&forecolor, &(**h).objectfillcolor);
	} /*pushobjectstyle*/
	
	
void popobjectstyle (void) {
	
	popstyle ();
	
	popcolors ();
	} /*popobjectstyle*/
	
	
static boolean autonamevisit (hdlobject h) {
	
	bigstring bs, bsuntitled;
	
	texthandletostring ((**h).objectname, bs);
	
	copystring (autoname.untitledstring, bsuntitled);
	
	if (patternmatch (bsuntitled, bs) == 1) {
		
		long x;
		
		deletestring (bs, 1, stringlength (bsuntitled));
		
		if ((bs [1] >= '0') && (bs [1] <= '9')) { /*dmb 1.0b20 - make sure it's a number*/
			
			StringToNum (bs, &x);
			
			if (x > autoname.highestnum)
				autoname.highestnum = x;
			}
		}
	
	return (true);
	} /*autonamevisit*/
	
	
boolean autonameobject (hdlobject h) {
	
	bigstring bs, bsnum;
	Handle htext;
	
	iowagetstring (untitledobjectname, bs);
	
	copystring (bs, autoname.untitledstring);
	
	autoname.highestnum = 0;
	
	visitobjects ((**iowadata).objectlist, &autonamevisit);
	
	NumToString (autoname.highestnum + 1, bsnum);
	
	pushstring (bsnum, bs);
	
	if (!newtexthandle (bs, &htext))
		return (false);
	
	setobjectname (h, htext);
	
	return (true);
	} /*autonameobject*/


static short framevertinset (void) {

	return (((globalfontinfo.ascent + globalfontinfo.descent) / 2) + textvertinset);
	} /*framevertinset*/
	
	
void toggleobjectflag (hdlobject h) {
	
	(**h).objectflag = !(**h).objectflag;
	} /*toggleobjectflag*/
	

void getlastinlist (hdlobject h, hdlobject *hlast) {
	
	hdlobject nomad = h;
	
	while (nomad != nil) {
		
		*hlast = nomad;
		
		nomad = (**nomad).nextobject;
		} /*while*/
	} /*getlastinlist*/
	
	
boolean inselection (hdlobject x) {
	
	hdlobject nomad = (**iowadata).selectionlist;
	
	while (nomad != nil) {
		
		if (nomad == x) /*it's in the list, therefore it is selected*/
			return (true);
			
		nomad = (**nomad).nextselectedobject;
		} /*while*/
		
	return (false); /*it wasn't in the list*/
	} /*inselection*/
	

short countobjects (hdlobject hlist) {

	hdlobject x = hlist;
	short ct = 0;
	
	while (x != nil) { /*count the objects in the list*/
		
		ct++;
		
		x = (**x).nextobject;
		} /*while*/
	
	return (ct);
	} /*countobjects*/
	
	
boolean getnthobject (hdlobject hlist, short n, hdlobject *h) {
	
	hdlobject x;
	short ct;
	short i;
	
	ct = countobjects (hlist) - n;
	
	if (ct < 0) /*not enough objects*/
		return (false);
		
	x = hlist;
	
	for (i = 1; i <= ct; i++) 
		x = (**x).nextobject;
		
	*h = x;
	
	return (true);
	} /*getnthobject*/
	
	
boolean findobject (Point pt, hdlobject hlist, hdlobject *h) {
	
	short i = 1;
	hdlobject nthobject;
	Rect r;
	
	while (getnthobject (hlist, i++, &nthobject)) {
		
		if (getobjectrect (nthobject, &r)) {
		
			if (PtInRect (pt, &r)) {
				
				*h = nthobject;
				
				return (true);
				}
			}
		} /*while*/
		
	/*1.0b15 -- some objects, e.g. frames, draw text outside the object rect*/
	
	i = 1;
	
	while (getnthobject (hlist, i++, &nthobject)) {
		
		if (geteditrect (nthobject, &r)) {
		
			if (PtInRect (pt, &r)) {
				
				*h = nthobject;
				
				return (true);
				}
			}
		} /*while*/
		
	return (false);
	} /*findobject*/


void getchildobjectlist (hdlobject h, hdlobject *childobjectlist) {
	
	*childobjectlist = (**h).childobjectlist;
	} /*getchildobjectlist*/
	
	
boolean findterminalobject (Point pt, hdlobject listhead, hdlobject *hparent, hdlobject *hobject) {
	
	hdlobject x;
	
	while (true) { 
		
		if (!findobject (pt, listhead, &x))
			return (false);
			
		if ((**x).objecttype != grouptype) { /*at a terminal node*/
			
			*hparent = listhead;
			
			*hobject = x;
			
			return (true);
			}
		
		getchildobjectlist (x, &listhead);
		} /*while*/
	} /*findterminalobject*/
	

boolean visitobjects (hdlobject listhead, tyobjectvisitroutine visit) {
	
	/*
	visits all terminal nodes at all levels. we don't call the visit routine
	on group nodes.
	*/
	
	hdlobject nomad = listhead;
	hdlobject nextnomad;
	
	while (nomad != nil) {
		
		nextnomad = (**nomad).nextobject;
		
		if ((**nomad).objecttype == grouptype) {
		
			if (!visitobjects ((**nomad).childobjectlist, visit))
				return (false);
			}
			
		if (!(*visit) (nomad))
			return (false);
			
		nomad = nextnomad;
		} /*while*/
	
	return (true);
	} /*visitobjects*/
	
	
boolean visittoplevelobjects (hdlobject listhead, tyobjectvisitroutine visit) {
	
	/*
	visit all nodes in the list, including groups. but don't dive into groups.
	*/
	
	hdlobject nomad = listhead;
	hdlobject nextnomad;
	
	while (nomad != nil) {
		
		nextnomad = (**nomad).nextobject;
		
		if (!(*visit) (nomad))
			return (false);
			
		nomad = nextnomad;
		} /*while*/
	
	return (true);
	} /*visittoplevelobjects*/
	
	
static boolean cleartmpbitvisit (hdlobject h) {
	
	(**h).objecttmpbit = false;
	
	return (true);
	} /*cleartmpbitvisit*/
	
	
void clearalltmpbits (void) {

	visitobjects ((**iowadata).objectlist, &cleartmpbitvisit);
	} /*clearalltmpbits*/
	

boolean derefclone (hdlobject *hsource) {
	
	/*
	if the object is a clone, return true, and set *h to the 
	thing the clone points to. loop until you find something that
	isn't a clone, allows clones to point to clones and on and on.
	*/
	
	hdlobject nomad = *hsource;
	boolean flclone = false;
	
	while ((**nomad).objecttype == clonetype) {
		
		hdlobject hdata = (hdlobject) (**nomad).objectdata;
		
		if (hdata == nil) /*encountered an unresolved clone*/
			return (false);
		
		nomad = hdata;
		
		flclone = true;
		} /*while*/
	
	*hsource = nomad;
	
	return (flclone);
	} /*derefclone*/
	

void turnoffallexclusiveobjects (hdlobject firstobject) {
	
	hdlobject x = firstobject;
	
	while (x != nil) {
		
		derefclone (&x);
		
		if (callmutallyexclusive (x)) {
			
			(**x).objectflag = false;
			
			invalobject (x);
			}
	
		x = (**x).nextobject;
		} /*while*/
	} /*turnoffallexclusiveobjects*/
	
	
void turnonfirstexclusiveobject (hdlobject firstobject) {
	
	/*
	12/29/92 DW: upgraded to turn on the upper, leftmost object
	in the list. before it was essentially a random selection as
	far as the user is concerned. a nose-hair.
	
	1/17/93 DW: make it general, it handles any exclusive type, but
	only one type per group.
	*/
	
	hdlobject x = firstobject;
	hdlobject hsmallest = nil;
	Rect rsmallest;
	
	while (x != nil) {
		
		if (!(**x).objectenabled)
			goto next;
			
		derefclone (&x);
		
		if (!(**x).objectenabled)
			goto next;
			
		if (callmutallyexclusive (x)) {
			
			Rect r = (**x).objectrect;
			
			if (hsmallest == nil) {
				
				rsmallest = r;
				
				hsmallest = x;
				}
			else {
				if (rectlessthan (r, rsmallest)) {
					
					rsmallest = r;
					
					hsmallest = x;
					}
				}
				
			(**x).objectflag = false;
			
			invalobject (x);
			}
		
		next:
		
		x = (**x).nextobject;
		} /*while*/
		
	if (hsmallest != nil) 
		(**hsmallest).objectflag = true;
	} /*turnonfirstexclusiveobject*/
	
	
boolean initexclusiveobjectsvisit (hdlobject h) {
	
	/*
	turn on the first mutually exclusive object in each group, turn 
	all others off.
	*/
	
	derefclone (&h);
	
	if ((**h).objecttype == grouptype) 
		turnonfirstexclusiveobject ((**h).childobjectlist);
		
	return (true);
	} /*initexclusiveobjectsvisit*/
	
	
boolean recalcbottleneck (hdlobject h, boolean flmajorrecalc) {
	
	if ((**h).objectrecalcscript == nil) /*save a bunch of effort, it wasn't meant to be recalcd*/
		return (true);
	
	iowareadclock ();
	
	(**h).objectnextruntime = now + (**h).objectrecalcperiod;
	
	#ifdef iowaRuntime
	
		return (callrecalcobject (h, flmajorrecalc));
	
	#else 
		if ((**iowadata).runmode)
			return (callrecalcobject (h, flmajorrecalc));
			
		#ifdef iowaRuntimeInApp
			return (callrecalcobject (h, flmajorrecalc));
		#else
			(**h).objecttmpbit = flmajorrecalc;
		
			(**h).owningcard = iowadata;
		
			return (uisRecalcObject ((Handle) h));
		#endif
	#endif
	} /*recalcbottleneck*/
	

boolean minorrecalcvisit (hdlobject h) {
	
	if (h == (**iowadata).recalcobject) /*dmb 1.0b20 - don't recalc the source of the recalc*/
		return (true);
	
	if ((**h).objectrecalcstatus >= changerecalc) {
	
		derefclone (&h);
	
		recalcbottleneck (h, false);
		}
		
	return (true); /*keep visiting*/
	} /*minorrecalcvisit*/
	

boolean majorrecalcvisit (hdlobject h) {

	switch ((**h).objecttype) {
		
		case clonetype:
			recalcclone (h);
			
			break;
			
		default:
			if ((**h).objectrecalcstatus >= initrecalc)
				recalcbottleneck (h, true);
			
			break;
		} /*switch*/
	
	return (true); /*keep visiting*/	
	} /*majorrecalcvisit*/
	

static boolean flsomethingrecalcd = false;


static boolean checkagentvisit (hdlobject h) {
	
	if ((**h).objectrecalcstatus < timerecalc)
		return (true);
	
	if (now < (**h).objectnextruntime)
		return (true);
	
	if (!flsomethingrecalcd) {
	
		frontSetRuntimeCard (true, false);
			
		flsomethingrecalcd = true;
		}
	
	recalcbottleneck (h, true); /*also sets objectnextruntime*/
	
	return (true); /*keep visiting*/	
	} /*checkagentvisit*/
	

void checkrecalc (void) {

	if ((**iowadata).recalcneeded) {
		
		if (TickCount () >= (**iowadata).recalctickcount) {
			
			frontSetRuntimeCard (true, false);
			
			visitobjects ((**iowadata).objectlist, &minorrecalcvisit);
			
			(**iowadata).recalcneeded = false;
			}
		}
	
	flsomethingrecalcd = false;
	
	iowareadclock ();
	
	visitobjects ((**iowadata).objectlist, &checkagentvisit);
	
	if (flsomethingrecalcd)
		iowaupdatenow ();
	} /*checkrecalc*/


void schedulerecalc (hdlobject h, short ctticks) {
	
	/*
	if ctticks == 0: schedule a minor recalc next time 
	thru the event loop.
	
	usually ctticks is 0, the only exception is when 
	the user is interacting and a delay could be 
	disruptive. an example, editing text in cardrunner.
	
	1.0b20 dmb: added hdlobject parameter, the object that
	caused the recalc and should not be recalced again.
	*/

	(**iowadata).recalcneeded = true;
	
	(**iowadata).recalctickcount = TickCount () + ctticks;
	
	(**iowadata).recalcobject = h;
	} /*schedulerecalc*/
	
	
void recalcobject (hdlobject h) {
	
	majorrecalcvisit (h);

	schedulerecalc (h, 0); /*do a minor recalc*/
	} /*recalcobject*/
	
	
void getgrouprect (hdlobject h, Rect *r) {
	
	hdlobject nomad;
	Rect robject, runion, rgroup;
	boolean flfirstloop = true;
	
	nomad = (**h).childobjectlist;
	
	while (nomad != nil) {
		
		getobjectrect (nomad, &robject);
		
		if (flfirstloop) {
			
			rgroup = robject;
			
			flfirstloop = false;
			}
		else {
			UnionRect (&robject, &rgroup, &runion);
		
			rgroup = runion;
			}
		
		nomad = (**nomad).nextobject;
		} /*while*/
	
	if ((**h).objecthasframe) {
		
		Rect robject;
		
		robject = (**h).objectrect;
		
		if (!EmptyRect (&robject)) { /*user has changed its rect, it takes precedence*/
			
			rgroup = robject;
			
			goto exit;
			}
			
		pushobjectstyle (h);
		
		rgroup.top -= framevertinset () * 2;
		
		rgroup.bottom += framevertinset ();
		
		rgroup.left -= 8;
		
		rgroup.right += 24;
		
		popobjectstyle ();
		}
	
	exit:
	
	*r = rgroup;
	} /*getgrouprect*/
	
	
boolean getobjectrect (hdlobject h, Rect *robject) {
	
	Rect r;
	
	r = (**h).objectrect;
	
	switch ((**h).objecttype) {
	
		case grouptype:
			getgrouprect (h, &r);
			
			break;
		} /*switch*/
		
	*robject = r;
	
	return (true);
	} /*getobjectrect*/
	
	
void getobjectnamestring (hdlobject h, bigstring bs) {
	
	/*
	object names are limited to 255 characters. 
	
	doesn't seem like too onerous a limitation. 7/6/92 DW.
	*/
	
	texthandletostring ((**h).objectname, bs);
	} /*getobjectnamestring*/
	
	
void getobjectvalue (hdlobject h, Handle *hvalue) {

	if (h == (**iowadata).activetextobject)
		editgettexthandle ((hdleditrecord) (**h).objecteditrecord, hvalue);
	else
		*hvalue = (**h).objectvalue;
	
	if (*hvalue == nil)
		newemptyhandle (hvalue);
	} /*getobjectvalue*/
	
	
void getobjectsize (hdlobject h, short *height, short *width) {
	
	/*
	12/19/92 DW: what if htext is nil? at first it seemed best to set 
	height and width to 0. but there extra smartness in editgetsize. 
	so we allocate an empty handle and pass it to editgetsize and
	release it when we're done.
	*/
	
	Handle htext;
	boolean flrelease = false;
	
	getobjectvalue (h, &htext); 
	
	if (htext == nil) {
		
		if (!newclearhandle (0, &htext)) { /*fail gracefully*/
		
			*height = *width = 0;
		
			return;
			}
		
		flrelease = true;
		}
		
	pushobjectstyle (h);

	editgetsize (htext, height, width);
	
	popobjectstyle ();
	
	if (flrelease)
		disposehandle (htext);
	} /*getobjectsize*/
	
	
void getobjectname (hdlobject h, Handle *hname) {

	*hname = (**h).objectname;
	} /*getobjectname*/
	

void getobjectscript (hdlobject h, Handle *hscript) {

	*hscript = (**h).objectscript;
	} /*getobjectscript*/
	

void getobjectrecalcscript (hdlobject h, Handle *hscript) {

	*hscript = (**h).objectrecalcscript;
	} /*getobjectrecalcscript*/
	

void setobjectvalue (hdlobject h, Handle hvalue) {
	
	/*
	DW 8/24/93: if it's the active text object, also set the text
	displayed in the edit buffer.
	
	8/12/96 dmb: just like setCardValue, we must force the object to
	update. Brent's Fortune Generator cards revealed this bug under 
	MacBird Runtime; calling setObjectAttributes to change the fortune 
	text (which ends up calling this routine) would not force an update. 
	I suspect that under Iowa, the object must be the active text object, 
	which would always be updated above.
	*/
	
	if (h == (**iowadata).activetextobject)
		editsettexthandle ((hdleditrecord) (**h).objecteditrecord, hvalue, true);
	else {
		
		(**h).objectinval = true; /*8/12/96 dmb: force update*/
		
		(**iowadata).needsupdate = true;
		}
	
	disposehandle ((**h).objectvalue);
	
	(**h).objectvalue = hvalue;
	
	schedulerecalc (h, 0);
	} /*setobjectvalue*/
	
	
void setobjectscript (hdlobject h, Handle hscript) {
	
	disposehandle ((**h).objectscript);
	
	if (hscript != nil) {
	
		if (GetHandleSize (hscript) == 0) { /*empty scripts are represented by a nil handle*/
			
			disposehandle (hscript);
			
			hscript = nil;
			}
		}
	
	(**h).objectscript = hscript;
	
	recalcobject (h);
	
	schedulerecalc (h, 0);	
	} /*setobjectscript*/
	
	
void setobjectrecalcscript (hdlobject h, Handle hscript) {
	
	disposehandle ((**h).objectrecalcscript);
	
	if (hscript != nil) {
	
		if (GetHandleSize (hscript) == 0) { /*empty scripts are represented by a nil handle*/
			
			disposehandle (hscript);
			
			hscript = nil;
			}
		}
	
	(**h).objectrecalcscript = hscript;
	
	recalcobject (h);
	
	schedulerecalc (h, 0);	
	} /*setobjectrecalcscript*/
	
	
void setobjectname (hdlobject h, Handle hname) {
	
	disposehandle ((**h).objectname);
	
	(**h).objectname = hname;
	
	schedulerecalc (h, 0);
	} /*setobjectname*/
	
	
void setobjecterrormessage (hdlobject h, Handle herrorstring) {
	
	disposehandle ((**h).objecterrormessage);
	
	(**h).objecterrormessage = herrorstring;
	
	#ifndef iowaRuntime 
		
		reseterrorinfowindow ();

	#endif
	} /*setobjecterrormessage*/
	
	
void waitfornexttick (void) {
	
	/*
	call this before drawing to avoid flicker in displaying stuff where
	flickering is a problem. should be rarely used. the theory is that 
	if the tick count just rolled over, the vertical blanking interval
	code just ran. it's fair to assume we can still get a few more things
	to happen before the screen starts refreshing again.
	*/

	unsigned long tc = TickCount ();

	while (TickCount () == tc) {} 
	} /*waitfornexttick*/
	
	
void debugrect (Rect r) {

	if (displaydebug) {
		//Code change by Timothy Paustian Monday, June 26, 2000 2:46:27 PM
		//
		#if TARGET_API_MAC_CARBON == 1
		{
		Pattern gray;
		GetQDGlobalsGray(&gray);
		FillRect(&r, &gray);
		}
		#else
		FillRect (&r, &quickdrawglobal (gray));
		#endif
		/*FrameRect (&r);*/
		
		waitfornexttick ();
		}
	} /*debugrect*/
	

boolean iowagetstring (short ix, bigstring bs) {
	
	GetIndString (bs, uistringlist, ix);
	
	return (true);
	} /*iowagetstring*/


void iowainvalrect (Rect *rinval) {
	
	Rect r = *rinval;
	//Code change by Timothy Paustian Friday, May 5, 2000 10:48:00 PM
	//Changed to Opaque call for Carbon
	if (!(IsWindowVisible((**iowadata).drawwindow))) /*dmb 1.0b24*/
		return;
	
	if (r.left < 0) /*avoid invalidating part of the tool palette*/
		r.left = 0;
	
	if (r.top < 0) /*avoid invalidating part of the status bar*/
		r.top = 0;
		
	if (!emptyrect (r)) {
	
		debugrect (r);
		
		InsetRect (&r, -1, -1); /*1.0b14 -- hide grid display glitchiness*/
	
		#if TARGET_API_MAC_CARBON == 1
		if((**iowadata).drawwindow != nil)
			InvalWindowRect((**iowadata).drawwindow, &r);
		#else
		InvalRect (&r);
		#endif
		
		}
	} /*iowainvalrect*/
	
	
void getobjectinvalrect (hdlobject h, Rect *r) {
	
	callgetinvalrect (h, r);
	
	(*r).right += (**h).objectdropshadowdepth;
	
	(*r).bottom += (**h).objectdropshadowdepth;

	switch ((**h).objecttype) {
		
		case clonetype: 
			if (derefclone (&h)) {
			
				Rect rtarget;
			
				callgetinvalrect (h, &rtarget);
			
				rtarget.right += (**h).objectdropshadowdepth;
	
				rtarget.bottom += (**h).objectdropshadowdepth;
				
				UnionRect (r, &rtarget, r);
				}
			
			break;
			
		case grouptype: {
			hdlobject nomad = (**h).childobjectlist;
		
			zerorect (r);
		
			while (nomad != nil) {
			
				Rect rnomad;
			
				getobjectinvalrect (nomad, &rnomad);
			
				UnionRect (r, &rnomad, r);
			
				nomad = (**nomad).nextobject;
				} /*while*/
			
			break;
			}
		} /*switch*/
	} /*getobjectinvalrect*/
	

void invalobject (hdlobject h) {
	
	Rect r;
	
	if (h == nil) /*defensive driving, needed for clones, 1/25/93 DW*/
		return;
	
	getobjectinvalrect (h, &r);
	
	iowainvalrect (&r);
	
	if ((**h).objecttype == clonetype)
		invalobject ((hdlobject) (**h).objectdata);
	} /*invalobject*/
	
	
void validobject (hdlobject h) {

	Rect r;
	
	if (h == nil)
		return;
	
	getobjectinvalrect (h, &r);
	//Code change by Timothy Paustian Monday, June 26, 2000 2:53:26 PM
	//
	#if TARGET_API_MAC_CARBON == 1
	if((**iowadata).drawwindow != nil)
		ValidWindowRect((**iowadata).drawwindow, &r);
	#else	
	ValidRect (&r);
	#endif
		
	(**h).objectinval = false; /*cancel any pending update*/
	} /*validobject*/
	
			
boolean unlinkobject (hdlobject x) {
	
	/*
	unlink the indicated object from the object list.
	*/

	hdlobject nomad = (**iowadata).objectlist;
	hdlobject prevnomad = nil, nextnomad;
	
	while (nomad != nil) {
		
		nextnomad = (**nomad).nextobject;
		
		if (nomad == x) {
			
			if (prevnomad == nil)
				(**iowadata).objectlist = nextnomad;
			else
				(**prevnomad).nextobject = nextnomad;
			
			return (true);
			}
		
		prevnomad = nomad;
		
		nomad = nextnomad;
		} /*while*/
	
	return (false);
	} /*unlinkobject*/


void movegroupto (hdlobject x, short h, short v) {
	
	hdlobject childobjectlist;
	hdlobject nomad;
	Rect r;
	short hdiff, vdiff;
	
	getobjectrect (x, &r);
	
	hdiff = h - r.left;
		
	vdiff = v - r.top;
	
	getchildobjectlist (x, &childobjectlist);
	
	nomad = childobjectlist;
	
	while (nomad != nil) {
		
		getobjectrect (nomad, &r);
		
		OffsetRect (&r, hdiff, vdiff);
		
		putobjectrect (nomad, r);
		
		nomad = (**nomad).nextobject;
		} /*while*/
	} /*movegroupto*/


boolean putobjectrect (hdlobject h, Rect r) {
	
	if ((**h).objecttype == grouptype) { 
		
		movegroupto (h, r.left, r.top);
		
		(**h).objectrect = r;
		
		return (true);
		}
	
	(**h).objectrect = r;
	
	return (true);
	} /*putobjectrect*/


static boolean preupdatevisit (hdlobject h) {
	
	if ((**h).objectinval) {
		
		(**h).objectinval = false;
		
		invalobject (h);
		
		debugrect ((**h).objectrect);
		}
		
	return (true);
	} /*preupdatevisit*/
	
	
boolean iowapreupdate (void) {
	
	if (iowadata != nil)
		visitobjects ((**iowadata).objectlist, &preupdatevisit);
	
	return (true);
	} /*iowapreupdate*/
	
	
void disposeobjectlist (hdlobject hlist) {
	
	hdlobject nomad = hlist;
	hdlobject nextnomad;
	
	while (nomad != nil) { 
		
		nextnomad = (**nomad).nextobject;
		
		disposeobject (nomad);
		
		nomad = nextnomad;
		} /*while*/
	} /*disposeobjectlist*/
	

void disposeobject (hdlobject h) {
	
	if (h == nil)
		return;
	
	if ((**h).objecttype == clonetype) 
		disposeobject ((hdlobject) (**h).objectdata);
	else {
		calldisposedata (h);
		
		disposehandle ((**h).objectdata);
		}
		
	disposeobjectlist ((**h).childobjectlist);
	
	disposehandle ((Handle) (**h).objectname);
	
	disposehandle ((Handle) (**h).objectvalue);
	
	disposehandle ((Handle) (**h).objectscript);
	
	disposehandle ((Handle) (**h).objecterrormessage);
	
	disposehandle ((Handle) h);
	} /*disposeobject*/
	
	
boolean iowadisposerecord (void) {

	/*
	DW 1/14/95: Card Editor must run even if Frontier
	isn't running.
	*/
	
	releasetemphandles ();
	
	if (findlangcomponent (idfrontier) != 0) /*DW 1/14/95*/
		frontDeleteCardTable (); /*call a script that deletes the table*/
	
	disposeobjectlist ((**iowadata).objectlist);
	
	disposehandle ((**iowadata).embeddedtable);

	disposehandle ((**iowadata).tablename);
	
	disposehandle ((Handle) iowadata);
		
	return (true);
	} /*iowadisposerecord*/
	
	
boolean geteditrect (hdlobject h, Rect *redit) {
	
	/*
	return false if the object can't be edited.
	*/
	
	Rect r;
	boolean fl = true;
	
	if (h == nil) /*defensive driving*/
		return (false);
	
	getobjectrect (h, &r); 
	
	switch ((**h).objecttype) {
		
		case grouptype: 
			fl = false; /*can't edit the text of a group*/
			
			break;
			
		case clonetype:
			fl = false; /*text can't be edited*/
			
			break;
			
		default:
			pushobjectstyle (h);
			
			fl = callgeteditrect (h, &r);
			
			popobjectstyle ();
			
			break;
		} /*switch*/
		
	*redit = r;
	
	return (fl);
	} /*geteditrect*/
	
	
void getobjectframe (hdlobject h, Rect *r) {
	
	getobjectrect (h, r);
	
	/*
	if ((**h).objecttype == frametype) {
		
		pushobjectstyle (h);
		
		(*r).top += framevertinset ();
		
		popobjectstyle ();
		}
	*/
	} /*getobjectframe*/
	
	
boolean iowaopenbitmap (Rect r) {
	
	boolean fl;
	
	if (displaydebug)
		return (false);
		
	if ((**iowadata).bitmapactive)
		return (false);
		
	fl = openbitmap (r, (**iowadata).drawwindow);
		
	(**iowadata).bitmapactive = fl;
	
	return (fl);
	} /*iowaopenbitmap*/
	
	
boolean iowaclosebitmap (void) {
	
	closebitmap ((**iowadata).drawwindow);
	
	(**iowadata).bitmapactive = false;
	
	return (true);
	} /*iowaclosebitmap*/
	

static void checkminmax (Point pt) {

	Rect r = (**iowadata).pictrect;
	
	if (pt.h < r.left)
		r.left = pt.h;
	
	if (pt.h > r.right)
		r.right = pt.h;
	
	if (pt.v < r.top)
		r.top = pt.v;
	
	if (pt.v > r.bottom)
		r.bottom = pt.v;
	
	(**iowadata).pictrect = r;
	} /*checkminmax*/
	
	
static void checkrectoperation (Rect r) {
	
	Point pt;
	
	pt.v = r.top;
	
	pt.h = r.left;
	
	checkminmax (pt);
	
	pt.v = r.bottom;
	
	pt.h = r.right;
	
	checkminmax (pt);
	} /*checkrectoperation*/
	
	
static boolean trackingandhot (void) {

	return ((**iowadata).tracking && (**iowadata).trackerpressed);
	} /*trackingandhot*/
	
	
static void resetclonerect (hdlobject h) {
	
	/*
	set the rectangle of the target of the clone object as a function
	of the rect of the clone object.
	*/
	
	hdlobject htarget = (hdlobject) (**h).objectdata;
	Rect r, rtarget;
	
	if ((**h).objecttype != clonetype)
		return;
		
	if (htarget == nil)
		return;
	
	getobjectrect (h, &r);
	
	getobjectrect (htarget, &rtarget);
	
	if ((**h).objectjustification == centerjustified)
		centerrect (&rtarget, r);
	else
		OffsetRect (&rtarget, r.left - rtarget.left, r.top - rtarget.top);
	
	putobjectrect (htarget, rtarget); 
	} /*resetclonerect*/
	
	
static void drawclone (hdlobject h) {
	
	hdlobject htarget = (hdlobject) (**h).objectdata;
	Rect r;
	
	getobjectrect (h, &r);
	
	if (!(**h).objecttransparent)
		EraseRect (&r);
	
	if (htarget == nil)
		return;
	
	resetclonerect (h); /*set the rectangle of htarget*/
	
	drawobject (htarget);
	} /*drawclone*/


void drawobject (hdlobject h) { 
	
	hdlcard hc = iowadata;
	Rect objectrect, rdraw, rinval;
	boolean flbitmap;
	tyobjecttype objecttype = (**h).objecttype;
	short shadowdepth = (**h).objectdropshadowdepth;
	boolean flframe;
	boolean fldrawselectionframe = true;
	
	getobjectrect (h, &objectrect); 
	
	rdraw = objectrect;
	
	callgetinvalrect (h, &rinval); /*allow for objects that draw outside their rects*/
	
	UnionRect (&rinval, &rdraw, &rdraw);
	
	rdraw.bottom += shadowdepth;
	
	rdraw.right += shadowdepth;
		
	checkrectoperation (rdraw);
	
	if ((**hc).dontdraw)
		return;
		
	if (!(**h).objectvisible) {
		
		#ifndef iowaRuntime /*4/11/93 DW*/
			
			if (inselection (h))
				drawselectionframe (h, objectrect, true);
			
		#endif
		
		return;
		}
	
	if (!SectRect (&rdraw, &(**hc).updaterect, &rdraw)) /*no intersection, save time!*/
		return;
	
	flbitmap = false;
	
	/*new fancier logic governing bitmaps -- 12/16/93 DW*/ {
		
		boolean cantbitmap = callcantdrawinbitmap (h);
		boolean bitmapactive = (**iowadata).bitmapactive;
		
		if (cantbitmap && bitmapactive)
			return;
			
		if ((!cantbitmap) && specialflag1) /*it's already been drawn in previous pass*/
			return;
			
		if ((!bitmapactive) && (!cantbitmap))
			flbitmap = iowaopenbitmap (rdraw);
		}
	
	pushobjectstyle (h);
	
	if ((**h).objecttransparent)
		pushbackcolor (&(**hc).backcolor);
	
	(**hc).flskiptext = h == (**hc).activetextobject; 
	
	switch (objecttype) {
	
		case clonetype:
			drawclone (h);
			
			break;
			
		case grouptype: {
			boolean x;
			
			if ((!(**h).objecttransparent) && (!(**h).objecthasframe))
				EraseRect (&objectrect);
	
			x = (**hc).flskiptext; /*must save and restore*/
			
			drawobjectlist ((**h).childobjectlist);
			
			(**hc).flskiptext = x;
			
			break;
			}
			
		default:
			(**h).objecttmpbit = false; /*turn it on (rare case) if you don't want a frame*/
			
			calldrawobject (h);
			
			fldrawselectionframe = !(**h).objecttmpbit;
			
			break;
		} /*switch*/
		
	flframe = (**h).objecthasframe;
	
	if ((**hc).flskiptext) {
		
		hdleditrecord hedit = (hdleditrecord) (**h).objecteditrecord;
		
		if ((**hc).bitmapactive) /*defer updating until bitmap is closed*/
			(**hc).updatethiseditbuffer = (Handle) hedit;
		else
			editupdate (hedit);
			
		if (callframewhenediting (h)) { /*it has a frame when it's being edited*/
		
			Rect editrect;
			
			geteditrect (h, &editrect);
	
			FrameRect (&editrect);
			}
			
		flframe = false; /*we've already drawn it*/
		
		(**hc).flskiptext = false; 
		}
	
	if (flframe) {
		
		pushforecolor (&(**h).objectframecolor);
		
		if (objecttype != grouptype)
			FrameRect (&objectrect);
		
		popforecolor ();
		}
	
	pushforecolor (&(**h).objectframecolor);
		
	if (shadowdepth > 0)
		dropshadowrect (objectrect, shadowdepth, false);
		
	popforecolor ();
	
	#ifndef iowaRuntime
	
		if (inselection (h))
			drawselectionframe (h, objectrect, fldrawselectionframe);
		
	#endif
		
	if ((**h).objecttransparent)
		popbackcolor ();
		
	popobjectstyle ();
	
	if (flbitmap) 
		iowaclosebitmap ();
	} /*drawobject*/


void drawobjectnow (hdlobject h) {

	Rect oldupdaterect = (**iowadata).updaterect;
	Rect r;
	
	getobjectrect (h, &r); 
	
	(**iowadata).updaterect = r;
	
	drawobject (h);
	
	(**iowadata).updaterect = oldupdaterect;
	} /*drawobjectnow*/
	
	
void drawobjectlist (hdlobject x) {
	
	while (x != nil) {
		
		drawobject (x);
		
		x = (**x).nextobject; 
		} /*while*/
	} /*drawobjectlist*/
	

#ifndef isFrontier
static boolean iscolorport (GrafPtr pport) {

	return ((*(CGrafPtr) pport).portVersion < 0);
	} /*iscolorport*/
#endif


#ifndef iowaRuntime
	
	void setnubonborder (void) {
	
		hdlcard hc = iowadata;
		boolean flbordernub;
		hdleditinfo he;
		short rb = (**hc).rightborder;
		short bb = (**hc).bottomborder;
		Rect r;
		
		if ((**hc).runmode)
			return;
		
		he = (hdleditinfo) (**hc).refcon;
		
		assert (he != nil);
		
		flbordernub = ((**hc).selectionlist == nil) && ((**hc).activetextobject == nil);
		
		if (flbordernub == (**he).flnubonborder) /*no change*/
			return;
		
		(**he).flnubonborder = flbordernub;
			
		r.right = rb;
		
		r.left = rb - nubsize;
		
		r.bottom = bb;
		
		r.top = bb - nubsize;
	
		iowainvalrect (&r); 
		} /*setnubonborder*/
		
	
	static void voidrect (Rect *rvoid) {
				
		pushbackcolor (&whitecolor);
		
		pushforecolor (&whitecolor);
	
		FillRect (rvoid, &quickdrawglobal (gray)); 
		
		popforecolor ();
	
		popbackcolor ();
		} /*voidrect*/
	

	static void drawgrid (void) {
		
		hdlcard hc = iowadata;
		boolean flcolor;
		short gridunits = (**hc).gridunits;
		short hmax, vmax;
		Rect r = (**hc).updaterect;
		short x;
		
		flcolor = iscolorport ((**hc).drawwindow);
		
		flcolor = colorenabled ();
		
		if (flcolor)
			pushgridcolor (&(**hc).backcolor);
		else {
			pushpen ();
		
			PenPat (&quickdrawglobal (gray));
			}
		
		x = (r.left / gridunits) * gridunits;	
		
		x += gridunits; /*skip the first one*/
		
		hmax = minint (r.right, (**hc).rightborder);
		
		vmax = minint (r.bottom, (**hc).bottomborder);
		
		while (x < hmax) {
			
			MoveTo (x, r.top);
			
			LineTo (x, vmax);
			
			x += gridunits;
			} /*while*/
			
		x = (r.top / gridunits) * gridunits;	
		
		x += gridunits; /*skip the first one*/
		
		while (x < vmax) {
		
			MoveTo (r.left, x);
			
			LineTo (hmax, x);
			
			x += gridunits;
			} /*while*/
		
		if (flcolor)	
			popforecolor ();
		else
			poppen ();
		} /*drawgrid*/
		
		
	static void drawborder (void) {
	
		hdlcard hc = iowadata;
		hdleditinfo he = (hdleditinfo) (**hc).refcon;
		short rb = (**hc).rightborder;
		short bb = (**hc).bottomborder;
		Rect r;
		
		pushpen ();
	
		PenPat (&quickdrawglobal (black));
		
		MoveTo (rb, 0);
		
		LineTo (rb, bb);
		
		LineTo (0, bb);
		
		poppen ();
		
		if ((**he).flnubonborder) {
		
			r.right = rb;
			
			r.left = rb - nubsize;
			
			r.bottom = bb;
			
			r.top = bb - nubsize;
		
			FillRect (&r, &quickdrawglobal (black)); 
			}
		} /*drawborder*/

#endif


static void runmodedrawbackground (Rect rdraw) {

	hdlcard hc = iowadata;

	pushbackcolor (&(**hc).backcolor);

	EraseRect (&rdraw);

	popbackcolor ();
	} /*runmodedrawbackground*/
	
	
static void drawbackground (Rect rdraw) {

	#ifdef iowaRuntime
	
		runmodedrawbackground (rdraw);
		
		return;
	
	#else /*in Card Editor*/
	
		hdlcard hc = iowadata;
		Rect rborders, rsect, rvoid;
		
		if ((**hc).runmode) {
		
			runmodedrawbackground (rdraw);
		
			return;
			}
		
		rborders.top = rborders.left = 0;
		
		rborders.bottom = (**hc).bottomborder;
		
		rborders.right = (**hc).rightborder;
		
		if (SectRect (&rdraw, &rborders, &rsect)) {
	
			pushbackcolor (&(**hc).backcolor);
		
			EraseRect (&rsect);
		
			popbackcolor ();
			}
		
		rvoid = rdraw; /*white out everything to the right of the drawing area*/
		
		rvoid.left = (**hc).rightborder;
		
		if (!EmptyRect (&rvoid)) 
			voidrect (&rvoid);
			
		rvoid = rdraw; /*white out everything below the drawing area*/
		
		rvoid.top = (**hc).bottomborder;
		
		if (!EmptyRect (&rvoid)) 
			voidrect (&rvoid);
			
	#endif
	} /*drawbackground*/
	
	
boolean iowaupdate (void) {
	
	hdlcard hc = iowadata;
	boolean flbitmap = false;
	Rect r;
	
	r = (**hc).updaterect;
	
	flbitmap = iowaopenbitmap (r);
	
	drawbackground (r);
	
	#ifndef iowaRuntime
		
		if (!(**hc).runmode) {
		
			if (!(**hc).flinvisiblegrid) /*dmb 1.0b20 -- don't require (**hc).flgrid to draw grid*/
				drawgrid ();
			
			drawborder ();
			}
	
	#endif
	
	(**hc).updatethiseditbuffer = nil;
	
	drawobjectlist ((**hc).objectlist);
	
	if (flbitmap)
		iowaclosebitmap ();
	
	/*special second pass for objects that can't be drawn in offscreen bitmaps*/ {
	
		specialflag1 = true;
	
		drawobjectlist ((**hc).objectlist);
	
		specialflag1 = false;
		}
		
	editupdate ((hdleditrecord) (**hc).updatethiseditbuffer);
	
	return (true);
	} /*iowaupdate*/
	
	
static boolean showinvisiblesvisit (hdlobject h) {
	
	derefclone (&h);
	
	if (!(**h).objectvisible) {
		
		Rect r;
		
		getobjectrect (h, &r);
		
		pushpen ();
		//Code change by Timothy Paustian Monday, June 26, 2000 2:51:24 PM
		//
		#if TARGET_API_MAC_CARBON == 1
		{
		Pattern gray;
		GetQDGlobalsGray(&gray);
		PenPat(&gray);
		}
		#else		
		PenPat (&quickdrawglobal (gray)); 
		#endif
		
		FrameRect (&r);
		
		poppen ();
		//Code change by Timothy Paustian Monday, June 26, 2000 2:52:22 PM
		//
		#if TARGET_API_MAC_CARBON == 1
		if((**iowadata).drawwindow != nil)
			InvalWindowRect((**iowadata).drawwindow, &r);
		#else
		InvalRect (&r);
		#endif
		
		}
		
	return (true);
	} /*showinvisiblesvisit*/
	

boolean showinvisiblesloop (void) {
	
	boolean firstloop = true;
	
	if (iowadata == nil) /*defensive, it can happen*/
		return (false);
	
	while (cmdkeydown () && optionkeydown ()) {
		
		if (firstloop) {
		
			visitobjects ((**iowadata).objectlist, &showinvisiblesvisit);
			
			firstloop = false;
			}
		} /*while*/
		
	return (!firstloop); /*return true if window needs updating*/
	} /*showinvisiblesloop*/


boolean setactivetextobject (hdlobject h) {

	hdlcard hc = iowadata;
	hdleditrecord hedit;
	Handle htext;
	boolean fl;
	Rect r;
	
	if (!geteditrect (h, &r))
		return (false);
	
	getobjectvalue (h, &htext); 
	
	pushobjectstyle (h);
	
	debugrect (r);
	
	fl = editnewbufferfromhandle (r, true, htext, &hedit);
	
	popobjectstyle ();
	
	if (!fl)
		return (false);
		
	editsetjustification ((**h).objectjustification, hedit);
		
	(**h).objecteditrecord = (Handle) hedit;
	
	invalobject (h);
	
	(**hc).activetextobject = h;
	
	#ifndef iowaRuntime
		setnubonborder ();
	#endif
	
	return (true);
	} /*setactivetextobject*/
	

void clearactivetextobject (void) {

	hdlcard hc = iowadata;
	hdlobject h = (**hc).activetextobject;
	hdleditrecord hedit;
	Handle hvalue;
	
	if (h == nil) /*no text object is active*/
		return;
		
	invalobject (h); /*force update*/
	
	hedit = (hdleditrecord) (**h).objecteditrecord;
	
	editgettexthandlecopy (hedit, &hvalue);
	
	editdispose (hedit);
	
	(**h).objecteditrecord = nil;
	
	/*(**h).objectautosize = false;*/ /*never 2B true again*/
	
	(**hc).activetextobject = nil;
	
	setobjectvalue (h, hvalue);
	
	invalobject (h);
	
	#ifndef iowaRuntime
		setnubonborder ();
	#endif
	} /*clearactivetextobject*/
	

hdleditrecord getactiveeditrecord (void) {
	
	/*
	factor a common piece of code. all textedit.c routines must be
	prepared to receive a nil editrecord handle.
	*/

	hdlcard hc = iowadata;
	hdlobject h;
	
	if (hc == nil)
		return (nil);
		
	h = (**hc).activetextobject;
	
	if (h == nil)
		return (nil);
		
	return ((hdleditrecord) (**h).objecteditrecord);
	} /*getactiveeditrecord*/
	
	
static boolean lookupvisit (hdlobject h) {
	
	bigstring bs;
	
	getobjectnamestring (h, bs); 
	
	allupper (bs);
	
	if (!equalstrings (bs, lookup.name)) /*continue traversal*/
		return (true);
		
	lookup.h = h;
	
	return (false); /*terminate the traversal*/
	} /*lookupvisit*/
	
	
boolean lookupname (bigstring bsname, hdlobject *h) {
	
	hdlcard hc = iowadata;

	copystring (bsname, lookup.name);
	
	allupper (lookup.name); /*lookup is unicase*/
	
	lookup.h = nil;
	
	visitobjects ((**hc).objectlist, &lookupvisit);
	
	*h = lookup.h;
	
	return (lookup.h != nil);
	} /*lookupname*/
	
	
boolean iowagetbooleanvalue (hdlobject hobj, Handle *hvalue) {

	bigstring bs;
	
	if ((**hobj).objectflag)
		copystring ("\ptrue", bs);
	else
		copystring ("\pfalse", bs);
		
	if (!newtexthandle (bs, hvalue))
		return (false);
		
	pushtemphandle (*hvalue);
	
	return (true);
	} /*iowagetbooleanvalue*/


boolean iowasetbooleanvalue (hdlobject h, Handle hvalue) {

	/*
	1.0b15 DW -- if the handle is nil, don't touch the value.
	
	this allows radio buttons with no script attached to stay properly
	initialized.
	*/
			
	bigstring bs;
	
	if (hvalue == nil)
		return (true);
	
	texthandletostring (hvalue, bs);
	
	disposehandle (hvalue);
	
	(**h).objectflag = unicaseequalstrings ("\ptrue", bs);
	
	return (true);
	} /*iowasetbooleanvalue*/


boolean iowagetstringvalue (hdlobject hobj, Handle *hvalue) {

	/*
	dmb 1.0b21 - use curly quotes instead of straight quotes. not as
	thorough as replacing all quotes with \", but very easy and fast
	*/
	
	Handle h, hval;
	
	if (!newtexthandle ("\pÒ", &h))
		return (false);
		
	pushtemphandle (h);
	
	getobjectvalue (hobj, &hval);
	
	if (!pushhandleonhandle (hval, h))
		return (false);
	
	if (!pushtexthandle ("\pÓ", h))
		return (false);
	
	*hvalue = h;
	
	return (true);
	} /*iowagetstringvalue*/
	
	
boolean iowasetpictrect (void) {

	hdlcard hc = iowadata;
	Rect r;
	
	r.left = r.top = intinfinity;
	
	r.right = r.bottom = intminusinfinity;
	
	(**hc).pictrect = r;
	
	(**hc).dontdraw = true;
	
	drawobjectlist ((**hc).objectlist);
	
	(**hc).dontdraw = false;
	
	if (equalrects ((**hc).pictrect, r)) 
		zerorect (&(**hc).pictrect);
	
	return (true);
	} /*iowasetpictrect*/
	

void sortobjectlist (void) {
	
	hdlcard hc = iowadata;
	hdlobject listhead = (**hc).objectlist;
	hdlobject nomad = listhead;
	short ct = 0, i;
	
	while (nomad != nil) {
		
		(**nomad).sorttag = 0; /*indicate that it hasn't been sorted yet*/
		
		ct++;
		
		nomad = (**nomad).nextobject;
		} /*while*/
		
	for (i = 1; i <= ct; i++) {
		
		Rect rfirst, rnomad;
		hdlobject hfirst = nil;
		
		nomad = listhead; /*start at the head of the list each time*/
		
		rfirst.top = rfirst.bottom = rfirst.left = rfirst.right = infinity;
		
		while (nomad != nil) { /*find the "smallest" rectangle*/
			
			if ((**nomad).sorttag == 0) { /*hasn't been sorted yet*/
				
				getobjectrect (nomad, &rnomad);
				
				if (rectlessthan (rnomad, rfirst)) { /*it's the new candidate*/
					
					rfirst = rnomad;
					
					hfirst = nomad;
					}
				}
				
			nomad = (**nomad).nextobject;
			} /*while*/
			
		(**hfirst).sorttag = i;
		} /*for*/
	} /*sortobjectlist*/
	
	
typedef struct tythread { 
	
	hdlobject h;
	} tythread;
	
tythread thread;


static boolean laythreadvisit (hdlobject h) {
	
	if (thread.h != nil) 
		(**thread.h).nextinthread = h;
		
	thread.h = h;
	
	return (true);
	} /*laythreadvisit*/
	
	
void laythread (void) {
	
	/*
	sometimes we can avoid hairy traversals by laying a a flat thread
	thru the grouped hierarchic structure of objects. 
	*/
	
	hdlcard hc = iowadata;
	
	thread.h = nil;
	
	visitobjects ((**hc).objectlist, &laythreadvisit);
	
	if (thread.h != nil)
		(**thread.h).nextinthread = nil; /*nil-terminated list*/	
	} /*laythread*/
	
	
void iowaupdatenow (void) {
	
	/*
	some operations need the window updated right now, they can't wait for
	the program to go back thru its main event loop.
	*/
	
	hdlcard hc = iowadata;
	
	(*(**hc).updatecallback) ();
	} /*iowaupdatenow*/
	

boolean recalcobjectvalue (hdlobject h) {
	
	/*
	a common bottleneck for all object types whose value is 
	the result of running its script.
	*/
	
	Handle hscript, hcopy, hvalue;
	bigstring errorstring;
	
	getobjectrecalcscript (h, &hscript); 
	
	if (hscript == nil) /*no linked script, nothing to do*/
		return (true);
	
	if (!preparseScript (h, hscript, &hcopy))
		return (false);
		
	disposehandle ((**h).objecterrormessage); /*DW 3/18/93 -- unconditionally dispose*/
	
	(**h).objecterrormessage = nil; /*DW 3/18/93 -- by default, no error message*/
		
	if (!runcardscript (hcopy, (**h).objectlanguage, true, errorstring, &hvalue)) {
		
		Handle htext;
		
		newtexthandle (errorstring, &htext);
		
		setobjecterrormessage (h, htext);
		
		newtexthandle ("\pERROR", &hvalue);
		}
	
	setobjectvalue (h, hvalue); /*1.0b20 dmb -- handles active edit object, invals*/
	
	/*
	disposehandle ((**h).objectvalue);
			
	(**h).objectvalue = hvalue;
	
	invalobject (h);
	*/
	
	return (true); 
	} /*recalcobjectvalue*/


static boolean evalscript (hdlobject hself, Handle hscript, Handle *hvalue, OSType idlanguage, bigstring errorstring) {
	
	/*
	for the IOA components to call back to.
	*/
	
	Handle hcopy;
	
	*hvalue = nil;
	
	if (hscript == nil) 
		return (true);
	
	if (!preparseScript (hself, hscript, &hcopy))
		return (false);
	
	return (runcardscript (hcopy, idlanguage, true, errorstring, hvalue));
	} /*evalscript*/
	
	
boolean runbuttonscript (hdlobject h) {
	
	Handle hscript, hcopy, htoss;
	bigstring errorstring;
	
	getobjectscript (h, &hscript); 
	
	if (hscript == nil) /*no script linked into object*/
		return (true);
	
	if (!preparseScript (h, hscript, &hcopy))
		return (false);
	
	frontSetRuntimeCard (true, false); /*the button script runs in the context of the embedded table*/
	
	if (!runcardscript (hcopy, (**h).objectlanguage, false, errorstring, &htoss)) {
	
		alertdialog (errorstring);
		
		return (false);
		}
	
	return (true);
	} /*runbuttonscript*/
	
	
static boolean afterclonevisit (hdlobject h) {
	
	if ((**h).objecttmpbit) {
		
		(**h).objecttmpbit = false;
		}
	
	return (true);
	} /*afterclonevisit*/
	
	
boolean recalcclone (hdlobject h) {

	tyobjecttype type = (**h).objecttype;
	Handle hcopy, hpackedobject;
	bigstring errorstring;
	OSType binarytype;
	
	if (type != clonetype)
		return (true);
		
	if (!recalcobjectvalue (h))
		return (false);
		
	getobjectvalue (h, &hcopy);
	
	if (!copyhandle (hcopy, &hcopy))
		return (false);
	
	if (!FrontierGetObject (hcopy, errorstring, &hpackedobject, &binarytype)) {
		
		alertdialog (errorstring);
		
		goto error;
		}
		
	/*unpack the object*/ {
		
		hdlobject hlinked;
		
		if (!unpacksingleobject (hpackedobject, &hlinked))
			goto error;
			
		disposeobject ((hdlobject) (**h).objectdata);
		
		(**h).objectdata = (Handle) hlinked; /*link into object*/
		
		resetclonerect (h);
		
		invalobject (h);
		
		majorrecalcvisit ((hdlobject) hlinked); /*1.0b11*/
		}
	
	return (true); 
	
	error:
	
	return (false);
	} /*recalcclone*/
	

static boolean unpackhandle (long ctbytes, Handle *h) {
	
	if (ctbytes == 0) {
		
		*h = nil;
		
		return (true);
		}
		
	if (unpack.p >= unpack.lastp) 
		return (false); 
		
	if (!newfilledhandle (unpack.p, ctbytes, h))
		return (false);
		
	unpack.p += ctbytes;
	
	return (true);
	} /*unpackhandle*/


static hdlobject hcheck;


static boolean checkobjectnamevisit (hdlobject h) {
	
	if (h != hcheck) {
	
		if (equalhandles ((**hcheck).objectname, (**h).objectname)) { /*2 objects with same name*/
			
			disposehandle ((**hcheck).objectname);
			
			(**hcheck).objectname = nil;
			
			autonameobject (hcheck);
			
			return (false); /*stop traversing*/
			}
		}
	
	return (true); /*keep visiting*/
	} /*checkobjectnamevisit*/
	

static boolean uniqueobjectname (hdlobject h) {

	if ((**h).objectname == nil)
		autonameobject (h);
	else {
		hcheck = h;
	
		visitobjects ((**iowadata).objectlist, &checkobjectnamevisit);
		}
	
	return (true);
	} /*uniqueobjectname*/
	
	
static void checkobjectnames (void) {
	
	visitobjects ((**iowadata).objectlist, &uniqueobjectname);
	} /*checkobjectnames*/
	
	
static void postunpackfilter (hdlobject h) {
	
	OffsetRect (&(**h).objectrect, unpack.hoffset, unpack.voffset);
	
	switch ((**h).objecttype) {
		
		case clonetype: /*force a recalc of the object*/
			disposehandle ((**h).objectdata);
			
			(**h).objectdata = nil;
			
			break;
			
		default:
			callunpackdata (h);
			
			break;
		} /*switch*/
	} /*postunpackfilter*/
	

boolean unpackobject (hdlobject *hobject, boolean *lastinlist) {
	
	tydiskobject info;
	tyobject obj;
	hdlobject h;
	
	moveleft (unpack.p, &info, longsizeof (info));
	
	if (info.versionnumber == 1) /*record format changed to allow 4 bytes for object type*/
		info.objecttype = (unsigned long) info.v1objecttype;
		
	if (info.versionnumber <= 2) { /*used to use a short index to store colors, now use RGB's*/
		
		oldclutconverter (info.objectrecalcstatus, &info.objectfillcolor);
		
		oldclutconverter (info.objectrecalcperiod, &info.objecttextcolor);
		
		oldclutconverter (info.unused1, &info.objectframecolor);		
		}
	
	clearbytes (&obj, longsizeof (obj));
	
	obj.objecttype = info.objecttype;
	
	obj.objectrect = info.objectrect;
	
	obj.objectflag = info.objectflag;
	
	obj.objectvisible = !info.objectinvisible;
	
	obj.objectenabled = !info.objectdisabled;
	
	obj.objecttransparent = info.objecttransparent;
	
	GetFNum ((ConstStr255Param) info.objectfont, &obj.objectfont);
	
	obj.objectfontsize = info.objectfontsize;
	
	obj.objectstyle = info.objectstyle;
	
	obj.objectjustification = (tyjustification) info.objectjustification;
	
	obj.objectfillcolor = info.objectfillcolor;
	
	obj.objecttextcolor = info.objecttextcolor;
	
	obj.objectframecolor = info.objectframecolor;
	
	obj.objecthasframe = info.objecthasframe;
	
	obj.objectdropshadowdepth = info.objectdropshadowdepth;
	
	obj.objectlinespacing = info.objectlinespacing;
	
	obj.objectindentation = info.objectindentation;
	
	obj.objectlanguage = info.objectlanguage;
	
	obj.objectrecalcstatus = info.objectrecalcstatus;
	
	if ((obj.objectrecalcstatus < neverrecalc) || (obj.objectrecalcstatus > timerecalc))
		obj.objectrecalcstatus = neverrecalc;
	
	obj.objectrecalcperiod = info.objectrecalcperiod;
	
	if (obj.objectlanguage == 0) /*converting an earlier object*/
		obj.objectlanguage = idUserTalk;
	
	obj.objecttmpbit = true;

	unpack.p += longsizeof (info);
	
	if (!unpackhandle (info.lenname, &obj.objectname))
		goto error;
	
	if (!unpackhandle (info.lenvalue, &obj.objectvalue))
		goto error;
		
	if (!unpackhandle (info.lenscript, &obj.objectscript))
		goto error;
	
	if (!unpackhandle (info.lenrecalcscript, &obj.objectrecalcscript))
		goto error;
	
	if (!unpackhandle (info.lendata, &obj.objectdata)) 
		goto error;
		
	obj.owningcard = iowadata;
	
	if (!newfilledhandle (&obj, longsizeof (tyobject), (Handle *) &h))
		goto error;

	postunpackfilter (h); /*do editing on newly unpacked object*/
	
	if (obj.objecttype == grouptype) {
		
		hdlobject firstchild;
		
		if (!unpacklist (&firstchild))
			return (false);
		
		(**h).childobjectlist = firstchild;
		}
	
	*hobject = h;
	
	*lastinlist = info.lastinlist;
	
	return (true);
		
	error:
	
	disposehandle (obj.objectname);
	
	disposehandle (obj.objectvalue);
	
	disposehandle (obj.objectscript);
	
	disposehandle (obj.objectdata);
	
	return (false);
	} /*unpackobject*/
	
	
boolean unpacklist (hdlobject *firstobject) {
	
	hdlobject lastobject = nil;
	hdlobject h;
	boolean lastinlist;
	
	while (true) {
		
		if (!unpackobject (&h, &lastinlist))
			return (false);
		
		if (lastobject == nil)
			*firstobject = h;
		else
			(**lastobject).nextobject = h;
			
		lastobject = h;
		
		if (lastinlist) /*it was the last guy in his group's list*/
			return (true);
			
		if (unpack.p >= unpack.lastp) 
			return (true);
		} /*while*/

	return (false);
	} /*unpacklist*/
	
	
boolean unpacksingleobject (Handle hpacked, hdlobject *hobject) {
	
	boolean fl, lastinlist;
	
	lockhandle (hpacked);
	
	unpack.p = *hpacked;
	
	unpack.lastp = unpack.p + GetHandleSize (hpacked);
	
	fl = unpackobject (hobject, &lastinlist);
	
	unlockhandle (hpacked);
	
	return (fl);
	} /*unpacksingleobject*/
	
	
boolean iowaunpack (Handle hpacked) {
	
	hdlcard hc = iowadata;
	hdlobject newlist;
	tydiskheader header;
	
	initIOAcallbacks (); /*set up callbacks in iowadata*/
	
	clearalltmpbits ();
	
	lockhandle (hpacked);
	
	unpack.p = *hpacked;
	
	unpack.lastp = unpack.p + GetHandleSize (hpacked);
	
	moveleft (unpack.p, &header, longsizeof (header));
	
	unpack.p += longsizeof (header);
	
	if (header.versionnumber < 4) { /*the header grew by 66 bytes in version 4*/
		
		header.lenwindowtitle = 0;
		
		unpack.p -= 66;
		}
	
	if (header.versionnumber <= 2) /*back color was stored as an index, now stored as a RGBColor*/
		oldclutconverter (header.v2backcolor, &header.backcolor);
	
	switch (header.versionnumber) {
		
		case 1:
			break;
			
		case 2: case 3: case 4: {
			Handle htable;
			
			if (header.flselection) /*we're unpacking for a paste operation*/
				break;
			
			if (!unpackhandle (header.lenembeddedtable, &htable))
				goto error;
				
			(**iowadata).embeddedtable = htable;
			
			break;
			}
			
		default:
			goto error;
		} /*switch*/
	
	/*DW 12/12/93 -- unpack the card's window title*/ {
		
		Handle hwindowtitle;
		
		if (!header.flselection) {
		
			if (!unpackhandle (header.lenwindowtitle, &hwindowtitle))
				goto error;
		
			(**iowadata).windowtitle = hwindowtitle;
			}
		}
			
	if (unpack.lastp > unpack.p) { /*a non-empty card*/
	
		if (!unpacklist (&newlist))
			goto error;
		}
	else
		newlist = nil;
		
	unlockhandle (hpacked);
	
	if ((**hc).objectlist == nil)
		(**hc).objectlist = newlist;
	else {
		hdlobject lastobject;
		
		getlastinlist ((**hc).objectlist, &lastobject);
		
		(**lastobject).nextobject = newlist;
		}
	
	laythread (); /*re-establish the flat thread thru all non-group objects*/
	
	sortobjectlist ();
	
	if (!header.flselection) {
	
		(**hc).backcolor = header.backcolor;
		
		(**hc).flgrid = header.flgrid;
		
		(**hc).flinvisiblegrid = header.flinvisiblegrid;
		
		if (header.gridunits == 0) /*be kind with old files*/
			header.gridunits = 12;
			
		(**hc).gridunits = header.gridunits;
		
		if (header.rightborder == 0) /*be kind with old files*/
			header.rightborder = 250;
			
		(**hc).rightborder = header.rightborder;
		 
		if (header.bottomborder == 0) /*be kind with old files*/
			header.bottomborder = 250;
			
		(**hc).bottomborder = header.bottomborder;
		
		(**hc).floater = header.floater;
		
		if (header.idwindow == 0) /*be kind with old files*/
			header.idwindow = 128;
			
		(**hc).idwindow = header.idwindow;
		}
	
	checkobjectnames ();
	
	return (true);
	
	error:
	
	unlockhandle (hpacked);
	
	return (false);
	} /*iowaunpack*/
	
	
boolean setCardValue (bigstring name, Handle hvalue) {
	
	hdlcard hc = iowadata;
	hdlobject h;
	
	if (!lookupname (name, &h))
		return (false);
		
	callsetvalue (h, hvalue);
		
	(**h).objectinval = true; /*force update*/
	
	(**hc).needsupdate = true; /*we watch for this in idle callback*/
	
	return (true);
	} /*setCardValue*/
	
		
boolean setCardValueCopy (bigstring name, Handle hvalue) {
	
	Handle hcopy;
	
	if (!copyhandle (hvalue, &hcopy))
		return (false);
		
	return (setCardValue (name, hcopy));
	} /*setCardValueCopy*/
	
	
boolean setCardString (bigstring name, bigstring value) {
	
	Handle hvalue;
	
	if (!newtexthandle (value, &hvalue))
		return (false);
		
	if (!setCardValue (name, hvalue)) {
		
		disposehandle (hvalue);
		
		return (false);
		}
	
	return (true);
	} /*setCardString*/
	
	
boolean getCardValue (bigstring name, Handle *hvalue) {
	
	hdlobject h;
	
	if (!lookupname (name, &h))
		return (false);
		
	getobjectvalue (h, hvalue);
	
	return (true);
	} /*getCardValue*/
	
	
boolean getCardValueCopy (bigstring name, Handle *hcopy) {
	
	Handle hvalue;
	
	if (!getCardValue (name, &hvalue))
		return (false);
		
	if (!copyhandle (hvalue, hcopy))
		return (false);
		
	releasetemphandles ();
	
	return (true);
	} /*getCardValueCopy*/
	
	
boolean setCardFlag (bigstring name, boolean fl) {
	
	hdlcard hc = iowadata;
	hdlobject h;	
	
	if (!lookupname (name, &h))
		return (false);
		
	(**h).objectflag = fl;
	
	(**h).objectinval = true; 
	
	(**hc).needsupdate = true; 
	
	return (true);
	} /*setCardFlag*/
	

boolean getCardFlag (bigstring name, boolean *fl) {

	hdlobject h;
	
	if (!lookupname (name, &h))
		return (false);
		
	*fl = (**h).objectflag;
		
	return (true);
	} /*getCardFlag*/
	
	
static boolean getnthstring (hdlobject h, short n, bigstring bs) {
	
	/*
	get the nth semicolon-delimited string from the object's 
	value text handle. n is 1-based.
	*/
	
	Handle htext = (**h).objectvalue;
	short i;
	short ctchars;
	short itemnum = 1;
	
	ctchars = GetHandleSize (htext);
	
	setstringlength (bs, 0);
		
	for (i = 0; i < ctchars; i++) {
		
		char ch = (*htext) [i];
		
		if (ch == ';') {
			
			if (itemnum == n)
				return (true);
				
			itemnum++;
			
			setstringlength (bs, 0);
			}
		else 
			pushchar (ch, bs);
		} /*for*/
	
	return ((itemnum == n) && (stringlength (bs) > 0));
	} /*getnthstring*/
	
		
void geteventwindow (EventRecord *ev, WindowPtr *eventwindow) {
	
	/*
	returns the window that the event applies to. nil if it doesn't
	apply to us.
	
	we chose this way of breaking things up to mimic the Mac dialog 
	manager, so that developers who are comfortable with it will know
	how to use this stuff.
	*/
	
	WindowPtr w;
	WindowPtr wfront;
	
	wfront = FrontWindow ();
	
	w = nil; /*default*/
	
	switch ((*ev).what) { /*some events apply to windows other than the frontmost*/
		
		case nullEvent: 
			w = wfront; 
			
			break;
			
		case keyDown: case autoKey: case keyUp:
			w = wfront;
			
			/*
			setkeyboardstatus (*ev); 
		
			if (keyboardstatus.flcmdkey) 
				if (keyboardstatus.chkb != '.') 
					w = nil;
			*/
				
			break;

		case mouseDown:	case mouseUp:
			FindWindow ((*ev).where, &w);
			
			break;
			
		case updateEvt: case activateEvt:
			w = (WindowPtr) (*ev).message;
			
			break;
			
		default: /*we don't handle other events*/
			break;
		} /*switch*/
		
	*eventwindow = w;
	} /*geteventwindow*/
	
	
#if defined (coderesource) && !defined (IOAinsideApp) && !__powerc

	/*
	2/1/93 dmb: callback routines need to have a4 set up. we could create a 
	wrapper for each routine that declares matching parameters, but the marcros 
	below automate things a bit while decreasing overhead. after setting up a4, 
	the a4callback macro calls the real callback with the actual parameters passed 
	in by the caller.  It then restores a4 and the return address before exiting.
	*/
	
	static void __GetA4etc (void)
	{
		asm {
			bsr.s	@1
			dc.l	0			;  store our A4 here
			dc.l	0			;  store ioa A4 here
			dc.l	0			;  store return here
	@1		move.l	(sp)+,a1
		}
	}
	
	#define RememberA4()	do { __GetA4etc(); asm { move.l a4,(a1) } } while (0)
	
	
	#define a4callback(x) \
		a4##x (void) { \
			asm { jsr __GetA4etc } \
			asm { move.l (sp)+,8(a1) } \
			asm { move.l a4,4(a1) } \
			asm { move.l (a1),a4 } \
			asm { jsr x } \
			asm { jsr __GetA4etc } \
			asm { move.l 4(a1),a4 } \
			asm { move.l 8(a1),-(sp) } \
			}
	
	static void a4callback (getobjectsize)
	static void a4callback (iowagetstringvalue)
	static void a4callback (pushforecolor)
	static void a4callback (popforecolor)
	static void a4callback (pushbackcolor)
	static void a4callback (popbackcolor)
	static void a4callback (runbuttonscript)
	static void a4callback (iowagetbooleanvalue)
	static void a4callback (iowasetbooleanvalue)
	static void a4callback (recalcobjectvalue)
	static void a4callback (evalscript)
	static void a4callback (invalobject)
	static void a4callback (setobjectvalue)
	static void a4callback (clearactivetextobject)
	static void a4callback (setactivetextobject)
	static void a4callback (editdrawtexthandle)
	static void a4callback (editclickbottleneck)
	static void a4callback (editselectall)
	static void a4callback (editidle)
	static void a4callback (editkeystroke)
	static void a4callback (getactiveeditrecord)
	static void a4callback (getnthstring)


void initIOAcallbacks (void) {
	
	/*
	initialize the IOA callback routines
	*/ 
	
	hdlcard hc = iowadata;
	
	RememberA4 ();
	
	(**hc).IOAgetobjectsizeCallback = (tygetobjectsizecallback) a4getobjectsize;
	
	(**hc).IOAgetstringvalueCallback = (tyobjectptrhandlecallback) a4getstringvalue;
	
	(**hc).IOApushforecolorCallback = (tyRGBcallback) a4pushforecolor;
	
	(**hc).IOApopforecolorCallback = (tyvoidcallback) a4popforecolor;
	
	(**hc).IOApushbackcolorCallback = (tyRGBcallback) a4pushbackcolor;
	
	(**hc).IOApopbackcolorCallback = (tyvoidcallback) a4popbackcolor;
	
	(**hc).IOArunbuttonscriptCallback = (tyobjectcallback) a4runbuttonscript;
	
	(**hc).IOAiowagetbooleanvalueCallback = (tyobjectptrhandlecallback) a4iowagetbooleanvalue;
	
	(**hc).IOAsetbooleanvalueCallback = (tyobjecthandlecallback) a4setbooleanvalue;
	
	(**hc).IOArecalcobjectvalueCallback = (tyobjectcallback) a4recalcobjectvalue;
	
	(**hc).IOAevalscriptCallback = (tyevalscriptcallback) a4evalscript;
	
	(**hc).IOAinvalobjectCallback = (tyobjectcallback) a4invalobject;
	
	(**hc).IOAsetobjectvalueCallback = (tyobjecthandlecallback) a4setobjectvalue;
	
	(**hc).IOAclearactivetextobjectCallback = (tyvoidcallback) a4clearactivetextobject;
	
	(**hc).IOAsetactivetextobjectCallback = (tyobjectcallback) a4setactivetextobject;
	
	(**hc).IOAeditdrawtexthandleCallback = (tydrawtexthandlecallback) a4editdrawtexthandle; 
	
	(**hc).IOAeditclickCallback = (tyeditclickcallback) a4editclickbottleneck; 
	
	(**hc).IOAeditselectallCallback = (tyhandlecallback) a4editselectall; 
	
	(**hc).IOAeditidleCallback = (tyhandlecallback) a4editidle; 
	
	(**hc).IOAeditkeystrokeCallback = (tycharhandlecallback) a4editkeystroke; 
	
	(**hc).IOAgetactiveeditrecordCallback = (tyreturnshandlecallback) a4getactiveeditrecord;
	
	(**hc).IOAgetnthstringCallback = (tygetnthstringcallback) a4getnthstring;
	} /*initIOAcallbacks*/

#else

void initIOAcallbacks (void) {
	
	/*
	initialize the IOA callback routines
	*/ 
	
	hdlcard hc = iowadata;
	
	(**hc).IOAgetobjectsizeCallback = getobjectsize;
	
	(**hc).IOAgetstringvalueCallback = iowagetstringvalue;
	
	(**hc).IOApushforecolorCallback = pushforecolor;
	
	(**hc).IOApopforecolorCallback = popforecolor;
	
	(**hc).IOApushbackcolorCallback = pushbackcolor;
	
	(**hc).IOApopbackcolorCallback = popbackcolor;
	
	(**hc).IOArunbuttonscriptCallback = runbuttonscript;
	
	(**hc).IOAgetbooleanvalueCallback = iowagetbooleanvalue;
	
	(**hc).IOAsetbooleanvalueCallback = iowasetbooleanvalue;
	
	(**hc).IOArecalcobjectvalueCallback = recalcobjectvalue;
	
	(**hc).IOAevalscriptCallback = evalscript;
	
	(**hc).IOAinvalobjectCallback = (tyobjectcallback) invalobject;
	
	(**hc).IOAsetobjectvalueCallback = (tyobjecthandlecallback) setobjectvalue;
	
	(**hc).IOAclearactivetextobjectCallback = (tyvoidcallback) clearactivetextobject;
	
	(**hc).IOAsetactivetextobjectCallback = setactivetextobject;
	
	(**hc).IOAeditdrawtexthandleCallback = editdrawtexthandle; 
	
	(**hc).IOAeditclickCallback = (tyeditclickcallback) editclickbottleneck; 
	
	(**hc).IOAeditselectallCallback = (tyhandlecallback) editselectall; 
	
	(**hc).IOAeditidleCallback = (tyhandlecallback) editidle; 
	
	(**hc).IOAeditkeystrokeCallback = (tycharhandlecallback) editkeystroke; 
	
	(**hc).IOAgetactiveeditrecordCallback = (tyreturnshandlecallback) getactiveeditrecord;
	
	(**hc).IOAgetnthstringCallback = getnthstring;
	} /*initIOAcallbacks*/

#endif



