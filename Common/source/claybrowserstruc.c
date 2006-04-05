
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
#include "dialogs.h"
#include "error.h"
#include "file.h"
#include "fileloop.h"
#include "kb.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "process.h"
#include "scrap.h"
#include "strings.h"
#include "opinternal.h"
#include "oplineheight.h"
#include "claybrowser.h"
#include "claycallbacks.h"
#include "claybrowserexpand.h"
#include "claybrowservalidate.h"
#include "claybrowserstruc.h"
#if odbbrowser
	#include "shell.rsrc.h"
	#include "shellundo.h"
	#include "tableinternal.h"
	#include "tableverbs.h"
	#include "wpengine.h"
#endif




boolean browsergetrefcon (hdlheadrecord hnode, tybrowserinfo *info) {
	
	return (opgetrefcon (hnode, info, sizeof (tybrowserinfo)));
	} /*browsergetrefcon*/


boolean browsersetrefcon (hdlheadrecord hnode, tybrowserinfo *info) {
	
	return (opsetrefcon (hnode, info, sizeof (tybrowserinfo)));
	} /*browsersetrefcon*/


boolean browsercopyfileinfo (hdlheadrecord hnode, tybrowserinfo *fileinfo) {
	
	/*
	set the fields of a browser refcon handle that come from 
	a file's fileinfo record. ignore the other fields.
	*/
	
//	tybrowserinfo browserinfo;
	
//	browsergetrefcon (hnode, &browserinfo);
	
	if (!browsersetrefcon (hnode, fileinfo))
		return (false);
	
	return (true);
	} /*browsercopyfileinfo*/


static void filepushsuffixnumber (short suffixnum, bigstring name) {
	
	if (suffixnum > 0) {
	
		pushstring ("\x02" " #", name);
	
		pushint (suffixnum, name);
		}
	} /*filepushsuffixnumber*/


static void filepopsuffixnumber (short suffixnum, bigstring name) {
	
	byte bssuffix [32];
	short ixsuffix;
	
	if (suffixnum > 0) {
	
		copystring ("\x02" " #", bssuffix);
		
		pushint (suffixnum, bssuffix);
		
		ixsuffix = patternmatch (bssuffix, name) - 1;

		if (ixsuffix == stringlength (name) - stringlength (bssuffix))
			setstringlength (name, ixsuffix);
		}
	} /*filepushsuffixnumber*/
		

boolean claygetfilespec (hdlheadrecord hnode, tybrowserspec *fs) { 
	
	/*
	turn a headrecord into a filespec.
	
	6/27/93 DW: per recommendation of Think Reference, if it's a
	volume, just set the vRefNum field of the filespec.
	
	8/30/93 DW: rewrite.
	
	9/21/93 dmb: don't need special case for volumes anymore. Think Ref's 
	recommendation is fine for specifying volumes, but it's event better 
	to specify the root directory of the volume, removing any special cases.
	
	5.1.4 dmb: on failure, clear fs
	*/
	
	tybrowserinfo info;
	bigstring name;
	
	if (!browsergetrefcon (hnode, &info)) {
		
		clearbytes (fs, sizeof (tybrowserspec));
		
		return (false);
		}
	
	/*
	if (info.flvolume) { /%special case%/
		
		(*fs).vRefNum = info.vnum;
		
		(*fs).parID = 0; 
		
		setstringlength ((*fs).name, 0);
		
		return (true);
		}
	*/
	
	opgetheadstring (hnode, name);
	
	// dmb 5.0b9 - was: filepushsuffixnumber (info.suffixnum, name);
	
	return (claymakespec (info.vnum, info.dirid, name, fs));
	} /*claygetfilespec*/


boolean browserloadnode (hdlheadrecord hnode) {

	tybrowserspec fs;
	tybrowserinfo fileinfo;
	
	(**hnode).fldirty = true; /*force update to reflect new info*/
	
	claygetfilespec (hnode, &fs);
	
	if (!claygetfileinfo (&fs, &fileinfo))
		return (false);
	
	(**hnode).flnodeisfolder = fileinfo.flfolder;
	
	return (browsercopyfileinfo (hnode, &fileinfo));
	} /*browserloadnode*/
	
	
