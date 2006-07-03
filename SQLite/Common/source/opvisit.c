
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

#include "op.h"
#include "opinternal.h"



boolean oplistvisit (hdlheadrecord hnode, opvisitcallback visit, ptrvoid refcon) {
	
	/*
	just visit the node and its siblings, no recursion.
	*/
	
	register hdlheadrecord nomad = hnode, nextnomad;
	
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (!(*visit) (nomad, refcon))
			return (false);
			
		if (nextnomad == nomad) 
			return (true);
			
		nomad = nextnomad;
		} /*while*/
	} /*oplistvisit*/
	
	
boolean opsummitvisit (opvisitcallback visit, ptrvoid refcon) {
	
	hdlheadrecord nomad = (**outlinedata).hsummit, nextnomad;
	
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (!(*visit) (nomad, refcon))
			return (false);
		
		if (nextnomad == nomad)
			return (true);
			
		nomad = nextnomad;
		} /*while*/
	} /*opsummitvisit*/
	

boolean opparentvisit (hdlheadrecord nomad, boolean flincludenode, opvisitcallback visit, ptrvoid refcon) {
	
	hdlheadrecord nextnomad;
	
	if (!flincludenode) {
		
		nextnomad = (**nomad).headlinkleft;
		
		if (nextnomad == nomad)
			return (true);
		 
		nomad = nextnomad;
		}
		
	while (true) {
		
		nextnomad = (**nomad).headlinkleft;
		
		if (!(*visit) (nomad, refcon))
			return (false);
		
		if (nextnomad == nomad)
			return (true);
			
		nomad = nextnomad;
		} /*while*/
	} /*opparentvisit*/
	

boolean oprecursivelyvisit (hdlheadrecord h, short lev, opvisitcallback visit, ptrvoid refcon) {
	
	register hdlheadrecord nomad, nextnomad;
	
	if (h == nil)
		return (true);
	
	nomad = (**h).headlinkright;
	
	if (nomad == h) /*nothing to the right*/
		return (true);
	
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (!(*visit) (nomad, refcon))
			return (false);
			
		if (lev > 1) {
		
			if (!oprecursivelyvisit (nomad, lev - 1, visit, refcon))
				return (false);
			}
			
		if (nextnomad == nomad) /*just processed last subhead*/
			return (true);
			
		nomad = nextnomad;
		} /*while*/
	} /*oprecursivelyvisit*/


boolean opvisiteverything (opvisitcallback visit, ptrvoid refcon) {
	
	/*
	visit every node in the outline
	*/
	
	hdlheadrecord nomad = (**outlinedata).hsummit, nextnomad;
	
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (!(*visit) (nomad, refcon)) /*visit the summit*/
			return (false);
			
		if (!oprecursivelyvisit (nomad, infinity, visit, refcon)) /*visit its subs*/
			return (false);
		
		if (nextnomad == nomad)
			return (true);
			
		nomad = nextnomad;
		} /*while*/
	
	} /*opvisiteverything*/
	
	
boolean oprecursivelyvisitkidsfirst (hdlheadrecord h, short lev, opvisitcallback visit, ptrvoid refcon) {
		
	register hdlheadrecord nomad, nextnomad;
	
	if (h == nil)
		return (true);
		
	nomad = (**h).headlinkright;
	
	if (nomad == h) /*nothing to the right*/
		return (true);
		
	while (true) {
		
		if (lev > 1)
			if (!oprecursivelyvisitkidsfirst (nomad, lev - 1, visit, refcon))
				return (false);
		
		nextnomad = (**nomad).headlinkdown; /*visit may dealloc nomad*/
		
		if (!(*visit) (nomad, refcon))
			return (false);
			
		if (nextnomad == nomad) /*just processed last subhead*/
			return (true);
			
		nomad = nextnomad;
		} /*while*/
	} /*oprecursivelyvisitkidsfirst*/
	

