
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

#include "memory.h"
#include "strings.h"
#include "quickdraw.h"
#include "bitmaps.h"
#include "ops.h"
#include "smallicon.h"
#include "scrap.h"
#include "op.h"
#include "opinternal.h"
#include "font.h"
#include "search.h"
#include "timedate.h"
#include "process.h"



typedef struct tyscanrecord { /*information for contains/find/search recursive scans*/
	
	hdlheadrecord hnodelookfor;
	
	hdlheadrecord hnodecurrent;
	
	long lnumlookfor;
	
	long lnumcurrent;
	
	boolean flfoundit;
	} tyscanrecord, *ptrscanrecord;




short topoutlinestack = 0;

hdloutlinerecord outlinestack [ctoutlinestack];


hdloutlinerecord opsetoutline (hdloutlinerecord houtline) {
	
	/*
	5.1.5b7 dmb: carefully manage the setting of outlinedata
	
	return the value being set for convenience
	*/
	
	hdloutlinerecord ho = outlinedata;
	
	if (houtline != ho) {
	
		outlinedata = houtline;
		
		if (houtline)
			(**houtline).ctpushes++;
		
		if (ho) {
			
			if ((--(**ho).ctpushes == 0) && (**ho).fldisposewhenpopped)
				opdisposeoutline (ho, false);
			}
		}
	
	return (houtline);
	} /*opsetoutline*/
		

boolean oppushoutline (hdloutlinerecord houtline) {
	
	/*
	when you want to temporarily work with a different outlinerecord, call this
	routine, do your stuff and then call oppopoutline.
	*/
	
	if (topoutlinestack >= ctoutlinestack) {
		
		shellinternalerror (idoutlinestackfull, STR_outline_stack_overflow);
		
		return (false);
		}
	
	outlinestack [topoutlinestack++] = outlinedata;
	
	outlinedata = houtline;
	
	if (houtline)
		(**houtline).ctpushes++;
	
	return (true);
	} /*oppushoutline*/
		

boolean oppopoutline (void) {
	
	hdloutlinerecord ho = outlinedata;
	
	if (topoutlinestack <= 0)
		return (false);
	
	outlinedata = outlinestack [--topoutlinestack];
	
	if (ho) {
		
		if ((--(**ho).ctpushes == 0) && (**ho).fldisposewhenpopped)
			opdisposeoutline (ho, false);
		}
	
	return (true);
	} /*oppopoutline*/


boolean oppushglobals (void) {
	
	/*
	5.1.5b11 dmb: we need this layer for globals nesting
	*/

	if (outlinedata)
		++(**outlinedata).ctpushes;
	
	return (true);
	} /*oppushglobals*/


boolean oppopglobals (void) {
	
	hdloutlinerecord ho = outlinedata;
	
	if (ho && (**ho).ctpushes > 1) // has been set and pushed
		--(**ho).ctpushes;
	
	return (true);
	} /*oppopglobals*/


boolean opfirstinlist (hdlheadrecord hnode) {
	
	return ((**hnode).headlinkup == hnode);
	} /*opfirstinlist*/
	
	
boolean oplastinlist (hdlheadrecord hnode) {
	
	return ((**hnode).headlinkdown == hnode);
	} /*oplastinlist*/
	

boolean opnosubheads (hdlheadrecord hnode) {
	
	/*
	note that we don't acount dynamic subs. we answer the 
	question: does it have subheads now?
	*/
	
	return ((**hnode).headlinkright == hnode);
	} /*opnosubheads*/
	

boolean ophassubheads (hdlheadrecord hnode) {
	
	/*
	unlike opnosubheads, we count dynamic subs has being there, 
	event if they're not physically present now.
	*/
	
	if ((**hnode).headlinkright != hnode)
		return (true);
	
	return ((*(**outlinedata).hasdynamicsubscallback) (hnode));
	} /*ophassubheads*/
	

boolean opislastsubhead (hdlheadrecord hnode) {
	
	return ((**hnode).headlinkdown == hnode);
	} /*opislastsubhead*/
	
	
boolean opsubheadsexpanded (hdlheadrecord hnode) {
	
	register hdlheadrecord hkid = (**hnode).headlinkright;
	
	if (hkid == hnode) /*no subheads*/
		return (false);
		
	return ((**hkid).flexpanded);
	} /*opsubheadsexpanded*/
	
	
boolean opsubordinateto (hdlheadrecord h1, hdlheadrecord h2) {
	
	/*
	return true if h1 is part of h2's suboutline.
	
	if h2 is in the path back to a summit from h1.
	*/
	
	register hdlheadrecord x = h1, y = h2;
	
	if (x == y) /*a node is not subordinate to itself*/
		return (false);
	
	while (true) {
		
		if ((**x).headlevel == 0) /*at a summit, h1 is not subordinate to h2*/
			return (false);
		
		x = (**x).headlinkleft; /*move towards a summit*/
		
		if (x == y) /*x is subordinate to y*/
			return (true);
		} /*while*/
	} /*opsubordinateto*/
	
	
boolean opnestedincomment (hdlheadrecord hnode) {
	
	/*
	return true if the node is subordinate to a comment line, or if it is
	a comment line itself.
	*/
	
	register hdlheadrecord x = hnode;
	
	while (true) {
		
		if ((**x).flcomment) /*it is nested inside a comment*/
			return (true);
			
		if ((**x).headlevel <= 0) /*can't surface any further*/
			return (false);
			
		x = (**x).headlinkleft;
		} /*while*/
	} /*opnestedincomment*/
	
      