boolean browserchecklinelength (short newlen, bigstring bs) {

	bigstring bsalert;
		
	if (newlen <= (short) (**outlinedata).maxlinelen) 
		return (true);
	
	copystring ("\x0e" "The file name ", bsalert);
	
	if (stringlength (bs) > 0) {
		
		pushstring ("\x01" "“", bsalert);
		
		pushstring (bs, bsalert);
		
		pushstring ("\x01" "”", bsalert);
		}
	
	pushstring ("\x2e" " is too long. The maximum file name length is ", bsalert);
	
	pushlong ((**outlinedata).maxlinelen, bsalert);
	
	pushstring ("\x0c" " characters.", bsalert);
	
	alertdialog (bsalert);
	
	return (false);
	} /*browserchecklinelength*/

#if 0

static boolean browserupdatefileinfo (hdlheadrecord hnode) {

	/*
	change the in-memory info to reflect what's on disk
	*/ 
	
	tybrowserspec fs;
	tybrowserinfo info;
	
	if (hnode == nil) /*defensive driving*/
		return (false);
	
	claygetfilespec (hnode, &fs);
	
	claygetfileinfo (&fs, &info);

	browsercopyfileinfo	(hnode, &info);
	
	return (true);
	} /*browserupdatefileinfo*/

#endif


boolean browserfileadded (hdlheadrecord hnodeparent, const tybrowserspec *fsnew, hdlheadrecord *hnew) {	
	
	/*
	a new item has been added to a folder, our job is to add the file
	to the display.
	
	dmb 9/23/93: removed setting/clearing of lineinsertedcallbackdisabled. 
	browserexpandvisit now calls opstart/endinternalchange to handle this.
	*/
		
	tyexpandinfo expandinfo;
	tybrowserinfo newfileinfo;
	bigstring fname;
	boolean fl;
	
	claygetfileinfo (fsnew, &newfileinfo);

	claygetfilename (fsnew, fname);
	
	expandinfo.hparent = hnodeparent;

	expandinfo.ctlevels = 1;
	
	expandinfo.flsortnodes = true;
	
	expandinfo.flsettmpbits = false;
	
	/*
	lineinsertedcallbackdisabled = true;
	*/
	
	fl = browserexpandvisit (fname, &newfileinfo, (long) &expandinfo);
	
	/*
	lineinsertedcallbackdisabled = false;
	*/
	
	if (!fl)
		return (false);
	
	opexpandupdate (expandinfo.hnewnode);
		
	*hnew = expandinfo.hnewnode;
	
	return (fl);
	} /*browserfileadded*/


#if 0

static boolean browsernodeislocked (hdlheadrecord hnode) {
	
	tybrowserinfo info;
	
	browsergetrefcon (hnode, &info);
	
	#if filebrowser
		if (info.flnamelocked || info.fllocked)
			return (false);
			
		while (opchaseleft (&hnode)) {}; /*get out to volume node*/
		
		browsergetrefcon (hnode, &info);
		
		if (info.fllocked || info.flhardwarelock)
			return (false);
	#endif
	
	return (true);
	} /*browsernodeislocked*/


static boolean browserpostpaste (hdlheadrecord hfirstpasted) {
	
	browsersortfolder ((**hfirstpasted).headlinkleft);
	
	return (true);
	} /*browserpostpaste*/

#endif


static boolean foldercontainsfile (tybrowserspec *fsfolder, tybrowserspec *fsfile) {
	
	tybrowserdir dirid;
	
	if ((*fsfolder).vRefNum != (*fsfile).vRefNum)
		return (false);
		
	claygetdirid (fsfolder, &dirid);
	
	if ((*fsfile).parID == dirid)
		return (true);
		
	return (false);
	} /*foldercontainsfile*/


