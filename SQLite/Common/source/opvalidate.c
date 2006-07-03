
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

#include "strings.h"
#include "shell.h"
#include "op.h"
#include "opinternal.h"



#define invalid(h) {getheadstring (h,bs);shellinternalerror(idinvalidoutline, bs);return(false);}


static boolean opvalidtree (hdlheadrecord hnode, long *ptrexpansioncount) {
	
	bigstring bs;
	hdlheadrecord h, hdown, hparent;
	
	if ((**hnode).flexpanded)
		*ptrexpansioncount += opgetnodelinecount (hnode);
	
	hparent = hnode;
	
	h = (**hnode).headlinkright;
	
	if (h == hnode) 
		return (true);
	
	if ((**h).headlinkup != h) /*up pointer doesn't point at himself*/
		invalid (h);
		
	if ((**h).appbit4) /*there should be a callback here doing this*/
		invalid (h);
		
	while (true) {
		
		if ((**h).headlinkleft != hparent) /*we don't point at the right parent*/
			invalid (h);
			
		if (!opvalidtree (h, ptrexpansioncount)) /*recurse*/
			return (false);
			
		hdown = (**h).headlinkdown;
		
		if (hdown == h) /*cool, he's the last guy at the level*/
			return (true);
		
		if ((**hdown).headlinkup != h) /*guy down from us doesn't point up back at us*/
			invalid (h);
			
		h = hdown; /*advance to next guy*/
		} /*while*/
	} /*opvalidtree*/
	
	
boolean opvalidate (hdloutlinerecord houtline) {
	
	/*
	bullshit detector -- if there's anything wrong with the given outline
	structure, ie any redundant information doesn't agree, we break into the
	debugger with an appropriate message.
	*/
	
	bigstring bs;
	hdlheadrecord hsummit, h, hdown;
	hdloutlinerecord ho;
	boolean fl;
	long ctexpanded;
	
	ho = houtline;
	
	if (ho == nil)
		return (true);
	
	hsummit = (**ho).hsummit;
	
	h = hsummit;
		
	if ((**h).headlinkup != h) /*first guy doesn't point up at himself*/
		invalid (h);
		
	while (true) { /*check all level 0 guys for proper structure*/
	
		if ((**h).headlevel != 0) /*a level-0 guy who isn't level 0*/
			invalid (h);
			
		if ((**h).headlinkleft != h) /*a level-0 guy that doesn't point left at himself*/
			invalid (h);
			
		hdown = (**h).headlinkdown; /*advance to next guy*/
		
		if (hdown == h) /*reached the last guy*/
			goto L1;
		
		if ((**hdown).headlinkup != h) /*guy down from h doesn't point up at h*/
			invalid (h);
			
		h = hdown;
		} /*while*/
	
	L1: /*passed the level-0 test*/
	
	h = hsummit;
	
	ctexpanded = 0;
	
	while (true) {
		
		oppushoutline (ho);
		
		fl = opvalidtree (h, &ctexpanded);
		
		oppopoutline ();
		
		if (!fl)
			return (false);
			
		if (!opchasedown (&h))
			goto L2;
		} /*while*/
		
	L2: /*passed the level-1 test*/
	
	if (ctexpanded != (**ho).ctexpanded) {
	
		shellinternalerror(idinvalidoutline, "\x15" "expansion count error");
		
		(**ho).ctexpanded = ctexpanded; /*once is enough for this message*/

		return (false); /* 6.2b10 AR */
		}
		
	return (true);
	} /*opvalidate*/
	
	