hdlheadrecord opgetancestor (hdlheadrecord hnode, short level) {
	
	/*
	surface out from hnode until you reach a node at the indicated level.
	
	interesting function -- it's used in implementing dragging move.
	*/
	
	register hdlheadrecord nomad = hnode;
	register short headlevel;
	
	while (true) {
		
		headlevel = (**nomad).headlevel;
		
		if (headlevel <= level) /*surfaced out far enough*/
			return (nomad);
			
		if (headlevel <= 0) /*internal error*/
			return (nil);
			
		nomad = (**nomad).headlinkleft;
		} /*while*/
	} /*opgetancestor*/
	
	
hdlheadrecord opgetlastsubhead (hdlheadrecord hnode) {
	
	/*
	return a handle to the last line subordinate to the node.
	*/
	
	register hdlheadrecord nomad = (**hnode).headlinkright;
	register hdlheadrecord nextnomad;
	
	if (nomad == hnode)
		return (hnode);
		
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad) /*reached the last subhead*/
			return (nomad);
			
		nomad = nextnomad; /*advance to next node*/
		} /*while*/
	} /*opgetlastsubhead*/
	
      
hdlheadrecord opnthsubhead (hdlheadrecord hnode, long n) {
	
	/*
	return the nth subhead of the indicated node, nil if he doesn't have
	that many subheads.
	
	the number is 1-based, so the first sub is returned when n == 1.
	*/
	
	register hdlheadrecord h = hnode;
	register hdlheadrecord nomad = (**h).headlinkright;
	register hdlheadrecord nextnomad;
	register long i;
	
	if (n <= 0) /*special case*/
		return (h);
	
	if (nomad == h) /*no subs*/
		return (nil);
	
	for (i = 1; i < n; i++) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad) /*aren't that many subs*/
			return (nil);
			
		nomad = nextnomad; /*advance to next sub*/
		} /*for*/
		
	return (nomad);
	} /*opnthsubhead*/
	
	
boolean opnthsummit (long n, hdlheadrecord *hsummit) {
	
	/*
	return the nth summit of the current structure in *hsummit.
	
	return true if there are that many summits, false otherwise.
	*/
	
	register hdlheadrecord nomad = (**outlinedata).hsummit;
	register hdlheadrecord nextnomad;
	register long ctloops = n - 1;
	register long i;
	
	for (i = 1; i <= ctloops; i++) {
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad) { /*reached the end of the list, less than n summits*/
			
			*hsummit = nomad;
			
			return (false);
			}
			
		nomad = nextnomad;
		} /*for*/
		
	*hsummit = nomad;
	
	return (true);
	} /*opnthsummit*/


static boolean opgetnthnodevisit (hdlheadrecord hnode, ptrvoid refcon) {

	ptrscanrecord scanrecord = (ptrscanrecord) refcon;
	
	if ((*scanrecord).lnumcurrent == (*scanrecord).lnumlookfor) { /*we're at the node*/
		
		(*scanrecord).hnodecurrent = hnode;
		
		return (false); /*terminate the traversal*/
		}
	
	(*scanrecord).lnumcurrent++; 
	
	return (true); /*continue the traversal*/
	} /*opgetnthnodevisit*/


boolean opgetnthnode (long lnum, hdlheadrecord *hnode) {
	
	/*
	traverse the outline, counting nodes until we hit the line
	numbered "lnum".  return that node.
	
	if there aren't that many headlines, return false
	*/
	
	tyscanrecord scanrecord;
	
	scanrecord.lnumcurrent = 1; /*counts the lines, start with line 1*/
	
	scanrecord.lnumlookfor = lnum; /*this is the line we're looking for*/
	
	if (opsiblingvisiter ((**outlinedata).hsummit, false, &opgetnthnodevisit, &scanrecord)) /*not enough lines*/
		return (false);
	
	*hnode = scanrecord.hnodecurrent;
	
	return (true);
	} /*opgetnthnode*/
	

boolean oparrayreference (long ix, hdlheadrecord *hnode) {
	
	/*
	view the outline as an array.  return the string linked into the ixth summit.
	*/
	
	return (opnthsummit (ix, hnode));
	} /*oparrayreference*/
	
      
hdlheadrecord oplastexpanded (hdlheadrecord hnode) {
	
	/*
	return the last expanded subhead under hnode.  if it has no subheads
	or it is not expanded, we return hnode itself.
	*/
	
	register hdlheadrecord h = hnode;
	
	while (true) {
	
		if (!opsubheadsexpanded (h))
			return (h);
			
		h = opgetlastsubhead (h);
		} /*while*/
	} /*oplastexpanded*/
	

hdlheadrecord oplastexpandedatlevel (hdlheadrecord hnode, short level) {
	
	/*
	return the last expanded subhead under hnode.  if it has no subheads
	or it is not expanded, we return hnode itself.
	
	repeat until you hit a node whose headlevel == level.
	
	supports the dragging move operation.
	*/
	
	register hdlheadrecord h = hnode;
	
	while (true) {
	
		if (!opsubheadsexpanded (h))
			return (h);
			
		if ((**h).headlevel == level)
			return (h);
			
		h = opgetlastsubhead (h);
		} /*while*/
	} /*oplastexpandedatlevel*/
	