static boolean claygetuniquefilename (tybrowserspec *fs, short *suffixnum) {
	
	/*
	5.0b9 dmb: we now expect suffixnum to have a meaningingful initial value.
	
	if it's non-zero, and the headstring ends in that suffix, remove the 
	existing suffix before comparing
	*/

	bigstring origname, name;
	////Code change by Timothy Paustian Sunday, May 7, 2000 8:42:22 PM
	//This isn't used
	//boolean flfolder;
	boolean flmustsuffixize = false;
	
	filepopsuffixnumber (*suffixnum, (*fs).name);
	
	copystring ((*fs).name, origname);
	
	*suffixnum = 0;
	
	if (isemptystring (origname)) {
		#if filebrowser
			return (false);
		#else
			copystring ("\x04" "item", origname);
			flmustsuffixize = true;
		#endif
		}
		
	while (flmustsuffixize || clayfileexists (fs, &flfolder)) {
	
		flmustsuffixize = false; // reset
		
		copystring (origname, name);
		
		(*suffixnum)++;
		
		filepushsuffixnumber (*suffixnum, name);
		
		if (!claymakespec ((*fs).vRefNum, (*fs).parID, name, fs))
			return (false);
		} /*while*/
	
	return (true);
	} /*claygetuniquefilename*/


static tybrowserspec undofolderspec;

static tybrowserspec clipfolderspec;

/*
boolean lineinsertedcallbackdisabled = false;
*/

static hdlheadrecord hdeletednode = nil; /*last node deleted (unlinked) -- may yet be re-inserted*/


static boolean getundofolderspec (void) {
	
	static boolean folderfound = false;

	if (folderfound)
		return (true);
	
	folderfound = claygetspecialfolder ("\x04" "Undo", true, &undofolderspec);
	
	return (folderfound);
	} /*getundofolderspec*/
	
	
static boolean getclipfolderspec (void) {
	
	static boolean folderfound = false;

	if (folderfound)
		return (true);
	
	folderfound = claygetspecialfolder ("\x09" "Clipboard", true, &clipfolderspec);
	
	return (folderfound);
	} /*getclipfolderspec*/


boolean browserclearundo (void) {
	
	getundofolderspec ();
	
	return (clayemptyfilefolder (&undofolderspec));
	} /*browserclearundo*/


static boolean deletetmpbitvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	if ((**hnode).tmpbit) {
		
		(**hnode).tmpbit = false;
		
		opdeletenode (hnode);
		
		browsercommitchanges (); /*moves the file into the undo folder*/
		}
	
	return (true); /*keep visiting*/
	} /*deletetmpbitvisit*/
	
	
boolean browserdeletenodeswithtmpbitset (void) {
	
	/*
	delete files with their tmp bits turned on
	*/

	return (opsiblingvisiter ((**outlinedata).hsummit, true, &deletetmpbitvisit, nil)); 
	} /*browserdeletenodeswithtmpbitset*/


/*
static boolean browsercreatefile (hdlheadrecord hnode, tybrowserspec *fsfolder) {
	
	bigstring bs;
	tybrowserspec fsdest;
	
	opgetheadstring (hnode, bs);
	
	claygetsubitemspec (fsfolder, bs, &fsdest);
	
	if (claycreatefile (&fsdest)) {
	
	/%update the node's refcon to reflect the new file's position & attributes%/
		
		tybrowserinfo browserinfo;
		
		claygetfileinfo (&fsdest, &browserinfo);
		
		browsersetrefcon (hnode, &browserinfo);
		}
	
	return (true);
	} /%browsercreatefile%/
*/

