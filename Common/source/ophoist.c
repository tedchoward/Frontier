
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

#include "op.h"
#include "opinternal.h"



typedef struct tyrestorablehoist {
	
	hdlheadrecord hhoisted; /*the head that was hoisted, whose subs are the summits*/
	
	hdlheadrecord hcursor; /*the location of the bar cursor*/
	} tyrestorablehoist;
	

tyrestorablehoist savedhoists [cthoists]; /*for saving and restoring the hoist state*/

short ctsavedhoists = 0;




void ophoistdisplay (void) {
	
	/*
	7/8/92 dmb: call opresetscrollbars after opgetscrollbarinfo to force update
	*/
	
	if (!opdisplayenabled ())
		return;
	
	opsetdisplaydefaults (outlinedata);
	
	opdirtymeasurements (); //6.0a14 dmb
	
	opsetctexpanded (outlinedata);
	
	opgetscrollbarinfo (true);
	
	opredrawscrollbars ();
	
	opinvaldisplay ();
	
	oploadeditbuffer ();
	
	/*opupdatenow ();*/ /*uncomment for debugging*/
	} /*ophoistdisplay*/
	

static boolean oppushhoistvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	assume we are only being called for the 1st level subheads of the guy
	being hoisted.  we reset his left pointer to indicate that he's a summit,
	and his level to 0.  then we reset the levels of all his subordinate
	nodes.
	*/
	
	(**hnode).headlinkleft = hnode; /*wire him back to himself*/
	
	(**hnode).flexpanded = true; /*all 0-th level items are expanded*/
	
	(**hnode).headlevel = 0;
	
	opresetlevels (hnode);
	
	return (true);
	} /*oppushhoistvisit*/
	

boolean oppushhoist (hdlheadrecord hnode) {
	
	register hdloutlinerecord ho = outlinedata;
	hdlheadrecord hsummit = (**hnode).headlinkright;
	tyhoistelement item;
	
	if ((**ho).tophoist >= cthoists)
		return (false);
		
	if (hsummit == hnode) /*has no subs*/
		return (false);
	
	opunloadeditbuffer ();
	
	item.hhoisted = hnode;
	
	item.hbarcursor = (**ho).hbarcursor;
	
	item.hsummit = (**ho).hsummit;
	
	item.lnumbarcursor = (**ho).lnumbarcursor;
	
	item.hline1 = (**ho).hline1;
	
	(**ho).hoiststack [(**ho).tophoist++] = item;
	
	oprecursivelyvisit (hnode, 1, &oppushhoistvisit, nil);
	
	(**ho).hsummit = hsummit; /*first sub is the first summit*/
	
	if (!opcontainsnode (hsummit, (**ho).hbarcursor))
		(**ho).hbarcursor = hsummit;
	
	(**ho).hline1 = hsummit;
	
	ophoistdisplay (); /*a really thorough smash of all data structures*/
	
	opdirtyoutline ();
	
	return (true);
	} /*oppushhoist*/
		
	
boolean oppophoist (void) {

	register hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad, nextnomad;
	hdlheadrecord hhoisted; 
	hdlheadrecord hfirstsummit;
	tyhoistelement item;
	
	if ((**ho).tophoist <= 0)
		return (false);
	
	opunloadeditbuffer ();
	
	item = (**ho).hoiststack [--(**ho).tophoist];
	
	hhoisted = item.hhoisted;
	
	nomad = hfirstsummit = (**ho).hsummit; /*start with the current first summit*/
	
	while (true) { /*re-link old summits to point at their old parents*/
		
		(**nomad).headlinkleft = hhoisted; /*point at his old parent*/
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad) /*reached the end of the list*/
			break;
			
		nomad = nextnomad;
		} /*while*/
	
	(**hhoisted).headlinkright = hfirstsummit; /*old 1st summit might have been X'd*/
	
	opresetlevels (hhoisted);
	
	(**ho).hbarcursor = item.hbarcursor;
	
	(**ho).hsummit = item.hsummit;
	
	(**ho).lnumbarcursor = item.lnumbarcursor;
	
	(**ho).hline1 = item.hline1;

	ophoistdisplay (); /*a really thorough smash of all data structures*/
	
	opdirtyoutline ();
	
	return (true);
	} /*oppophoist*/
	

boolean oppopallhoists (void) {
	
	/*
	restore the structure to a normal, nothing-hoisted state so that the caller can
	save the structure.  pop the hoists in such a way that they can be restored after
	the saving is finished.
	
	3/1/91 dmb: in order to support nested calls -- which needs to happen when a 
	menubar is saved, for instance, we can't use globals so save the hoist 
	information.  instead, we re-use the hoist stack, and set tophoist to the 
	negative of its original value
	*/
	
	register hdloutlinerecord ho = outlinedata;
	tyhoistelement item;
	short i;
	short cthoisted;
	
	cthoisted = (**ho).tophoist;
	
	if (cthoisted <= 0)
		return (false);
	
	for (i = cthoisted - 1; i >= 0; i--) {
		
		item.hhoisted = ((**ho).hoiststack [i]).hhoisted;
		
		item.hbarcursor = (**ho).hbarcursor;
		
		oppophoist ();
		
		(**ho).hoiststack [i] = item; /*this is now just beyond the end of the stack*/
		} /*for*/
	
	(**ho).tophoist = -cthoisted;
	
	return (true);
	} /*oppopallhoists*/
	
	
void oprestorehoists (void) {
	
	/*
	after the saving process is finished, call this routine to restore the hoist
	state of the structure.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	short i;
	short cthoisted;
	tyhoistelement item;
	
	cthoisted = -(**ho).tophoist;
	
	if (cthoisted == 0)
		return;
	
	(**ho).tophoist = 0; /*nothing is really pushed*/
	
	for (i = 0; i < cthoisted; i++) {
		
		item = (**ho).hoiststack [i]; /*grab saved hoist info from beyond end of stack*/
		
		oppushhoist (item.hhoisted);
		
		(**ho).hbarcursor = item.hbarcursor;
		} /*for*/
	
	ophoistdisplay (); /*a really thorough smash of all data structures*/
	} /*oprestorehoists*/
	
	
void opoutermostsummit (hdlheadrecord *hsummit) {
	
	/*
	punch through the illusion created by hoisting, and return to the caller
	a handle to the topmost summit in the structure.
	
	beware that the headlinkleft's of some of the nodes may not be what you'd
	expect without hoisting, but the right and down pointers are valid for
	traversals.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	
	if ((**ho).tophoist <= 0) /*nothing hoisted*/
		*hsummit = (**ho).hsummit;
	else
		*hsummit = ((**ho).hoiststack [0]).hsummit;
	} /*opoutermostsummit*/
	
	
	
	
	
	
