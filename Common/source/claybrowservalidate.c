
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

#ifdef MACVERSION
#include <standard.h>
#endif

#ifdef WIN95VERSION
#include "standard.h"
#endif

#include "dialogs.h"
#include "error.h"
#include "strings.h"
#include "opinternal.h"
#include "claybrowserstruc.h"
#include "claybrowservalidate.h"



#define collidewithequal 0x0001
#define collidewitholder 0x0002
#define collidewithnewer 0x0004

#define nocollisions 0

#define bilateralcollision (collidewitholder + collidewithnewer)

static long dragmodified;

static char *pastefname;

typedef enum tyaction {
	
	validatemove,
	
	validatepaste
	} tyaction;


byte * actionstrings [] = {
	
	"\x06" "moving",
	"\x07" "pasting"
	};


enum {
	ixsomeitems,
	ixan,
	ixanewer,
	ixanolder,
	ixitemnamed,
	ixalreadyexists
	};


byte * dialogstrings [] = {
	"\x40" "Some items in this location have the same names as items you're ",
	"\x02" "An",
	"\x07" "A newer",
	"\x08" "An older",
	#ifdef MACVERSION
		"\x0d" " item named “",
		"\x22" "” already exists in this location."
	#else
		"\x0d" " item named \"",
		"\x22" "\" already exists in this location."
	#endif
	};

typedef struct tydraginfo {

	long ctcollisions;

	short collisiontype;
	
	tyaction action;
	
	hdlheadrecord hdrag;
	
	hdlheadrecord hdest;
	
	tydirection dir;

	hdlheadrecord hcollided;

	hdlheadrecord hcompare;
	} tydraginfo, *ptrdraginfo;


boolean browservalidatedrag (hdlheadrecord hsource, hdlheadrecord hdest, tydirection dir) {
	
	/*
	5.0a5 dmb: same level, resort check is bogus; headlink lefts can be the same for a 
	summit and its child, and two summits have different headlinklefts. but we don't 
	want to enforce this anyway, so I just commented it out

	5.0b15 dmb: removed vestigal sourceinfo code
	*/
	
	tybrowserinfo destinfo;
	
	browsergetrefcon (hdest, &destinfo);
	
	if (destinfo.flvolume) {
		
		if (dir == down)
			return (false);
		
	#if filebrowser
		if (destinfo.fllocked || destinfo.flhardwarelock)
			return (false);
	#endif
		}
		
	if ((!destinfo.flfolder) && (dir == right)) /*can't move to the right of a file*/
		return (false);
	
	/*
	if ((**hsource).headlinkleft == (**hdest).headlinkleft) { //they're at the same level
	
		if ((dir == up) || (dir == down)) //can't re-sort the list with dragging move
			return (false);
		}
	
	browsergetrefcon (hsource, &sourceinfo);
	*/
	
	return (true);
	} /*browservalidatedrag*/


static boolean browsercompareforcollision (hdlheadrecord hnode, long dragmodified, bigstring bs1, bigstring bs2, ptrdraginfo draginfo) {

	/*
	5.0.2b18 dmb: set tmpbit of subs too, so we can detect illegal moves
	*/
	
	tybrowserinfo info;
	
	if (equalidentifiers (bs1, bs2)) {
		
		opsettmpbitvisit (hnode, (ptrvoid) true);
		
		oprecursivelyvisit (hnode, infinity, &opsettmpbitvisit, (ptrvoid) true); // set subs too
		
		++(*draginfo).ctcollisions;
		
		(*draginfo).hcollided = hnode; /*last collision*/
		
		if ((*draginfo).collisiontype < bilateralcollision) { /*still useful to check dates*/
			
			browsergetrefcon (hnode, &info);
			
			switch (sgn (dragmodified - info.timemodified)) {
				
				case +1:
					(*draginfo).collisiontype |= collidewitholder; break;
				
				case 0:
					(*draginfo).collisiontype |= collidewithequal; break;
				
				case -1:
					(*draginfo).collisiontype |= collidewithnewer; break;
				} /*switch*/
			}
		}
	
	return (true);	
	} /*browsercompareforcollision*/