static boolean browsermoveto (hdlheadrecord hnode, tybrowserspec *fsfolder) {
	
	/*
	move or copy the file or folder connected to hnode to the 
	indicated folder.
	
	if a file with that name already exists, append a suffix
	to the name.
	
	the "flnodeonscrap" attribute of a node indicates whether or
	not it owns the file described by the refcon. it can be false
	when a node is on the outline scrap, for example.
	
	dmb 9/20/93: 1) since undo & clip folders are always on sys volume now, we 
	can't assume vRefNum of source & dest are the same; 2) fixed suffixnum 
	updating; was assigning to info instead of browserinfo; 3) fsdest must not 
	take suffixnum into account. suffixnum is essential for determining fssource 
	from the node, but the dest should correspond to the headline name.

	5.0b13 dmb: with the new meaning of suffixnum -- that the node/value name 
	has been modified to be unique in it's new location -- we shouldn't consider 
	and intially-unique name like "item #1" to be suffixized
	
	5.0.2b20 dmb: don't do copy/delete if source or dest vnum is zero
	
	5.1.4 dmb: don't ignore return value of claygetsubitemspec
	*/
	
	tybrowserspec fssource, fsdest;
	tybrowserinfo info;
	short suffixnum;
	bigstring bs;
	boolean flsourceexists;
	
	flsourceexists = claygetfilespec (hnode, &fssource);
	
	if (flsourceexists) { // hnode is associated with a file
		
		if (foldercontainsfile (fsfolder, &fssource)) // already in the folder*/
			return (true);
		}
	
	opgetheadstring (hnode, bs);
	
	if (!claygetsubitemspec (fsfolder, bs, &fsdest))
		return (false);
	
	if (!browsergetrefcon (hnode, &info)) // 5.0b9 dmb
		info.suffixnum = 0;
	
	suffixnum = info.suffixnum;
	
	claygetuniquefilename (&fsdest, &suffixnum);
	
	if (!flsourceexists) {
		
		if (!claycreatefile (&fsdest))
			return (false);
		}
	else {
		if (!(**hnode).flnodeonscrap) { /*we own this file -- move it*/
			
			if (((fssource.vRefNum == fsdest.vRefNum) || (fssource.vRefNum == 0) || (fsdest.vRefNum == 0)) && equalidentifiers (fssource.name, fsdest.name)) {
				
				if (!claymovefile (&fssource, fsfolder))
					return (false);
				}
			else { 
				
				if (!claycopyfile (&fssource, &fsdest))
					return (false);
				
				claydeletefile (&fssource);
				}
			}
		else { /*file not owned -- make a copy*/
			
			if (!claycopyfile (&fssource, &fsdest))
				return (false);
			}
		}
	
	/*update the node's refcon to reflect the new file's position & attributes*/ {
		
		tybrowserinfo browserinfo;
		
		claygetfileinfo (&fsdest, &browserinfo);
	
	#if filebrowser
		browserinfo.suffixnum = suffixnum;
	#else
		browserinfo.suffixnum = suffixnum; // dmb 5.0b9: new

		if (suffixnum != info.suffixnum) { // suffix was added or removed
			
			opstartinternalchange ();
			
			opsetactualheadstring (hnode, fsdest.name);
			
			opendinternalchange ();
			}
	#endif
		if (!flsourceexists && isemptystring (bs)) { // a new insertion
			
			(**hnode).tmpbit2 = true;
			
			langexternaldontsave (fsdest.parID, fsdest.name); // 5.1.4
			
			browserinfo.suffixnum = 0; // we made up the name, it's real w/whatever suffix
			}
		
		browsersetrefcon (hnode, &browserinfo);
		}
	
	(**hnode).flnodeonscrap = false;
	
	return (true);
	} /*browsermoveto*/


boolean browsergetparentspec (hdlheadrecord hnode, tybrowserspec *fsparent) {
	
	/*
	5.13.97 dmb: fixed summit (root table) case

	5.0.2b21 dmb: use new shellgetdatabase to get parent of summit
	*/

	hdlheadrecord hparent = (**hnode).headlinkleft;
	hdltableformats hformats;
	
	if (hparent == hnode) { // it's a summit
		
		hformats = (hdltableformats) (**outlinedata).outlinerefcon;
		
		shellgetdatabase (shellwindow, &(*fsparent).vRefNum); // was: databasedata; what about GDBs?

		(*fsparent).parID = (**hformats).htable; // 5.0d18 dmb: this is as clean as it gets for now
		
		setemptystring ((*fsparent).name);

		return (true);
		}
	
	return (claygetfilespec (hparent, fsparent));
	} /*browsergetparentspec*/


static boolean safedragvisit (hdlheadrecord hnode, ptrvoid punsafe) {
	
	if ((**hnode).tmpbit) {
		
		opgetheadstring (hnode, punsafe);
		
		return (false);
		}
	
	return (true);
	} /*safedragvisit*/


