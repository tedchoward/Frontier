
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

#include "cursor.h"
#include "file.h"
#include "fileloop.h"
#include "kb.h"
#include "memory.h"
#include "strings.h"
#include "shellundo.h"

#include "op.h"
#include "opinternal.h"
#include "claybrowserstruc.h"
#include "claybrowserexpand.h"
#include "claycallbacks.h"


static boolean compoundexpand = false;

static short expandcount; /*for debugging*/


static void browsercalcinsertionpoint (hdlheadrecord hparent, hdlheadrecord *hpre, tydirection *dir) {
	
	/*
	using whatever sorting options we have, determine the insertion point for the
	new line. we return the info that opaddheadline needs, a handle to the "pre"
	node and the direction relative to that node to create the new headline.
	
	special case: if hparent == nil, we do it at the summit level.
	
	possible view settings: name, date, kind, size, label.
	
	9/22/93 dmb: added cases for remaining possible view settings, treating 
	them all as viewbyname. also viewbyname sort order is unicase.
	*/
	
	if (hparent == nil) {
		
		*hpre = oprepeatedbump (down, longinfinity, (**outlinedata).hsummit, true); /*special case*/;
		
		*dir = down;
		}
	else {
		
		if (opnosubheads (hparent)) {
			
			*hpre = hparent;
			
			*dir = right;
			}
		else {
			*hpre = opgetlastsubhead (hparent);
		
			*dir = down;
			}
		}
	} /*browsercalcinsertionpoint*/
	
	
void browserfindinsertionpoint (hdlheadrecord hparent, bigstring bsnewhead, tybrowserinfo *newfileinfo, hdlheadrecord *hpre, tydirection *dir) {
	
	/*
	using whatever sorting options we have, determine the insertion point for the
	new line. we return the info that opaddheadline needs, a handle to the "pre"
	node and the direction relative to that node to create the new headline.
	
	special case: if hparent == nil, we do it at the summit level.
	
	possible view settings: name, date, kind, size, label.
	
	9/22/93 dmb: added cases for remaining possible view settings, treating 
	them all as viewbyname. also viewbyname sort order is unicase.
	*/
	
	hdlheadrecord nomad;
	tyfolderview folderview;
#if filebrowser
	unsigned long time1, time2;
	unsigned long size1, size2;
	short label1, label2;
	bigstring bsnew;
#endif
#if odbbrowser
	hdlhashtable ht;
	hdlhashnode hnode, hnext;
	bigstring bsnext;
	hdlheadrecord hmatch;
#endif		
	
	if (hparent == nil) {
		
		folderview = viewbyname;
		
		nomad = (**outlinedata).hsummit; /*special case*/
		
		*hpre = nomad;
		
		*dir = up;
		}
	else {
		
	#if filebrowser
		tybrowserinfo browserinfo;
		
		browsergetrefcon (hparent, &browserinfo);
		
		folderview = browserinfo.folderview;
	#endif
	
		nomad = hparent;
		
		*hpre = nomad;
		
		*dir = right;
	
		if (!opchaseright (&nomad)) /*has no expanded subs*/
			return;
		}
	
	#if odbbrowser
		
		ht = (*newfileinfo).dirid;
		
		if (hashtablelookupnode (ht, bsnewhead, &hnode)) { // should not fail
			
			hnext = (**hnode).sortedlink;
			
			if (hnext != nil) { // we're not last
				
				gethashkey (hnext, bsnext);
				
				if (opfindhead (nomad, bsnext, &hmatch)) { // shouldn't fail
					
					*hpre = hmatch;
					
					*dir = up;
					
					return;
					}
				}
			}
		
		// default: insert at end of list
		
		*hpre = oprepeatedbump (down, longinfinity, nomad, true);
		
		*dir = down;
		
		return;
		
	#endif
	
	
	#if filebrowser
	
	folderview = -1;
	
	switch (folderview) { /*initialize the things we compare against*/ 
		
		case viewbyname:
		case viewbyicon:
		case viewbysmallicon:
		case viewbycomment:
		case viewbyversion:
			copystring (bsnewhead, bsnew);
			
			alllower (bsnew);
			
			break;
			
		case viewbykind:
			setstringlength (bsnew, 4);
			
			moveleft (&(*newfileinfo).filecreator, &bsnew [1], 4);
	
			break;
			
		case viewbydate:
			time1 = (unsigned long) (*newfileinfo).timemodified;
	
			break;
			
		case viewbysize:
			size1 = (*newfileinfo).filesize;
			
			break;
			
		case viewbycolor:
			label1 = (*newfileinfo).ixlabel;
			
			break;
		} /*switch*/
		
	while (true) {
		
		switch (folderview) {
		
			case viewbyname:
			case viewbyicon:
			case viewbysmallicon:
			case viewbycomment:
			case viewbyversion: {
				bigstring bs;
				
				opgetheadstring (nomad, bs);
				
				alllower (bs);
				
				if (comparestrings (bsnew, bs) == -1) /*bsnewhead is < bs*/
					return;
				
				break;
				}
			
			case viewbykind: {
				tybrowserinfo nomadinfo;
				bigstring bs;
			
				browsergetrefcon (nomad, &nomadinfo);
			
				setstringlength (bs, 4);
			
				moveleft (&nomadinfo.filecreator, &bs [1], 4);
				
				if (comparestrings (bsnew, bs) == -1) /*bsnew is < bs*/
					return;
					
				break;
				}
				
			case viewbydate: {
				tybrowserinfo nomadinfo;
			
				browsergetrefcon (nomad, &nomadinfo);
			
				time2 = (unsigned long) nomadinfo.timemodified;
			
				if (time1 >= time2)
					return;
					
				break;
				}
				
			case viewbysize: {
				tybrowserinfo nomadinfo;
			
				browsergetrefcon (nomad, &nomadinfo);
			
				size2 = nomadinfo.filesize;
			
				if (size1 >= size2)
					return;
					
				break;
				}
				
			case viewbycolor: {
				tybrowserinfo nomadinfo;
			
				browsergetrefcon (nomad, &nomadinfo);
			
				label2 = nomadinfo.ixlabel;
			
				if (label1 >= label2)
					return;
					
				break;
				}
			} /*switch*/
		
		*hpre = nomad;
		
		*dir = down;
		
		if (!opchasedown (&nomad)) /*it's the last guy in the list*/
			return;
		} /*while*/
	#endif
	} /*browserfindinsertionpoint*/
	