boolean opchaseup (hdlheadrecord *hnode) {
	
	register hdlheadrecord h = *hnode;
	register hdlheadrecord hup;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	hup = (**h).headlinkup;
	
	if (hup == h)
		return (false);
		
	*hnode = hup;
	
	return (true);
	} /*opchaseup*/
	
	
boolean opchasedown (hdlheadrecord *hnode) {
	
	register hdlheadrecord h = *hnode;
	register hdlheadrecord hdown;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	hdown = (**h).headlinkdown;
	
	if (hdown == h)
		return (false);
		
	*hnode = hdown;
	
	return (true);
	} /*opchasedown*/
	
	
boolean opchaseleft (hdlheadrecord *hnode) {
	
	register hdlheadrecord h = *hnode;
	register hdlheadrecord hleft;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	hleft = (**h).headlinkleft;
	
	if (hleft == h)
		return (false);
		
	*hnode = hleft;
	
	return (true);
	} /*opchaseleft*/
	
	
boolean opchaseright (hdlheadrecord *hnode) {
	
	register hdlheadrecord h = *hnode;
	register hdlheadrecord hright;
	
	if (h == nil) /*defensive driving*/
		return (false);
		
	hright = (**h).headlinkright;
	
	if (hright == h)
		return (false);
		
	*hnode = hright;
	
	return (true);
	} /*opchaseright*/
	
	
void opsummitof (hdlheadrecord hnode, hdlheadrecord *hsummit) {
	
	*hsummit = hnode;
	
	while (opchaseleft (hsummit)) {}
	} /*opsummitof*/
	
	
void opgetsiblingnumber (hdlheadrecord hnode, long *siblingnumber) {
	
	/*
	if hnode is the third head at its level, return 2.  in other words, it's a 
	zero-based index into the head's level.
	*/
	
	register long ct = 0;
	
	while (opchaseup (&hnode))
		ct++;
		
	*siblingnumber = ct;
	} /*opgetsiblingnumber*/
	
	
hdlheadrecord opgetnextexpanded (hdlheadrecord hnode) {
	
	/*
	DW 7/13/93: lots of routines want to bump flatdown, only looking at
	expanded nodes. this saves a little bit of space and time when 
	you have to iterate this way.
	*/
	
	hdlheadrecord nomad = hnode;
	hdlheadrecord hcheck;
	
	hcheck = (**nomad).headlinkright;
	
	if (hcheck != nomad) { 
		
		if ((**hcheck).flexpanded)
			return (hcheck);
		}
		
	hcheck = (**nomad).headlinkdown; 
	
	if (hcheck != nomad) { 
		
		if ((**hcheck).flexpanded)
			return (hcheck);
		}
	
	while (true) {
		
		if ((**nomad).headlinkleft == nomad) /*at a summit, return the original node*/
			return (hnode); 
		
		nomad = (**nomad).headlinkleft; /*move out to parent*/
		
		hcheck = (**nomad).headlinkdown; /*check next sibling*/
		
		if (hcheck != nomad) { 
			
			if ((**hcheck).flexpanded)
				return (hcheck);
			}
		} /*while*/
	} /*opgetnextexpanded*/
	
	
hdlheadrecord opbumpflatdown (hdlheadrecord hnode, boolean flexpanded) {
	
	/*
	return the node that is flatdown from the indicated node.  if the boolean
	is true, then only expanded nodes are visited.
	
	if hnode points to the last node, return the node itself.
	*/
	
	hdlheadrecord nomad = hnode;
	hdlheadrecord hcheck;
	
	if (flexpanded)
		return (opgetnextexpanded (hnode));
		
	hcheck = (**nomad).headlinkright;
	
	if (hcheck != nomad) 
		return (hcheck);
		
	hcheck = (**nomad).headlinkdown; /*check next sibling*/
	
	if (hcheck != nomad) 
		return (hcheck);
	
	while (true) {
		
		if ((**nomad).headlinkleft == nomad) /*at a summit*/
			return (hnode); 
		
		nomad = (**nomad).headlinkleft; /*move out to parent*/
		
		hcheck = (**nomad).headlinkdown; /*check next sibling*/
		
		if (hcheck != nomad) 
			return (hcheck);
		} /*while*/
	} /*opbumpflatdown*/
	
	
hdlheadrecord opbumpflatup (hdlheadrecord hnode, boolean flexpanded) {
	
	/*
	12/28/91 dmb: handle flexpanded flag true when original node is collapsed
	
	5.1.5b9 dmb: recoded to not use opbumpflatdown; avoids infinite looping 
	in mis-expanded outlines
	*/
	
	hdlheadrecord origh = hnode;
	hdlheadrecord lasth;
	
	if ((**hnode).headlinkup == hnode) { /*no way up*/
		
		if ((**hnode).headlinkleft != hnode) /*not at summit*/
			return ((**hnode).headlinkleft);
			
		return (origh); /*hnode is on the summit -- no movement*/
		}
	
	hnode = (**hnode).headlinkup; /*go to previous sibling*/
	
	if (flexpanded && !(**hnode).flexpanded) { /*special case*/
		
		if (hnode != origh) /*we moved, though still collapsed*/
			return (hnode);
		
		return ((**hnode).headlinkleft); /*bump out a level -- hopefully expanded*/
		}
	
	if (flexpanded)
		return (oplastexpanded (hnode));
	
	while (true) {
		
		lasth = hnode;
		
		hnode = opgetlastsubhead (hnode);
		
		if (hnode == lasth)
			return (hnode);
		} /*while*/
	} /*opbumpflatup*/