boolean browserpredrag (hdlheadrecord *htarget, tydirection *dragdir) {
	
	/*
	the user dragged the selection to the indicated destination. it's our
	job to interpret the move, and return true only if the caller should 
	move (or copy) the selection to the destination.
	
	if files are being dragged into a folder, we must first delete any name
	conflicts, with confirmation.
	
	if files are being dragged into an application, we do the launch and 
	return false; no move it to take place
	
	9/22/93 dmb: dragging move can give us a dir of up; handle it.
	
	5.0.2b18 dmb: if a marked headline comes back with its tmpbit set, then
	the move is trying to replace its ancestor -- don't let it!
	*/
	
	tybrowserinfo info;
	hdlheadrecord hpre = *htarget;
	tydirection dir = *dragdir;
	bigstring bsunsafe, bsmsg;
	
	if (dir == right) {
		
		browsergetrefcon (hpre, &info);
		
		if (!info.flfolder)
			return (false);
		}
	
	if (!browservalidatemove (hpre, nil, dir))
		return (false);
	
	if (!opvisitmarked (down, safedragvisit, bsunsafe)) {
		
		opcleartmpbits ();
		
		#ifdef MACVERSION
			parsedialogstring ("\x3b" "Can’t move “^0” here because it would replace its ancestor.", bsunsafe, nil, nil, nil, bsmsg);
		#else
			parsedialogstring ("\x3b" "Can't move \"^0\" here because it would replace its ancestor.", bsunsafe, nil, nil, nil, bsmsg);
		#endif
		
		alertdialog (bsmsg);
		
		return (false);
		}
	
	if ((**hpre).tmpbit) { //work hard to find a safe target
		
		assert (dir == down);
		
		while ((**hpre).tmpbit) {  // look above us
			
			if (!opchaseup (&hpre))
				break;
			}
		
		if ((**hpre).tmpbit) { // reset and look below
			
			hpre = *htarget;
			
			*dragdir = up;
			
			while ((**hpre).tmpbit) {
				
				if (!opchasedown (&hpre))
					break;
				}
			}
		
		if ((**hpre).tmpbit) { //go to our parent
			
			opchaseleft (&hpre);

			*dragdir = right;
			}
		}
	
	*htarget = hpre;
	
	browserdeletenodeswithtmpbitset ();
	
	return (true); /*go ahead with the move*/
	
	#if filebrowser
	
	if (info.filetype == 'APPL') { /*need to see if app handles file type*/
		
		tybrowserspec fapp, fdoc;
		
		claygetfilespec (htarget, &fapp);
		
		claygetfilespec ((**outlinedata).hbarcursor, &fdoc);
		
		claylaunchappwithdoc (&fapp, &fdoc, true);
		}
	
	#endif
	
	} /*browserpredrag*/


boolean browserdragcopy (hdlheadrecord hmove, hdlheadrecord hdest) {
	
	/*
	we're moving hmove down or to the right of hdest. if they're 
	not both from the same volume, we need to move a copy, not the 
	original.
	*/
	
	tybrowserspec fs1, fs2;
	
	claygetfilespec (hmove, &fs1);
	
	claygetfilespec (hdest, &fs2);
	 
	if (fs1.vRefNum != fs2.vRefNum) /*must copy if it's cross-volumes*/
		return (true);
		
	if (keyboardstatus.floptionkey || optionkeydown ()) {
		
		bigstring bs;
		
		copystring ("\x20" "Copy or move the selected items?", bs);
		
	//	return (twowaydialog (bs, "\x04" "Copy", "\x04" "Move"));
		return (msgdialog (bs));
		}
		
	return (false);
	} /*browserdragcopy*/