boolean browserexpandvisit (bigstring bsitem, tybrowserinfo *info, long refcon) {
	
	/*
	dmb 9/23/93: call opstart/endinternalchange around opaddheadline. this turns 
	off the lineinsertedcallback and prevents an undo from being built -- which 
	would otherwise delete the file!  these calls used to be in browserpreexpand, 
	but that was when this visit routine wasn't directly callable.
	*/
	
	register tyexpandinfo *expandinfo = (tyexpandinfo *) refcon;
	hdlheadrecord hnew;
	
	if (!testheapspace (512)) /*512 bytes should be plenty*/ 
		return (false);
		
	/*debugging code {
		
		bigstring bs;
		
		NumToString (expandcount++, bs);
		
		appsetmessage (app.appwindow, bs);
		}
	*/
		
	if (keyboardescape ()) /*user pressed cmd-period*/
		return (false);
	
	rollbeachball ();
	
	/*insert the new line*/ {
		
		hdlheadrecord hpre;
		tydirection dir;
		
		if ((*expandinfo).flsortnodes)
			browserfindinsertionpoint ((*expandinfo).hparent, bsitem, info, &hpre, &dir);
		else
			browsercalcinsertionpoint ((*expandinfo).hparent, &hpre, &dir);
		
		opstartinternalchange (); /*this operation is not undo-able, and mustn't trigger callbacks*/
		
		opaddheadline (hpre, dir, bsitem, &hnew);
		
		opendinternalchange ();
		}
	
	browsercopyfileinfo (hnew, info);
	
	if ((*expandinfo).flsettmpbits)
		(**hnew).tmpbit = true; /*if expand is cancelled, we nuke all nodes with tmp bit set*/
	
	if ((*info).flfolder) {
		
		(**hnew).flnodeisfolder = true;
		
		if ((*expandinfo).ctlevels > 1) {
		
			if (!browserexpand (hnew, (*expandinfo).ctlevels - 1))
				return (false);
			}
		}
	
	(*expandinfo).hnewnode = hnew; /*some callers need this*/
	
	return (true);
	} /*browserexpandvisit*/