hdlheadrecord oprepeatedbump (tydirection dir, long ctbumps, hdlheadrecord hstart, boolean flexpanded) {
	
	/*
	navigate from hstart in the indicated tydirection, ctbumps times.
	
	if flexpanded is true only visit nodes that are expanded.
	
	return the resulting node.
	
	this was designed early on, and doesn't return a boolean indicating whether
	it was able to move in the desired tydirection.
	*/
	
	register hdlheadrecord nomad = hstart;
	register long ct = ctbumps;
	register boolean fl = flexpanded;
	register hdlheadrecord lastnomad;
	register long i;
	
	if (nomad == nil) /*defensive driving*/
		return (nil);
	
	if (ct == 0)
		goto L1;
	
	if (ct < 0) {
		
		ct = -ct;
		
		dir = oppositdirection (dir);
		}
	
	switch (dir) {
		
		case flatup:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = opbumpflatup (nomad, fl);
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case flatdown:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = opbumpflatdown (nomad, fl);
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case left:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = (**nomad).headlinkleft;
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case right:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = (**nomad).headlinkright;
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case up:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = (**nomad).headlinkup;
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
			
		case down:
			for (i = 1; i <= ct; i++) { /*replicated code for speed*/
				
				lastnomad = nomad;
				
				nomad = (**nomad).headlinkdown;
				
				if (nomad == lastnomad)
					goto L1;
				} /*for*/	
				
			break;
		
		default:
			/* do nothing */
			break;
			
		} /*switch*/
	
	L1:
			
	return (nomad);
	} /*oprepeatedbump*/


boolean opnavigate (tydirection dir, hdlheadrecord *hnode) {
	
	/*
	yet another way to move through an outline structure.
	
	return true if you can navigate in the indicated tydirection.  return with
	*hnode pointing to the resulting node.
	*/
	
	register hdlheadrecord h = *hnode;
	register hdlheadrecord hnext;
	
	hnext = oprepeatedbump (dir, 1, h, false);
	
	if (hnext == h) /*didn't move anywhere*/
		return (false);
		
	*hnode = hnext;
	
	return (true);
	} /*opnavigate*/
	

boolean opfindhead (hdlheadrecord hfirst, bigstring bs, hdlheadrecord *hnode) {
	
	/*
	search starting with hfirst and step through its siblings looking
	for a headline that exactly matches bs.
	*/
	
	register hdlheadrecord nomad, nextnomad;
	bigstring bsnomad;
	bigstring bslook;
	
	*hnode = nil; /*default returned value*/
	
	nomad = hfirst;
	
	copystring (bs, bslook); /*work on a copy*/
	
	alllower (bslook); /*unicase*/
	
	while (true) {
		
		opgetheadstring (nomad, bsnomad);
		
		alllower (bsnomad);
		
		if (equalstrings (bslook, bsnomad)) {
			
			*hnode = nomad;
			
			return (true);
			}
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad)
			return (false);
			
		nomad = nextnomad;
		} /*while*/
	} /*opfindhead*/
	

boolean opbestfind (hdlheadrecord hfirst, bigstring bs, hdlheadrecord *hnode) {
	
	/*
	find the best approximation of the string in the siblings of hfirst.
	*/
	
	register hdlheadrecord nomad, nextnomad;
	bigstring bsnomad;
	bigstring bsfind;
	bigstring bsbest;
	register hdlheadrecord hbest = nil; /*no candidate found*/
	
	*hnode = nil; /*default returned value*/
	
	nomad = op1stsibling (hfirst);
	
	copystring (bs, bsfind); /*work on a copy*/
	
	alllower (bsfind); /*unicase*/
	
	while (true) {
		
		opgetheadstring (nomad, bsnomad);
		
		alllower (bsnomad);
		
		switch (comparestrings (bsnomad, bsfind)) {
			
			case 0: /*string equal*/
				*hnode = nomad;
				
				return (true);
			
			case -1: /*key less than name*/
				break;
			
			case +1: /*key greather than name*/
				if ((hbest == nil) || (comparestrings (bsnomad, bsbest) == -1)) {
					
					copystring (bsnomad, bsbest);
					
					hbest = nomad;
					}
			} /*switch*/
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad)
			break;
			
		nomad = nextnomad;
		} /*while*/
	
	if (hbest == nil) /*didn't find any item that would follow bsname*/
		hbest = nomad; /*select last name*/
	
	*hnode = hbest;
	
	return (false);
	} /*opbestfind*/


long opcountatlevel (hdlheadrecord hnode) {
	
	/*
	return the number of nodes down from the indicated node plus 1.
	
	xxx the name is kind of a misnomer, it returns the count at a level if you pass
	xxx in a handle to the first guy at the level.
	
	8/28/92 dmb: recoded to handle arbitrary node in list. slighly less optimal, 
	but simple.
	*/
	
	register long ct = 0;
	hdlheadrecord nomad = hnode;
	
	if (nomad != nil) { /*defensive driving*/
		
		ct = 1;
		
		while (opchaseup (&nomad))
			ct++;
		
		nomad = hnode;
		
		while (opchasedown (&nomad))
			ct++;
		}
	
	return (ct);
	} /*opcountatlevel*/
	
	