void browsersortfolder (hdlheadrecord hnode) {
	
	/*
	hnode is a folder whose sort order has changed. unlink all its
	subnodes and reinsert them into the list sorted according to the
	new order.
	*/
	
	hdlheadrecord nomad = (**hnode).headlinkright, nextnomad;
	tybrowserinfo browserinfo;
	bigstring bs;
	hdlheadrecord hpre;
	tydirection dir;
	hdlscreenmap hmap;
	
	if (nomad == hnode) /*no subs expanded, nothing to do*/
		return;
		
	(**hnode).headlinkright = hnode; /*no longer has any kids*/
	
	initbeachball (right);
	
	/*opupdatenow ();*/
	
	opstartinternalchange (); /*this operation is not undo-able*/
	
	opnewscreenmap (&hmap);
	
	while (true) {
		
		nextnomad = (**nomad).headlinkdown;
		
		browsergetrefcon (nomad, &browserinfo);
		
		opgetheadstring (nomad, bs);
		
		browserfindinsertionpoint (hnode, bs, &browserinfo, &hpre, &dir);
		
		opdeposit (hpre, dir, nomad);
		
		if (nextnomad == nomad)
			break;
			
		nomad = nextnomad;
		
		rollbeachball ();
		} /*while*/
		
	opinvalscreenmap (hmap);
		
	opendinternalchange ();
	
	opsetctexpanded (outlinedata);
	
	opsetscrollpositiontoline1 ();
	
	#ifdef fldebug
		opvalidate (outlinedata);
	#endif
	} /*browsersortfolder*/


void browserinsertagain (hdlheadrecord hnode) {
	
	/*
	the node's text or other info has been changed, it might sort
	to a different place in its parent folder's list. we unlink it
	from the structure, and link it back in where it belongs.
	
	9/24/93 dmb: don't want an undo built for this operation, since 
	it's a passive reordering going on here.
	
	5.0d14 dmb: do nothing when hnode is the only node at its level.
	particularly imporant when hnode is an only summit.
	*/
	
	hdlheadrecord hparent;
	bigstring bs;
	tybrowserinfo browserinfo;
	hdlheadrecord hpre;
	tydirection dir;
	hdlscreenmap hmap;
	
	if (opcountatlevel (hnode) == 1) /*has no siblings, can't change order*/
		return;
	
	hparent = (**hnode).headlinkleft;
	
	if (hparent == hnode) /*it's a top-level node*/	
		hparent = nil;
	
	opstartinternalchange (); /*don't want this to trigger callbacks or add to undo stack*/
	
	opgetheadstring (hnode, bs);
	
	opnewscreenmap (&hmap);
	
	opunlink (hnode);
	
	browsergetrefcon (hnode, &browserinfo);
	
	browserfindinsertionpoint (hparent, bs, &browserinfo, &hpre, &dir);
	
	opdeposit (hpre, dir, hnode);
	
	(**outlinedata).hbarcursor = hnode;
	
// 2/20/97 dmb: shouldn't need this:	opsetctexpanded ();
	
	opinvalscreenmap (hmap);
	
	opendinternalchange ();
	} /*browserinsertagain*/
	
	
boolean browsercopyrefcon (hdlheadrecord hsource, hdlheadrecord hdest) {
	
	/*
	called while processing the Edit/Copy command. we copy the refcon
	handle, but set the flnodeonscrap bit true, making this a reference 
	to the file. if this node gets deleted we don't delete the file.
	*/

	if (!opcopyrefconroutine (hsource, hdest))
		return (false);
	
	(**hdest).flnodeonscrap = true; /*we don't own the file*/
	
	return (true);
	} /*browsercopyrefcon*/


static boolean browsergetnodevalue (hdlheadrecord hnode, tyvaluerecord *val) {
	
	tybrowserspec fs;
	hdlhashnode hhashnode;
	
	if (!claygetfilespec (hnode, &fs))
		return (false);

	return (claylookupvalue (&fs, val, &hhashnode));
	} /*browsergetnodevalue*/


boolean browsertextualizerefcon (hdlheadrecord hnode, Handle htext) {
#pragma unused (htext)

	tyvaluerecord val;

	if (!browsergetnodevalue (hnode, &val))
		return (false);
	

	return (true);
	} /*browsertextualizerefcon*/


boolean browserreleaserefcon (hdlheadrecord hnode, boolean fldisk) {
	
	/*
	the node is going away. if we own the file we must delete it. this is how
	files get deleted from the Undo folder.
	
	flinternalchange is true, for example, when you collapse a folder. the
	nodes are deleted, but the files must not be.
	*/
	
	if (!opinternalchange () && fldisk && (!(**hnode).flnodeonscrap)) { 
		
		tybrowserspec fs;

		claygetfilespec (hnode, &fs);
		
		claydeletefile (&fs); 
		}
	
	if (hnode == hdeletednode)
		hdeletednode = nil;
	
	return (true);
	} /*browserreleaserefcon*/
	
	