boolean browserexpand (hdlheadrecord hnode, long ctlevels) {
	
	tyexpandinfo expandinfo;
	tybrowserspec fs;
	
	if (opsubheadsexpanded (hnode)) { /*special case for a node that's already expanded*/
		
		hdlheadrecord nomad = (**hnode).headlinkright;
		
		if (ctlevels <= 1) /*we aren't going any deeper*/
			return (true);
		
		while (true) { /*loop over all the subheads of the node*/
			
			if ((**nomad).flnodeisfolder) {
			
				if (!browserexpand (nomad, ctlevels - 1))
					return (false);
				}
				
			if (!opchasedown (&nomad))
				return (true);
			} /*while*/
		} /*dealing with already-expanded nodes*/
	
	claygetfilespec (hnode, &fs);
	
	expandinfo.hparent = hnode;
	
	expandinfo.ctlevels = ctlevels;
	
	expandinfo.flsettmpbits = true;
	
#if odbbrowser
	expandinfo.flsortnodes = false;
#else
	expandinfo.flsortnodes = true;
#endif

	expandcount = 0; /*for debugging*/
	
	return (clayfolderloop (&fs, false, &browserexpandvisit, (long) &expandinfo));
	} /*browserexpand*/
	
	
static boolean expandcancelledvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	if ((**hnode).tmpbit) {
		
		opstartinternalchange ();
		
		opdeletenode (hnode);
		
		opendinternalchange ();
		}
		
	return (true);
	} /*expandcancelledvisit*/
	

boolean browserpreexpand (hdlheadrecord hnode, short ctlevels, boolean flmaycreatesubs) {
	
	/*
	called by opexpand.c before expanding nodes. we create new nodes under
	the node by looping over its folder on disk.
	
	DW 8/18/93: allow the user to press cmd-period to cancel the expand
	operation. set the tmpbit of every newly created node. if the user
	hits cmd-period, or if we run out of memory, we delete all nodes with
	their tmp bit set. important: we don't clear the tmp bits here, we
	depend on them all being clear before the operation starts. the reason:
	some operations do several calls to us. we want them all to be 
	cancelled.
	
	DW 8/30/93: do it right this time. add another piece of baling wire, the
	compoundexpand flag. if it's not true, we clear the tmpbits. otherwise
	we assume they've been cleared.

	5.0a25 dmb: auto-create item in empty table to expand, if new flmaycreatesubs
	parameter is true
	*/
	
	register hdloutlinerecord ho = outlinedata;
	boolean fl;
	
	if (((**hnode).headlinkright != hnode) && (ctlevels <= 1)) /*the nodes are already there*/
		return (true);
		
	if (!(**hnode).flnodeisfolder) /*can't expand a file*/
		return (true);
	
	initbeachball (right);
	
	if (!compoundexpand)
		opcleartmpbits ();
	
	fl = browserexpand (hnode, ctlevels);
	
	if (!fl) /*memory error, or user pressed cmd-period or escape*/
		opsiblingvisiter ((**ho).hsummit, true, &expandcancelledvisit, nil);
	
	if (!compoundexpand)
		opcleartmpbits ();
	
	if ((**hnode).headlinkright == hnode) { // table was empty
		
		if (flmaycreatesubs) {

			hdlheadrecord hnew;
			
			opstartinternalchange ();
			
			opaddheadline (hnode, right, emptystring, &hnew);
			
			opendinternalchange ();
			
			if (hnode == nil)
				return (false);

			browserlineinserted (hnew);
			}
		}
	
	#ifdef xxxfldebug
		opsetctexpanded (ho);
	
		opvalidate (ho);
	#endif
		
	return (fl);
	} /*browserpreexpand*/