boolean opsiblingvisiter (hdlheadrecord hnode, boolean flkidsfirst, opvisitcallback visit, ptrvoid refcon) {

	/*
	visit the node indicated by hnode, and all of its submaterial, and all if its
	siblings and their submaterial.
	
	DW 8/27/93: added special case, if visit returns -1, we don't visit the submaterial.
	clay basket needs this to optimize its system idle routine when scanning CDs
	and other readonly media. they can't change, and they're slow to traverse.
	*/
	
	register hdlheadrecord nomad = hnode;
	register hdlheadrecord nextnomad;
	
	if (nomad == nil) /*defensive driving*/
		return (false);
	
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (flkidsfirst) {
			
			if (!oprecursivelyvisitkidsfirst (nomad, infinity, visit, refcon))
				return (false);
				
			if (!(*visit) (nomad, refcon))
				return (false);
			}
			
		else {
			short visitresult;
			
			visitresult = (*visit) (nomad, refcon);
			
			if (visitresult == 0) /*false*/
				return (false);
			
			if (visitresult != -1) {
			
				if (!oprecursivelyvisit (nomad, infinity, visit, refcon))
					return (false);
				}
			}
			
		if (nextnomad == nomad) /*have visited the last sibling*/
			return (true);
			
		if (nextnomad == nil) /*11/10/88: houtlinescrap sometimes has a nil down ptr*/
			return (true);
			
		nomad = nextnomad; /*advance to the next sibling*/
		} /*while*/
	} /*opsiblingvisiter*/


static boolean oprecursivelyvisitmarked (hdlheadrecord h, tydirection dir, opvisitcallback visit, ptrvoid refcon) {
	
	register hdlheadrecord nomad = h;
	register hdlheadrecord hnext;
	register hdlheadrecord hright;
	
	if (nomad == nil)
		return (true);
	
	if (dir == up)
		nomad = oprepeatedbump (down, longinfinity, nomad, false);
	
	while (true) {
		
		if (dir == up)
			hnext = (**nomad).headlinkup;
		else
			hnext = (**nomad).headlinkdown;
		
		if ((**nomad).flmarked)  {
			
			if (!(*visit) (nomad, refcon))
				return (false);
			}
		else {
			
			hright = (**nomad).headlinkright;
			
			if (hright != nomad) {
				
				if (!oprecursivelyvisitmarked (hright, dir, visit, refcon))
					return (false);
				}
			}
		
		if (hnext == nomad) /*just processed last subhead*/
			return (true);
		
		nomad = hnext;
		} /*while*/
	} /*oprecursivelyvisitmarked*/


boolean opvisitmarked (tydirection dir, opvisitcallback visit, ptrvoid refcon) {
	
	/*
	call the visit routine for every node which is marked in the current
	structure.
	
	8/27/92 dmb: added dir parameter. for certain structural changes, 
	must visit bottom-up
	*/
	
	hdloutlinerecord ho = outlinedata;
	
	if ((**ho).ctmarked == 0)
		return ((*visit) ((**ho).hbarcursor, refcon));
	else
		return (oprecursivelyvisitmarked ((**ho).hsummit, dir, visit, refcon));
	} /*opvisitmarked*/


boolean opbumpvisit (hdlheadrecord hstart, tydirection dir, opvisitcallback visit, ptrvoid refcon) {
	
	/*
	5.0a25 dmb: created to support true shift-clicking, there are probably 
	a number of places this could be used. opgetscreenline for sure, but now 
	isn't the time to futz with working code
	
	we visit hstart itself, then those nodes we navigate to.
	
	if performance if ever an issue, we could duplicate more of the 
	oprepeatedbump code here instead of calling it
	
	5.0b1 dmb: bump expanded only. Maybe we'll want to make that a parameter
	at some point, but not now.
	*/
	
	hdlheadrecord nomad = hstart;
	hdlheadrecord hnext;
	
	while (true) {
		
		if (!visit (nomad, refcon))
			return (false);
		
		hnext = oprepeatedbump (dir, 1, nomad, true);
		
		if (hnext == nomad)
			return (true);
		
		nomad = hnext;
		}
	} /*opbumpvisit*/