hdlheadrecord op1stsibling (hdlheadrecord hnode) {
	
	while (opchaseup (&hnode)) {}
	
	return (hnode);
	} /*op1stsibling*/


long opcountsubs (hdlheadrecord hnode) {
	
	/*
	returns the number of first level subs underneath the indicated headrecord.
	*/
	
	register hdlheadrecord hright = (**hnode).headlinkright;
	
	if (hright == hnode) /*no subs*/
		return (0);
		
	return (opcountatlevel (hright));
	} /*opcountsubs*/


long opsetctexpanded (hdloutlinerecord ho) {
	
	/*
	when you load an outline, or do something too complex to maintain
	arithmetically, call this routine to brute force compute the number
	of expanded headlines.
	*/
	
	hdlheadrecord nomad = (**ho).hsummit, nextnomad;
	long ct = 0;
	
	oppushoutline (ho);
	
	while (true) {
		
		ct += opgetnodelinecount (nomad);
		
		nextnomad = opgetnextexpanded (nomad);
		
		if (nextnomad == nomad) { /*could go no further*/
			
			(**ho).ctexpanded = ct;
			
			break;
			}
			
		nomad = nextnomad;
		} /*while*/
	
	oppopoutline ();
	
	return (ct);
	} /*opsetctexpanded*/
	
	
void opsetexpandedbits (hdlheadrecord hsummit, boolean fl) {
	
	/*
	set the expanded bits of the node and all its siblings as indicated
	by the boolean.
	*/

	register boolean bit = bitboolean (fl);
	register hdlheadrecord nomad = hsummit;
	register hdlheadrecord lastnomad;
	
	if (nomad == nil) /*defensive driving*/
		return;
	
	while (true) {
		
		(**nomad).flexpanded = bit;
		
		lastnomad = nomad;
		
		nomad = (**nomad).headlinkdown;
		
		if (nomad == lastnomad) 
			return;
		} /*while*/
	} /*opsetexpandedbits*/


static boolean opcontainsvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	ptrscanrecord scanrecord = (ptrscanrecord) refcon;
	
	if (hnode == (*scanrecord).hnodelookfor) {
		
		(*scanrecord).flfoundit = true;
		
		return (false); /*halt the search*/
		}
		
	return (true); /*continue searching*/
	} /*opcontainsvisit*/
	
	
boolean opcontainsnode (hdlheadrecord hlookunder, hdlheadrecord hlookfor) {
	
	/*
	traverse the structure indicated by hlookunder, searching for hlookfor.
	
	if it the node is found, return true, otherwise false.
	
	5.0.2b20 dmb: we must use opsiblingvisiter, not oprecursivelyvisit, and 
	search under all of hlookunder's level
	*/
	
	tyscanrecord scanrecord;
	
	scanrecord.hnodelookfor = hlookfor;
	
	scanrecord.flfoundit = false;
	
	opsiblingvisiter (hlookunder, false, &opcontainsvisit, &scanrecord);
	
	return (scanrecord.flfoundit);
	} /*opcontainsnode*/
	
	
void opgetnodeline (hdlheadrecord hnode, long *lnum) {
	
	/*
	get the node line number for the indicated node.  the first summit is
	node 0.  its first sub, if expanded, is node 1.
	
	we compute the number by bumping our way flatup until we can bump no more,
	counting nodes as we go.
	*/
	
	register hdlheadrecord nomad = hnode;
	register hdlheadrecord lastnomad;
	register long ct = 0;
	
	while (true) {
		
		lastnomad = nomad;
		
		nomad = opbumpflatup (nomad, true);
		
		if (nomad == lastnomad) {
			
			*lnum = ct;
			
			return;
			}
			
		ct++;
		} /*while*/
	} /*opgetnodeline*/


boolean opnewheadrecord (Handle hstring, hdlheadrecord *hnewnode) {
 	
 	/*
 	create a new headline, without linking into any structure.
 	*/
 	
 	register hdlheadrecord h;
 	Handle hnew;
 	
 	if (!newclearhandle (sizeof (tyheadrecord), &hnew)) {
 		
 		disposehandle (hstring);
 		
 		return (false);
 		}
 	
 	h = *hnewnode = (hdlheadrecord) hnew; /*copy into register*/
 	
	(**h).headstring = hstring;
 	
 	(**h).headlinkup = (**h).headlinkdown = (**h).headlinkleft = (**h).headlinkright = h;
 	
 	opnodechanged (h); 
 	
 	(**h).fldirty = true;
 	
 	return (true);
 	} /*opnewheadrecord*/
 

boolean opnewstructure (Handle hstring, hdlheadrecord *hsummit) {

	/*
	create a new summit for the current outline record.
	
	7/9/91 dmb: don't assign to hsummit until success
	*/
	
	register hdlheadrecord hnode;
	
	if (!opnewheadrecord (hstring, hsummit))
		return (false);
	
	hnode = *hsummit; /*copy into register*/
	
	(**hnode).flexpanded = true;
	
	return (true);
	} /*opnewstructure*/