boolean browserselectfile (ptrfilespec pfs, boolean flexpand, hdlheadrecord *hnode) {

	bigstring filepath;
	hdlheadrecord nomad;
	boolean flsomethingexpanded = false;
	boolean flreturn = false;
	boolean fldisplaywasenabled;
	
	*hnode = nil;
	
	compoundexpand = true; opcleartmpbits ();
	
	filespectopath (pfs, filepath);
	
	fldisplaywasenabled = opdisabledisplay ();
	
	nomad = (**outlinedata).hsummit;
	
//	addtohistorydisabled = true; /*don't want intermediate expansions added to the history menu*/
	
	while (true) {
		
		bigstring headstring;
		
		if (stringlength (filepath) == 0) 
			break;
		
		firstword (filepath, ':', headstring);
		
		if (stringlength (headstring) == 0)
			copystring (filepath, headstring);
			
		deletestring (filepath, 1, (short) (stringlength (headstring) + 1));
		
		if (!opfindhead (nomad, headstring, &nomad))
			break;
		
		if (stringlength (filepath) == 0) {
			
			flreturn = true; 
			
			break;
			}
			
		if (flexpand) {
		
			if (!opsubheadsexpanded (nomad)) {
				
				if (!opexpand (nomad, 1, false)) { /*user pressed cmd-period, or we ran out of memory*/
					
					openabledisplay ();
					
					return (false);
					}
					
				flsomethingexpanded = true;
				}
			}
			
		if (!opnavigate (right, &nomad))
			break;
		} /*while*/
	
//	addtohistorydisabled = false;
	
	if (fldisplaywasenabled)
		openabledisplay ();
	
	if (flsomethingexpanded) {
	
		opresetscrollbars ();
	
		opinvaldisplay ();
		}
	
	compoundexpand = false;
	
	opcleartmpbits ();
	
	*hnode = nomad;
	
	return (flreturn);
	} /*browserselectfile*/
	
	
boolean browserexpandtofile (ptrfilespec pfs) {
	
	hdlheadrecord hnode;
	
	opclearallmarks ();
	
	if (!browserselectfile (pfs, true, &hnode))
		return (false);
		
	(**outlinedata).flcursorneedsdisplay = true; /*might need to vertical-scroll*/
	
	opjumpto (hnode);
	
	opupdatenow ();
		
	return (true);
	} /*browserexpandtofile*/
	
	
boolean browsercanexpand (hdlheadrecord hnode) {

	tybrowserinfo info;
	
	browsergetrefcon (hnode, &info);
	
	return (info.flfolder && (info.filesize > 0));
	} /*browsercanexpand*/


boolean browserpostcollapse (hdlheadrecord hnode) {
	
	/*
	5.0a25 dmb: auto-delete auto-created node
	
	5.1.4 dmb: after deleting an auto-created node, we still need to delete subs
	*/

	hdloutlinerecord ho = outlinedata;
	boolean fldisplaywasenabled;
	
	killundo ();
	
	/*toss auto-created nodes, if present*/ {
		hdlheadrecord hsub = (**hnode).headlinkright;
		
		browserdeletedummyvalues (hsub);
		
		/*
		tybrowserspec fs;
		tyvaluerecord val;
		
		if ((**hsub).tmpbit2 && opislastsubhead (hsub)) {
			
			if (claygetfilespec (hsub, &fs) && claylookupvalue (&fs, &val))
				if (val.valuetype == novaluetype) {
					
					hashtabledelete (fs.parID, fs.name);
					
					//return (true);
					}
			}
		*/
		}
	 
	fldisplaywasenabled = opdisabledisplay (); /*keep the cursor line from flashing*/
	
	opstartinternalchange ();
	
	/*make sure the outline is (temporarily) not read-only*/ {
	
		boolean fltemp = (**ho).flreadonly;
		
		(**ho).flreadonly = false; /*allow deletion to happen*/
		
		opdeletesubs (hnode);
		
		(**ho).flreadonly = fltemp;
		}
	
	if (fldisplaywasenabled)
		openabledisplay ();
	
	opendinternalchange ();
	
	return (true);
	} /*browserpostcollapse*/


