
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

#include "dialogs.h"
#include "error.h"
#include "strings.h"
#include "opinternal.h"
#include "claybrowserstruc.h"
#include "claybrowservalidate.h"
#include "langinternal.h" /* 2005-09-26 creedon */
#include "tablestructure.h" /* 2005-09-26 creedon */

#define collidewithequal 0x0001
#define collidewitholder 0x0002
#define collidewithnewer 0x0004

#define nocollisions 0

#define bilateralcollision (collidewitholder + collidewithnewer)

static long dragmodified;

//static char *pastefname;

typedef enum tyaction {
	
	validatemove,
	
	validatepaste
	} tyaction;


byte * actionstrings [] = {
	
	BIGSTRING ("\x06" "moving"),
	BIGSTRING ("\x07" "pasting")
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
	BIGSTRING ("\x40" "Some items in this location have the same names as items you're "),
	BIGSTRING ("\x02" "An"),
	BIGSTRING ("\x07" "A newer"),
	BIGSTRING ("\x08" "An older"),
	#ifdef MACVERSION
		BIGSTRING ("\x0d" " item named “"),
		BIGSTRING ("\x22" "” already exists in this location.")
	#else
		BIGSTRING ("\x0d" " item named \""),
		BIGSTRING ("\x22" "\" already exists in this location.")
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
#pragma unused (hsource)

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


static boolean browsercompareforcollision (hdlheadrecord hnode, long pdragmodified, bigstring bs1, bigstring bs2, ptrdraginfo draginfo) {
// 2006-04-03 - kw --- renamed dragmodified
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
			
			switch (sgn (pdragmodified - info.timemodified)) {
				
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


static boolean collisionvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	5.0b9 dmb: don't conflict with ourself
	*/
	
	bigstring bsnode, bsdrag;
	ptrdraginfo draginfo = (ptrdraginfo) refcon;
	
	if (hnode == (*draginfo).hdrag)
		return (true);
	
	opgetheadstring (hnode, bsnode);
	
	opgetheadstring ((*draginfo).hdrag, bsdrag);
	
	return (browsercompareforcollision (hnode, dragmodified, bsnode, bsdrag, draginfo));
	} /*collisionvisit*/
		

static boolean browsercollisiondialog (hdlheadrecord hdest, ptrdraginfo draginfo) {
#pragma unused (hdest)

	/*
	2005-09-26 creedon: changed default order of buttons, default is Duplicate which is the safe option, checks user.prefs.flReplaceDialogExpertMode and if true Replace is the default option
	*/
	
	bigstring bs, bscollided, prompt;
	bigstring nobutton, yesbutton;
	short itemhit;
	boolean fl, flExpertMode = false;
	
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
	
	getsystemtablescript (idreplacedialogexpertmode, bs); // "user.prefs.flReplaceDialogExpertMode"

	disablelangerror ();

	fl = langrunstring (bs, bs);
	
	enablelangerror ();
	
	if (fl)
		stringisboolean (bs, &flExpertMode);
	
	if (flExpertMode) {
		copystring (duplicatebuttontext, nobutton);
		copystring (replacebuttontext, yesbutton);
		}
	else {
		copystring (duplicatebuttontext, yesbutton);
		copystring (replacebuttontext, nobutton);
		}

	itemhit = threewaydialog (prompt, yesbutton, nobutton, cancelbuttontext);

	if (flExpertMode)
		switch (itemhit) {
		
			case 1:
				itemhit = 2;
				
				break;
			
			case 2:
				itemhit = 1;
				break;
			
			}
	
	switch (itemhit) {
	
		case 1:
			opcleartmpbits ();
			
			/*caller should rename items where conflicts occur*/
			
			return (true);
		
		case 2:
			/*caller should delete all files with their tmpbit set*/
			
			return (true);
		
		default:
			opcleartmpbits ();
			
			return (false);
		} /* switch */

	} /*browsercollisiondialog*/


static boolean validatemovevisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	tybrowserinfo info;
	ptrdraginfo draginfo = (ptrdraginfo) refcon;
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
	
	#if filebrowser
	
	hdlheadrecord hfolder = (**hdest).headlinkleft;
	
	if (hfolder == hdest) {
		
		alertdialog (BIGSTRING ("\x32" "Can’t paste at the top level of a browser outline."));
		
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
		
		alertdialog (BIGSTRING ("\x32" "Can’t paste at the top level of a browser outline."));
		
		return (false);
		}
	
	draginfo.hdest = hfolder;
	
	draginfo.ctcollisions = draginfo.collisiontype = 0;
	
	opcleartmpbits ();
	
	folderloop (clipfolderspec, false, &validatepastecallback, &draginfo);
		
	return (browsercollisiondialog (hfolder, BIGSTRING ("\x07" "pasting")));
	} /*browservalidatefolderpaste*/

#endif


static ptrstring pcommand;


static boolean compareforcopyvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	bigstring bs, bsnode;
	ptrdraginfo draginfo = (ptrdraginfo) refcon;
	
	if (hnode == (*draginfo).hcompare)
		return (true);
	
	opgetheadstring (hnode, bsnode);
	
	opgetheadstring ((*draginfo).hcompare, bs);
	
	if (!equalidentifiers (bsnode, bs)) 
		return (true);
	
	copystring (BIGSTRING ("\x06" "Can’t "), bs);
	
	pushstring (pcommand, bs);
	
	pushstring (BIGSTRING ("\x35" " because there are two or more selected items named “"), bs);
	
	pushstring (bsnode, bs);
	
	pushstring (BIGSTRING ("\x02" "”."), bs);
	
	alertdialog (bs);
	
	return (false); /*stop both traversals*/
	} /*compareforcopyvisit*/
	
	
static boolean validatecopyvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

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
	
	