boolean opreleasevisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	register hdlheadrecord h = hnode;
	Handle hrefcon = (Handle) (**h).hrefcon;
	
	if (hrefcon != nil) { /*node has a refcon handle attached*/
	
		hdloutlinerecord x = outlinedata; /*preserve x over callback*/
		
		#if !fljustpacking
		
			(*(**outlinedata).releaserefconcallback) (hnode, (boolean) ((long) refcon));
			
			#endif
		
		opsetoutline (x); /*restore*/
		
		disposehandle (hrefcon); 
		}
	
	disposehandle ((**h).headstring);
	
	disposehandle ((Handle) h); 
	
	return (true);
	} /*opreleasevisit*/


void opreleasenode (hdlheadrecord hnode, boolean fldisk) {
	
	opreleasevisit (hnode, (ptrvoid) ((long) fldisk));
	} /*opreleasenode*/


static boolean opreleaserefconvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	Handle hrefcon = (Handle) (**hnode).hrefcon;
	
	if (hrefcon != nil) { /*node has a refcon handle attached*/
	
		hdloutlinerecord x = outlinedata; /*preserve x over callback*/
		
		#if !fljustpacking
		
			(*(**outlinedata).releaserefconcallback) (hnode, true);
			
			#endif
		
		opsetoutline (x); /*restore*/
		
		disposehandle (hrefcon);
		
		(**hnode).hrefcon = nil;
		}
	
	return (true);
	} /*opreleaserefconvisit			*/


static boolean opresetalllevelsvisit (hdlheadrecord hnode, ptrvoid toplevel) {
	
	(**hnode).headlevel = (long) toplevel;
	
	opresetlevels (hnode);
	
	return (true);
	} /*opresetalllevelsvisit*/


boolean opsetsummit (hdloutlinerecord houtline, hdlheadrecord hnode) {
	
	/*
	establish hnode as the summit of the indicated outline.  dispose of 
	the existing summit, if one exists.  also, make sure that the levels 
	of the outline are set correctly, starting at zero.
	
	5.0.2b15 dmb: set headlevels of all summits -- siblings of hnode
	*/
	
	register hdloutlinerecord ho = houtline;
	register hdlheadrecord h = hnode;
	
	if ((**h).headlevel != 0)
		opbumpvisit (h, down, opresetalllevelsvisit, (ptrvoid) 0L);
	
	if ((**ho).hsummit != nil)
		opreleasenode ((**ho).hsummit, false);
	
	(**ho).hsummit = (**ho).hbarcursor = (**ho).hline1 = h;
	
	(**ho).line1linesabove = 0;
	
	/*
	(**ho).fldirty = true;
	*/
	
	return (true);
	} /*opsetsummit*/


boolean opnewsummit (void) {
	
	/*
	create a new, blank summit for the current outline.  link it into the
	structure accordingly.
	*/
	
	Handle hstring;
	hdlheadrecord hnewsummit;
	
	if (!newemptyhandle (&hstring))
		return (false);
	
	if (!opnewstructure (hstring, &hnewsummit))
		return (false);
	
	return (opsetsummit (outlinedata, hnewsummit));
	} /*opnewsummit*/


boolean optexttooutline (hdloutlinerecord houtline, Handle htext, hdlheadrecord *hnode) {
	
	register hdloutlinerecord ho = houtline;
	
	return ((*(**ho).texttooutlinecallback) (ho, htext, hnode));
	} /*optexttooutline*/


boolean newoutlinerecord (hdloutlinerecord *houtline) {
	
	/*
	create a new outline record, returned in houtline.  we assume nothing
	about outlinewindowinfo or outlinewindow, and we preserve outlinedata.
	
	the rectangles and displayinfo are all zero after we're called.
	
	2.1b5 dmb: always initialize outlinesignature to LAND
	
	4.0b7 dmb: initialize timelastsave to creation time; it's really a modification date.
	*/
	
	#if !fljustpacking
	
		tyconfigrecord config;
	#endif
	
	register hdloutlinerecord ho;
	register boolean fl; 
	
	if (!newclearhandle (sizeof (tyoutlinerecord), (Handle *) houtline))
		return (false);
		
	ho = *houtline; /*copy into register*/
	
	oppushoutline (ho);
	
	fl = opnewsummit ();
	
	oppopoutline ();
		
	if (!fl) {	
	
		disposehandle ((Handle) ho);
		
		*houtline = nil;
		
		return (false);
		}
		
	(**ho).ctexpanded = 1; /*the summit line is expanded*/
	
	(**ho).lineindent = 15;
	
	(**ho).linespacing = oneandalittlespaced;
	
	(**ho).defaultlineheight = 12;

	(**ho).maxlinelen = 255;
	
	(**ho).flbarcursoron = true;
	
	(**ho).flusebitmaps = true;
	
	(**ho).flbuildundo = true;
	
	#if !fljustpacking
	
		(**ho).caretoffset = -1; /*no saved caret location on activation*/
	
		(**ho).selpoint.h = -1; /*no established horizontal cursor offset*/
	
	#endif
	
	opinitcallbacks (ho);
	
	(**ho).timecreated = (**ho).timelastsave = timenow ();
	
	(**ho).iconheight = 16; /*by default, use small icons*/

	(**ho).iconwidth = 16; /*by default, use small icons*/
		
	(**ho).pixelsaftericon = 3; /*skip this many pixels between icon and text*/
	
	(**ho).forecolor = blackcolor;
	
	(**ho).backcolor = whitecolor;
	
	(**ho).outlinescraptype = opscraptype;
	
	(**ho).outlinesignature = 'LAND';
	
	#if !fljustpacking
	
		shellgetconfig (idoutlineconfig, &config);
	
		(**ho).windowrect = config.defaultwindowrect; 
		
		(**ho).fontnum = config.defaultfont;
		
		(**ho).fontsize = config.defaultsize;
		
		(**ho).fontstyle = config.defaultstyle;
	
	#endif
	
	return (true);
	} /*newoutlinerecord*/
	
	