static boolean collisionvisit (hdlheadrecord hnode, ptrdraginfo draginfo) {
	
	/*
	5.0b9 dmb: don't conflict with ourself
	*/
	
	bigstring bsnode, bsdrag;
	
	if (hnode == (*draginfo).hdrag)
		return (true);
	
	opgetheadstring (hnode, bsnode);
	
	opgetheadstring ((*draginfo).hdrag, bsdrag);
	
	return (browsercompareforcollision (hnode, dragmodified, bsnode, bsdrag, draginfo));
	} /*collisionvisit*/
		

static boolean browsercollisiondialog (hdlheadrecord hdest, ptrdraginfo draginfo) {
	
	bigstring prompt;
	short itemhit;
	bigstring bscollided;
	
	if ((*draginfo).collisiontype == nocollisions) /*no confirmation or deletions needed*/	
		return (true);
		
	if ((*draginfo).ctcollisions > 1) {
	
		copystring (dialogstrings [ixsomeitems], prompt);
		
		pushstring (actionstrings [(*draginfo).action], prompt);
		
		pushchar ('.', prompt);
		}
	else {
		copystring (dialogstrings [ixan], prompt);
		
		switch ((*draginfo).collisiontype) {
			
			case collidewithnewer:
				copystring (dialogstrings [ixanewer], prompt);
				
				break;
			
			case collidewitholder:
				copystring (dialogstrings [ixanolder], prompt);
				
				break;
			} /*switch*/
	
		pushstring (dialogstrings [ixitemnamed], prompt);
		
		opgetheadstring ((*draginfo).hcollided, bscollided);
		
		pushstring (bscollided, prompt);
	
		pushstring (dialogstrings [ixalreadyexists], prompt);
		}
	
	#ifdef MACVERSION
		itemhit = threewaydialog (prompt, "\x07" "Replace", "\x09" "Duplicate", "\x06" "Cancel");
	#endif
	#ifdef WIN95VERSION
		itemhit = threewaydialog (prompt, "\x08" "&Replace", "\x0a" "&Duplicate", "\x07" "&Cancel");
	#endif

	switch (itemhit) {
	
		case 1:
			/*caller should delete all files with their tmpbit set*/
			
			return (true);
		
		case 2:
			opcleartmpbits ();
			
			/*caller should rename items where conflicts occur*/
			
			return (true);
		
		default:
			opcleartmpbits ();
			
			return (false);
		}
	} /*browsercollisiondialog*/


static boolean validatemovevisit (hdlheadrecord hnode, ptrdraginfo draginfo) {
	
	tybrowserinfo info;
	hdlheadrecord hdest = (*draginfo).hdest;
	
	(*draginfo).hdrag = hnode;
	
	browsergetrefcon (hnode, &info);
	
	dragmodified = info.timemodified;
	
	if ((*draginfo).dir == right)
		return (oprecursivelyvisit (hdest, 1, &collisionvisit, draginfo));
	else
		return (oplistvisit (opfirstatlevel (hdest), &collisionvisit, draginfo));
	} /*validatemovevisit*/


static boolean browservalidateinsertion (hdlheadrecord hdest, hdlheadrecord hscrap, tydirection dir, tyaction action) {
	
	/*
	confirm that the user wants to overwrite existing files,
	we return false to cancel the operation.
	*/
	
	tydraginfo draginfo;
	
	draginfo.hdest = hdest;
	
	draginfo.dir = dir;

	draginfo.ctcollisions = draginfo.collisiontype = 0;
	
	draginfo.action = action;
	
	opcleartmpbits ();
	 
	if (hscrap == nil)
		opvisitmarked (down, &validatemovevisit, &draginfo);
	else 
		oplistvisit (hscrap, &validatemovevisit, &draginfo); 
	
	return (browsercollisiondialog (hdest, &draginfo));
	} /*browservalidateinsertion*/


boolean browservalidatemove (hdlheadrecord hdest, hdlheadrecord hscrap, tydirection dir) {
	
	return (browservalidateinsertion (hdest, hscrap, dir, validatemove));
	} /*browservalidatemove*/