static boolean lineinsertvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	/*
	set the node's refcon info to reflect its current position in
	the outline hierarchy.
	*/
	
	tybrowserinfo info;
	tybrowserspec fsparent;
	
	browsergetparentspec (hnode, &fsparent);
	
	if (browsergetrefcon (hnode, &info)) {
		
		info.vnum = fsparent.vRefNum;
		
		claygetdirid (&fsparent, &info.dirid);
		
		browsersetrefcon (hnode, &info);
		}
	
	(**hnode).flnodeonscrap = false;
	
	return (true);
	} /*lineinsertvisit*/


boolean browserlineinserted (hdlheadrecord hnode) {
	
	/*
	dmb 9/23/93: no more lineinsertedcallbackdisabled flag. there's an 
	op mechanism for this, opstart/endinternalchange, that also turns 
	off undo (very important!)
	*/
	
	tybrowserspec fsparent;
	
	/*
	if (lineinsertedcallbackdisabled)
		return (true);
	*/
	
	if (hnode == hdeletednode) /*finishing a move operation*/
		hdeletednode = nil;
	else
		browsercommitchanges ();
	
	/*
	if ((**hnode).flnewlyinsertednode)
		browsercreatefile (hnode, &fsparent);
	
	else*/ {
		
		browsergetparentspec (hnode, &fsparent);
		
		if (!browsermoveto (hnode, &fsparent))
			return (false);
		}
	
	oprecursivelyvisit (hnode, infinity, &lineinsertvisit, nil); 
	
	return (true);
	} /*browserlineinserted*/


boolean browserlinedeleted (hdlheadrecord hnode) {
	
	browsercommitchanges (); /*flush previous deletion*/
	
	hdeletednode = hnode;
	
	return (true);
	} /*browserlinedeleted*/


static boolean notownrefconvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	(**hnode).flnodeonscrap = true;
	
	return (true);
	} /*notownrefconvisit*/


static boolean closeownedwindowsvisit (hdlhashnode hn, ptrvoid refcon) {
#pragma unused (refcon)

	tyvaluerecord val = (**hn).val;
	hdlexternalvariable hv;
	hdlwindowinfo hinfo;
	
	if (val.valuetype != externalvaluetype)
		return (true);
	
	if (langexternalwindowopen (val, &hinfo)) {
		
		flinhibitclosedialogs = true;
		
		shellclosewindow ((**hinfo).macwindow);
		
		flinhibitclosedialogs = false;
		}
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if (istablevariable (hv) && (**hv).flinmemory) {
		
		return (hashtablevisit ((hdlhashtable) (**hv).variabledata, closeownedwindowsvisit, nil));
		}
	
	return (true);
	} /*closeownedwindowsvisit*/


static boolean closeownedwindows (hdlheadrecord hnode) {
	
	/*
	hnode has been deleted, but with undo. so the external value nodes 
	aren't being disposed to trigger window closing. tablesymboldeleted 
	could theoretically take care of this, but traversing the outline is 
	easier & faster. in fact, tablesymboldeleted can't do it, because the
	node is already unlinked. it would have to be the symbolunlinking 
	callback, with an additional parameter (fldisposing), newly-hooked by
	tableexternal.c
	*/
	
	tybrowserspec fs;
	hdlhashnode hn;
	
	if (!claygetfilespec (hnode, &fs))
		return (false);
	
	if (!hashtablelookupnode (fs.parID, fs.name, &hn))
		return (false);
	
	return (closeownedwindowsvisit (hn, nil));
	} /*notownrefconvisit*/


boolean browsercommitchanges (void) {
	
	boolean fl;
	
	if (hdeletednode == nil)
		return (true);
		
	getundofolderspec (); 
	
	closeownedwindows (hdeletednode); 
	
	fl = browsermoveto (hdeletednode, &undofolderspec);
	
	oprecursivelyvisit (hdeletednode, infinity, &notownrefconvisit, nil); 
	
	hdeletednode = nil;
	
	return (fl);
	} /*browsercommitchanges*/


#if 0