boolean opnewrecord (Rect r, hdloutlinerecord *hnew) {
	
	/*
	create a new outline record and fit it into outlinewindow.  if you want a new
	outlinerecord, but don't have a window around to display it in, call 
	newoutlinerecord instead.
	
	we also set the global outlinedata to point at the newly allocated record.
	*/

	register hdloutlinerecord ho;
	
	opinitdisplayvariables ();
	
	if (!newoutlinerecord (hnew))
		return (false);
	
	ho = (hdloutlinerecord) *hnew;
	
	(**ho).outlinerect = r;
	
	#if !fljustpacking

		(**ho).fontnum = (**outlinewindowinfo).defaultfont;
	
		(**ho).fontsize = (**outlinewindowinfo).defaultsize;
	
		(**ho).fontstyle = (**outlinewindowinfo).defaultstyle;
		
		opsetdisplaydefaults (ho);
		
	#endif
	
	return (true);
	} /*opnewrecord*/
	
	
void opdisposestructure (hdlheadrecord hnode, boolean fldisk) {
	
	/*
	dispose of the structure pointed to by hnode, including all his
	submaterial and all of his siblings' submaterial.
	
	be sure to unlink the node first, if you don't want all his siblings
	to be disposed of too...
	
	if fldisk, then all disk storage used by the structure is dealloc'd
	too.
	*/
	
	opsiblingvisiter (hnode, true, &opreleasevisit, (ptrvoid) ((long) fldisk));
	} /*opdisposestructure*/


static void opdisposerefcons (hdloutlinerecord ho) {
	
	oppushoutline (ho);
	
	opdisabledisplay ();
	
	oppopallhoists ();
	
	opsiblingvisiter ((**ho).hsummit, true, &opreleaserefconvisit, nil);
	
	oprestorehoists ();
	
	oppopoutline ();
	} /*opdisposerefcons*/


void opdisposeoutline (hdloutlinerecord houtline, boolean fldisk) {
	
	/*
	5.0a10 dmb: if we're disposing outlinedata, nil the global
	
	5.1.5b7 dmb: ctpushes, fldisposewhenpopped replaces processinvalidoutline
	*/

	register hdloutlinerecord ho = houtline;
	//long ix;
	
	if (ho == nil) /*defensive driving*/
		return;
	
	//processinvalidoutline (ho);
	
	if ((**ho).ctpushes > 0) {
		
		if (fldisk)
			opdisposerefcons (ho); //do this in current odb context
		
		(**ho).fldisposewhenpopped = true;
		
		return;
		}
	
	(**ho).fldisposewhenpopped = false;
	
	oppushoutline (ho); /*set the current outline to this one*/
	
	#if !fljustpacking
	
		while (oppophoist ()) {} /*pop all the hoists*/
		
		if ((**ho).hbuffer != nil) /*avoid smashing wp globals*/
			opeditdispose ();
		
		#endif
	
	opdisposestructure ((**ho).hsummit, fldisk);
	
	oppopoutline ();
		
	disposehandle ((Handle) ho);

	if (ho == outlinedata)
		outlinedata = nil;
	
	} /*opdisposeoutline*/
	

hdlheadrecord opfirstatlevel (hdlheadrecord hnode) {
	
	/*
	shoot from hnode up to the first guy at his level, returning with
	a handle to the first guy.
	*/
	
	register hdlheadrecord h = hnode;
	register hdlheadrecord next;
	
	if (h == nil) /*defensive driving*/
		return (nil);
		
	while (true) {
		
		next = (**h).headlinkup;
		
		if (next == h) /*no way to go*/
			return (h);
			
		h = next;
		} /*while*/
	} /*opfirstatlevel*/
	

static boolean opcountvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	(*(long*)refcon)++; /*the simplest visit routine*/
	
	return (true);
	} /*opcountvisit*/
	
	
long opcountsubheads (hdlheadrecord hnode, short level) {
	
	long count = 0;
	
	oprecursivelyvisit (hnode, level, &opcountvisit, &count);
	
	return (count);
	} /*opcountsubheads*/
	
	
long opcountheads (void) {
	
	/*
	return the number of headlines linked into the current outline record.
	*/
	
	hdlheadrecord nomad = (**outlinedata).hsummit;
	long ct = 0;
	
	while (true) {
		
		ct += opcountsubheads (nomad, infinity) + 1;
		
		if (!opchasedown (&nomad))
			return (ct);
		} /*while*/
	} /*opcountheads*/
	

static boolean opnotinoutlinevisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	return (hnode != (*(ptrscanrecord)refcon).hnodelookfor);
	} /*opnotinoutlinevisit*/