boolean browservalidatepaste (hdlheadrecord hscrap, hdlheadrecord hdest, tydirection dir) {
	
	/*
	return false to cancel the paste.
	
	assume that paste always happens in the list of the cursor's parent.
	*/
	
	hdlheadrecord hfolder = (**hdest).headlinkleft;
	
	#if filebrowser
	
	if (hfolder == hdest) {
		
		alertdialog ("\x32" "Can’t paste at the top level of a browser outline.");
		
		return (false);
		}
	
	#endif
	
	if (!browservalidateinsertion (hdest, hscrap, dir, validatepaste)) /*user declined to replace already-existing files*/
		return (false);
	
	browserdeletenodeswithtmpbitset ();
	
	return (true);
	} /*browservalidatepaste*/
	

#if 0 /*we're not doing a folder-based clipboard, but if we did, this is how you would validate a paste*/

static boolean pastecollisionvisit (hdlheadrecord hnode, ptrdraginfo draginfo) {
	
	bigstring fname;
	bigstring bsnode;
	
	copystring (pastefname, fname);
	
	opgetheadstring (hnode, bsnode);
	
	return (browsercompareforcollision (hnode, dragmodified, bsnode, fname));
	} /*pastecollisionvisit*/
		

static boolean validatepastecallback (bigstring fname, tyfileinfo *info, ptrdraginfo draginfo) {
	
	dragmodified = (*info).timemodified;
	
	pastefname = (char *) fname;
	
	return (oprecursivelyvisit ((*draginfo).hdest, 1, &pastecollisionvisit, draginfo));
	} /*validatepastecallback*/
	
	
static boolean browservalidatefolderpaste (hdlheadrecord hfolder, FSSpec *clipfolderspec) {
	
	/*
	check for collisions -- if there already is a file in the target folder
	with the same name as one of the items we're pasting, a dialog appears
	confirming the replacement. all files that need to be deleted have their
	tmpbits set. the caller should delete them. we don't do the deletion 
	here so that the deletions can be undoable.
	*/
	
	tydraginfo;
	
	if ((**hfolder).headlinkleft == hfolder) {
		
		alertdialog ("\x32" "Can’t paste at the top level of a browser outline.");
		
		return (false);
		}
	
	draginfo.hdest = hfolder;
	
	draginfo.ctcollisions = draginfo.collisiontype = 0;
	
	opcleartmpbits ();
	
	folderloop (clipfolderspec, false, &validatepastecallback, &draginfo);
		
	return (browsercollisiondialog (hfolder, "\x07" "pasting"));
	} /*browservalidatefolderpaste*/

#endif


static ptrstring pcommand;


static boolean compareforcopyvisit (hdlheadrecord hnode, ptrdraginfo draginfo) {
	
	bigstring bs, bsnode;
	
	if (hnode == (*draginfo).hcompare)
		return (true);
	
	opgetheadstring (hnode, bsnode);
	
	opgetheadstring ((*draginfo).hcompare, bs);
	
	if (!equalidentifiers (bsnode, bs)) 
		return (true);
	
	copystring ("\x06" "Can’t ", bs);
	
	pushstring (pcommand, bs);
	
	pushstring ("\x35" " because there are two or more selected items named “", bs);
	
	pushstring (bsnode, bs);
	
	pushstring ("\x02" "”.", bs);
	
	alertdialog (bs);
	
	return (false); /*stop both traversals*/
	} /*compareforcopyvisit*/
	
	
static boolean validatecopyvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	tydraginfo draginfo;
	
	draginfo.hcompare = hnode;
	
	return (opvisitmarked (down, &compareforcopyvisit, &draginfo));
	} /*validatecopyvisit*/


boolean browservalidatecopy (bigstring bscommand) {
	
	/*
	don't allow a copy if there's a node selected that has the
	same name as another node that's selected. we do the dialog
	here, the caller should just exit if we return false.
	*/
	
	pcommand = bscommand; /*set static to point to string*/
	
	return (opvisitmarked (down, &validatecopyvisit, nil));
	} /*browservalidatecopy*/
	
	