static boolean xxxbrowserexportscrap (hdloutlinerecord hscrap) {
	
	hdlheadrecord nomad = (**hscrap).hsummit;
	
	if (!getclipfolderspec ())
		return (false);
		
	while (true) {
		
		if (!browsermoveto (nomad, &clipfolderspec))
			return (false);
		
		oprecursivelyvisit (nomad, infinity, &notownrefconvisit, nil);
		
		if (!opchasedown (&nomad))
			return (true);
		} /*while*/
	} /*browserexportscrap*/

#endif


static void tabledisposescrap (hdloutlinerecord houtline) {
	
	opdisposeoutline (houtline, false);
	} /*tabledisposescrap*/


static boolean tableexportscrap (hdloutlinerecord houtline, tyscraptype totype, Handle *hexport, boolean *fltempscrap) {
	
	/*
	5.0b9 dmb: for single headline table selection, use 4.x value exporting 
	*/
	
	hdlheadrecord hsummit = (**houtline).hsummit;

	*fltempscrap = true; /*usually the case*/
	
	if ((totype != hashscraptype) && opcountatlevel (hsummit) == 1) { // converting single-selection, use 4.x code
		
		tyvaluerecord val;
		
		if (!browsergetnodevalue (hsummit, &val))
			return (false);

		return (tableexportscrapvalue (&val, totype, hexport, fltempscrap));
		}

	switch (totype) {
		
		case hashscraptype: /*export flat version for system scrap*/
			return (oppackoutline (houtline, hexport));
		
		case opscraptype:
		case scriptscraptype:
			*hexport = (Handle) houtline; /*op and script scraps are the same*/
			
			*fltempscrap = false; /*it's the original, not a copy*/
			
			return (true);
		
		case textscraptype:
			return (opoutlinetonewtextscrap (houtline, hexport));
			
		default:
			return (false);
		}
	} /*tableexportscrap*/


boolean browsersetscrap (hdloutlinerecord houtline) {
	
	/*
	5.0d5 dmb: move to scrap folder here, before setting shell scrap
	*/
	
	hdlheadrecord nomad = (**houtline).hsummit;
	hdlhashtable cliptable;
	
	if (!getclipfolderspec ())
		return (false);
	
	if (!claygetdirid (&clipfolderspec, &cliptable))
		return (false);
	
	emptyhashtable (cliptable, false);
	
	while (true) {
		
		if (!browsermoveto (nomad, &clipfolderspec))
			return (false);
		
		oprecursivelyvisit (nomad, infinity, &notownrefconvisit, nil);
		
		if (!opchasedown (&nomad))
			break;
		} /*while*/
	
	return (shellsetscrap ((Handle) houtline, hashscraptype,
				(shelldisposescrapcallback) &tabledisposescrap,
				(shellexportscrapcallback) &tableexportscrap));
	} /*browsersetscrap*/


boolean browsergetscrap (hdloutlinerecord *houtline, boolean *fltempscrap) {
	
	Handle hscrap;
	tyscraptype scraptype;
	
	if (!shellgetscrap (&hscrap, &scraptype))
		return (false);
	
	if (scraptype == hashscraptype) {
		
		*houtline = (hdloutlinerecord) hscrap;
		
		*fltempscrap = false; /*we're returning a handle to the actual scrap*/
		
		return (true);
		}
	
	return (false);
	} /*browsergetscrap*/


boolean browserdeletedummyvalues (hdlheadrecord hnode) {

	/*
	hnode is no longer being displayed in its window.
	
	if it or any of its siblings is an auto-created node, toss the corresponding 
	table node
	
	return true if no errors occur
	*/
	
	tybrowserspec fs;
	tyvaluerecord val;
	hdlhashnode hhashnode;
	
	while (true) {
	
		if ((**hnode).tmpbit2) {
			
			if (claygetfilespec (hnode, &fs) && claylookupvalue (&fs, &val, &hhashnode)) {
			
				if (val.valuetype == novaluetype)
					if (!hashtabledelete (fs.parID, fs.name))
						return (false);
				}
			}
		
		if (!opchasedown (&hnode))
			return (true);
		}
	} /*browserdeletedummyvalues*/