boolean opnodeinoutline (hdlheadrecord hnode) {
	
	/*
	return true if the given node is in the outline somewhere
	*/
	
	register hdloutlinerecord ho = outlinedata;
	tyscanrecord scanrecord;
	
	scanrecord.hnodelookfor = hnode;
	
	if (opsiblingvisiter ((**ho).hsummit, false, &opnotinoutlinevisit, &scanrecord)) /*no match found*/
		return (false);
	
	return (true);
	} /*opnodeinoutline*/


static boolean opresetlevelsvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	register hdlheadrecord h = hnode;
	
	(**h).headlevel = (**(**h).headlinkleft).headlevel + 1;
	
	return (true);
	} /*opresetlevelsvisit*/
	
	
void opresetlevels (hdlheadrecord hnode) {

	oprecursivelyvisit (hnode, infinity, &opresetlevelsvisit, nil);
	} /*opresetlevels*/


void oppushstyle (hdloutlinerecord ho) {
	
	pushstyle ((**ho).fontnum, (**ho).fontsize, (**ho).fontstyle);
	} /*oppushstyle*/


void oppushcolors (hdloutlinerecord ho) {
	
	pushcolors (&(**ho).forecolor, &(**ho).backcolor);
	} /*oppushcolors*/


boolean opsetactualheadstring (hdlheadrecord hnode, bigstring newstring) {
	
	return (sethandlecontents (stringbaseaddress (newstring), stringlength (newstring), (**hnode).headstring));
	} /*opsetactualheadstring*/


boolean opsetheadtext (hdlheadrecord hnode, Handle hstring) {
	
	/*
	4/3/92 dmb: moved call to textchangedcallback into this bottleneck so 
	that find/replace and op.setlinetext will update menubar items properly.
	call used to be in opwriteeditbuffer
	
	8/14/93 DW: if the node's tmpbit is set, we display the line before
	calling the callback. in clay basket the callback can reorder the 
	outline. too much for a human mind to parse while doing a find/replace.
	
	8/15/93 DW: leave the tmp bit set over call to callback, so that
	the browser can avoid re-inserting the node, again for find & replace.
	we don't keep the lists sorted, to make things fast, avoid jumpiness,
	and also to keep the search order intact (!).
	
	5.0d14 dmb: do as little as possible when the head string isn't changing.
	*/
	
	register hdlheadrecord h = hnode;
	Handle horig;
	bigstring bsorig;
	
	opgetheadstring (hnode, bsorig);
	
	horig = (**hnode).headstring;
	
	(**hnode).headstring = hstring;
	
	if (equalhandles (horig, hstring))
		disposehandle (horig);
	
	else {
		
		opnodechanged (h);
		
		(**h).fldirty = true;
		
		opdirtyoutline ();
		
		if ((**h).tmpbit) { /*special for clay basket, find & replace: update line now*/
		
			opinvalnode (h);
		
			opupdatenow ();
			}
		
		if (!(*(**outlinedata).textchangedcallback) (h, bsorig)) {
			
			/*DW 8/31/93 -- file rename in cb failed*/
			
			(**hnode).headstring = horig;
			
			disposehandle (hstring);
			}
		else
			disposehandle (horig);
		}
			
	(**h).tmpbit = false; /*consume the bit*/

	return (true);
	} /*opsetheadtext*/


boolean opsetheadstring (hdlheadrecord hnode, bigstring bs) {
	
	/*
	6.0a1 dmb: use handles
	*/
	
	Handle hstring;
	
	if (!newtexthandle (bs, &hstring))
		return (false);
	
	return (opsetheadtext (hnode, hstring));
	} /*opsetheadstring*/
	
	
boolean opsetcursorlinetext (bigstring bs) {
	
	boolean fl;
	hdlheadrecord hcursor;
	
	opsettextmode (false);
	
	hcursor = (**outlinedata).hbarcursor;
	
	fl = opsetheadstring (hcursor, bs);
	
	if (fl) {
		
		opinvalnode (hcursor);
		
		opupdatenow ();
		}
	
	return (fl);	
	} /*opsetcursorlinetext*/


boolean opgetheadstring (hdlheadrecord hnode, bigstring bs) {
	
	if (hnode == nil) 
		setstringlength (bs, 0);
	else
		texthandletostring ((**hnode).headstring, bs);
	
	return (true);
	} /*opgetheadstring*/


void opgetsortstring (hdlheadrecord hnode, bigstring bs) {
	
	opgetheadstring (hnode, bs);
	
	if (searchparams.flunicase)
		alllower (bs);		
	} /*opgetsortstring*/


void opcopyformatting (hdloutlinerecord hsource, hdloutlinerecord hdest) {
	
	/*
	5.0.2b12 dmb: copy outlinerect and windowrect too
	*/
	
	register hdloutlinerecord ho1 = hsource;
	register hdloutlinerecord ho2 = hdest;
	
	(**ho2).fontnum = (**ho1).fontnum;
	
	(**ho2).fontsize = (**ho1).fontsize;
	
	(**ho2).fontstyle = (**ho1).fontstyle;
	
	(**ho2).lineindent = (**ho1).lineindent;
	
	(**ho2).linespacing = (**ho1).linespacing;
	
	(**ho2).outlinerect = (**ho1).outlinerect;
	
	(**ho2).windowrect = (**ho1).windowrect;
	} /*opcopyformatting*/



