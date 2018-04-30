
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
#include "strings.h"
#include "opinternal.h"
#include "claybrowserstruc.h"
#include "claybrowserexpand.h"
#include "claycallbacks.h"
#if odbbrowser
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "tableverbs.h"
#include "tableinternal.h"
#include "tablestructure.h"
#endif



#if filebrowser

boolean claymakespec (tybrowservol vnum, tybrowserdir dirid, bigstring fname, tybrowserspec *pfs) {

	return (filemakespec (vnum, dirid, fname, pfs));
	} /*claymakespec*/

#endif

#if odbbrowser

boolean claymakespec (tybrowservol vnum, tybrowserdir dirid, bigstring fname, tybrowserspec *pfs) {

	(*pfs).vRefNum = vnum;
	
	(*pfs).parID = dirid;
	
	copystring (fname, (*pfs).name);
	
	return (dirid != 0L);
	} /*claymakespec*/

#endif


#if odbbrowser

boolean claylookupvalue (const tybrowserspec *fs, tyvaluerecord *val, hdlhashnode * hnode) {

	return (hashtablelookup ((*fs).parID, (*fs).name, val, hnode));
	} /*claylookupvalue*/

	
boolean claycopyfile (const tybrowserspec *fsource, const tybrowserspec *fdest) {
	
	/*
	5.1b21 dmb: push/pop databases
	
	5.1.4 dmb: hook, trap errors
	*/

	tyvaluerecord val;
	boolean fl;
	Handle hpacked;
	hdldatabaserecord hdatabase;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	bigstring bspackerror;
	hdlhashnode hnode;
	
	fl = claylookupvalue (fsource, &val, &hnode);
	
	if (!fl)
		return (true); /*not fatal error; false is returned to caller*/
	
	if (!flscriptrunning)
		langhookerrors ();
	
	langtraperrors (bspackerror, &savecallback, &saverefcon);
	
	hdatabase = (*fsource).vRefNum;

	if (hdatabase)
		dbpushdatabase (hdatabase);
	
	fl = langpackvalue (val, &hpacked, hnode);
	
	if (hdatabase)
		dbpopdatabase ();

	if (!fl) /*error packing -- probably out of memory*/
		goto exit;
	
	hdatabase = (*fdest).vRefNum;
	
	if (hdatabase)
		dbpushdatabase (hdatabase);
	
	fl = langunpackvalue (hpacked, &val);
	
	if (hdatabase)
		dbpopdatabase ();

	disposehandle (hpacked);
	
	opstartinternalchange ();
	
	if (fl) {
		
		fl = hashtableassign ((*fdest).parID, (*fdest).name, val);
		
		if (!fl)
			disposevaluerecord (val, true);
		}
	
	opendinternalchange ();
	
exit:
	
	languntraperrors (savecallback, saverefcon, !fl);
	
	if (!flscriptrunning)
		langunhookerrors ();
	
	if (!fl)
		shellerrormessage (bspackerror);
	
	return (fl);
	} /*claycopyfile*/


boolean claycreatefile (const tybrowserspec *fs) {
	
	tyvaluerecord val;
	boolean fl;
	
	initvalue (&val, novaluetype);
	
	opstartinternalchange ();
	
	fl = hashtableassign ((*fs).parID, (*fs).name, val);
	
	opendinternalchange ();
	
	return (fl);
	} /*claycreatefile*/


boolean claydeletefile (const tybrowserspec *fs) {
	
	boolean fl;
	
	opstartinternalchange ();
	
	fl = hashtabledelete ((*fs).parID, (ptrstring) (*fs).name);
	
	opendinternalchange ();
	
	return (fl);
	} /*claydeletefile*/


boolean claymovefile (const tybrowserspec *fs, const tybrowserspec *fsto) {
	
	tyvaluerecord val;
	hdlhashtable hdest;
	boolean fl;
	hdlhashnode hnode;
	
	if (!claygetdirid (fsto, &hdest)) {
		
		langerrormessage (BIGSTRING ("\x1b" "destination must be a table"));
		
		return (false);
		}
	
	opstartinternalchange ();
	
	pushhashtable ((*fs).parID);
	
	fl = hashlookup ((*fs).name, &val, &hnode);
	
	if (fl) {
		
	//	if ((*fs).parID == agentstable)
		
		hashdelete ((*fs).name, false, false); /*don't toss the value*/
		}
		
	
	pophashtable ();
	
	if (fl) {
		
		fl = hashtableassign (hdest, (*fs).name, val);
		
		/*
		if (fl && (hdest == agentstable) {
			
			hashtablelookupnode (hdest, (*fs).name, &hnode);
			
			scriptinstallagent (hnode);
			}
		*/
		}

	opendinternalchange ();
	
	return (true);

	} /*claymovefile*/

#endif

#if filebrowswer

static boolean filedeletevisit (bigstring bsitem, tyfileinfo *info, long refcon) {
	
	tybrowserspec fs;
	
//	if (stringlength (fs.name) == 0) /*defensive driving -- if you turn this off -- it could delete C source code in the program you're testing*/
//		return (false);
	
	claymakespec ((*info).vnum, (*info).dirid, bsitem, &fs);
	
	return (filedelete (&fs));
	} /*filedeletevisit*/


boolean clayemptyfilefolder (tybrowserspec *fsfolder) {
	
	return (folderloop (fsfolder, true, &filedeletevisit, 0));
	} /*clayemptyfilefolder*/

#endif

#if odbbrowser

boolean clayemptyfilefolder (tybrowserspec *fsfolder) {

	return (emptyhashtable ((*fsfolder).parID, true) > 0);
	} /*clayemptyfilefolder*/


#endif


#if filebrowser

boolean claygetdirid (const tybrowserspec * fsfolder, tybrowserdir *dirid) {

	//
	// 2006-06-23 creedon: FSRef-ized
	//
	
	FSRefParam pb;

	if (!getmacfileinfo (fsfolder, &pb))
		return (false);
		
	*dirid = pb.dirInfo.ioDrDirID; 
	
	return (true);
	} // claygetdirid

#endif

#if odbbrowser

static boolean claygetfolder (const tybrowserspec * fsfolder, tybrowserdir *dirid, boolean flinmemory) {
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if ((*fsfolder).parID == nil) {
		
		assert (equalstrings ((*fsfolder).name, nameroottable));
		
		*dirid = roottable;
		
		goto tagandexit;
		}
	
	if (isemptystring ((*fsfolder).name)) {
		
		*dirid = (*fsfolder).parID;
		
		goto tagandexit;
		}
	
	if (!claylookupvalue (fsfolder, &val, &hnode))
		return (false);
	
	if (flinmemory) {
		
		hdlexternalvariable hv = (hdlexternalvariable) val.data.externalvalue;
		
		if (!(**hv).flinmemory)
			return (false);
		}
	
	if (!tablevaltotable (val, dirid, hnode))
		return (false);
	
tagandexit:
	
//	(***dirid).flmayaffectdisplay = true; // let tablesymbol calls know we may care
	
	return (true);
	} /*claygetfolder*/


boolean claygetdirid (const tybrowserspec * fsfolder, tybrowserdir *dirid) {
	
	return (claygetfolder (fsfolder, dirid, false));
	} /*claygetdirid*/


boolean claygetinmemorydirid (const tybrowserspec * fsfolder, tybrowserdir *dirid) {
	
	return (claygetfolder (fsfolder, dirid, true));
	} /*claygetinmemorydirid*/

#endif


#if filebrowser

static boolean filegetprefsspec (bigstring fname, tybrowserspec *fsprefs) {
	
	short vnum;
	long dirid;
	OSErr ec;
	
	ec = FindFolder (kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &vnum, &dirid);
	
	if (oserror (ec)) 
		return (false);
	
	claymakespec (vnum, dirid, fname, fsprefs);
	
	return (true);
	} /*filegetprefsspec*/

#endif



#if filebrowser

static boolean filecreatefolder (const tybrowserspec * fsfolder) {
	
	OSErr ec;
	long folderid;
	
	ec = FSpDirCreate (fsfolder, smSystemScript, &folderid);
	
	return (true);
	} /*filecreatefolder*/

#endif

#if odbbrowser

static boolean filecreatefolder (tybrowserspec * fsfolder) {
	
	tybrowserdir folderid;
	
	return (tablenewsystemtable ((*fsfolder).parID, (*fsfolder).name, (tybrowserdir *) &folderid));
	} /*filecreatefolder*/

#endif

boolean claygetsubitemspec (tybrowserspec *fsfolder, bigstring fname, tybrowserspec *fssubitem) {
	
	/*
	dmb 9/20/93: must return false if claymakespec returns false
	*/
	
	tybrowserdir dirid;
	
	if (!claygetdirid (fsfolder, &dirid))
		return (false);
	
	return (claymakespec ((*fsfolder).vRefNum, dirid, fname, fssubitem));
	} /*claygetsubitemspec*/


boolean claygetspecialfolder (void *foldername, boolean flcreate, tybrowserspec *fsspecialfolder) {

	/*
	a bottleneck for finding special Clay Basket folders. it actually works
	for files too -- it's used to locate the preferences file.
	
	if flcreate is true, and the folder doesn't exist, we create it.
	
	DW 3/12/95: Add another level, the Clay Basket folder is contained within
	the UserLand folder.
	*/
	
	#if filebrowser
		tybrowserspec fsuserland, fsfolder;
		
		if (!filegetprefsspec (BIGSTRING ("\x08" "UserLand"), &fsuserland)) 
			return (false);
			
		if (!clayfileexists (&fsuserland, &flfolder)) {
			
			if (!filecreatefolder (&fsuserland))
				return (false);
			}
		
		if (!claygetsubitemspec (&fsuserland, BIGSTRING ("\x12" "Clay Basket Folder"), &fsfolder))
			return (false);
			
		if (!clayfileexists (&fsfolder, &flfolder)) { 
		
			if (!filecreatefolder (&fsfolder))
				return (false);
			}
		
		if (!claygetsubitemspec (&fsfolder, foldername, fsspecialfolder))
			return (false);
	#endif

	#if odbbrowser
		static hdlhashtable hclaybaskettable = nil;
		
		if (hclaybaskettable == nil) { // first time called
			
			if (!newhashtable (&hclaybaskettable))
				return (false);
			}

		claymakespec (nil, hclaybaskettable, foldername, fsspecialfolder);
	#endif
	
	if (!flcreate)
		return (true);
		
	if (!clayfileexists (fsspecialfolder, &flfolder)) /*doesn't exist*/
		return (filecreatefolder (fsspecialfolder));
		
	return (true);
	} /*claygetspecialfolder*/

#if filebrowser
	
void fileinfotobrowserinfo (tyfileinfo *fileinfo, tybrowserinfo *browserinfo) {
	
	(*browserinfo).flfolder = (*fileinfo).flfolder;
	
	(*browserinfo).flvolume = (*fileinfo).flvolume;
	
	(*browserinfo).fllocked = (*fileinfo).fllocked;
	
	(*browserinfo).flhardwarelock = (*fileinfo).flhardwarelock;
	
	(*browserinfo).flremotevolume = (*fileinfo).flremotevolume;
	
	(*browserinfo).flstationery = (*fileinfo).flstationery;
	
	(*browserinfo).flalias = (*fileinfo).flalias;
	
	(*browserinfo).flejectable = (*fileinfo).flejectable;
	
	(*browserinfo).flnamelocked = (*fileinfo).flnamelocked;
	
	(*browserinfo).flcustomicon = (*fileinfo).flcustomicon;
	
	(*browserinfo).filetype = (*fileinfo).filetype;
	
	(*browserinfo).filecreator = (*fileinfo).filecreator;

	(*browserinfo).timecreated = (*fileinfo).timecreated;
	
	(*browserinfo).timemodified = (*fileinfo).timemodified;
	
	(*browserinfo).vnum = (*fileinfo).vnum;
	
	(*browserinfo).dirid = (*fileinfo).dirid;
	
	(*browserinfo).suffixnum = (*fileinfo).suffixnum; // dmb 5.0b9 - was: 0;
	
	(*browserinfo).ixlabel = (*fileinfo).ixlabel;
	
	(*browserinfo).folderview = (*fileinfo).folderview;
	
	if ((*browserinfo).flfolder) 
		(*browserinfo).filesize = (*fileinfo).ctfiles;
	else
		(*browserinfo).filesize = (*fileinfo).sizedatafork + (*fileinfo).sizeresourcefork;
	} /*fileinfotobrowserinfo*/
	
#endif


#if filebrowser

#pragma pack(2)
typedef struct tycallbackinfo {
	
	tyclayfileloopcallback callback;
	
	long refcon;
	} tycallbackinfo;
#pragma options align=reset


static boolean clayfolderexpandvisit (bigstring bsitem, tyfileinfo *info, long refcon) {
	
	tycallbackinfo *cb = (tycallbackinfo *) refcon;
	tybrowserinfo browserinfo;
	
	if ((*info).flinvisible && (!clayprefs.showinvisibles)) /*invisible file/folder, don't show it*/
		return (true);
	
	fileinfotobrowserinfo (info, &browserinfo);
	
	return ((*(*cb).callback) (bsitem, &browserinfo, (*cb).refcon));
	} /*clayfolderexpandvisit*/

boolean clayfolderloop (const tybrowserspec *pfs, boolean flreverse, tyclayfileloopcallback filecallback, long refcon) {

	tycallbackinfo callbackinfo;
	
	callbackinfo.callback = filecallback;
	callbackinfo.refcon = refcon;
	
	return (folderloop (pfs, flreverse, clayfolderexpandvisit, (long) &callbackinfo));
	} /*clayfolderloop*/

#endif


#if odbbrowser

#pragma pack(2)
typedef struct tycallbackinfo {
	
	hdldatabaserecord hdatabase;
	
	hdlhashtable hparenttable;
	
	tyclayfileloopcallback callback;
	
	long refcon;
	} tycallbackinfo;
#pragma options align=reset

static boolean clayfolderloopvisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
#pragma unused(hnode, val)

	/*
	5.0a3 dmb: set flmayaffectdisplay for the parent table

	5.0a17 dmb: no, let clayfolderloop do it, once, even if table is empty
	*/
	
	tybrowserspec fs;
	tybrowserinfo info;
	tycallbackinfo *cbinfo = (tycallbackinfo *) refcon;
	
	fs.vRefNum = (*cbinfo).hdatabase;
	
	fs.parID = (*cbinfo).hparenttable;
	
	copystring (bsname, fs.name);
	
	claygetfileinfo (&fs, &info);
	
	return (!(*(*cbinfo).callback) (bsname, &info, (*cbinfo).refcon));
	} /*clayfolderloopvisit*/


boolean clayfolderloop (const tybrowserspec *pfs, boolean flreverse, tyclayfileloopcallback filecallback, long refcon) {
#pragma unused(flreverse)

	/*
	5.0.2b21 dmb: set hdatafield according to pfs; no more databasedata refs
	*/

	hdlhashtable htable;
	tycallbackinfo fileloopcallbackinfo;
	
	if (!claygetdirid (pfs, &htable))
		return (false);
	
	(**htable).flmayaffectdisplay = true;
	
	fileloopcallbackinfo.hdatabase = (*pfs).vRefNum;
	
	fileloopcallbackinfo.hparenttable = htable;
	
	fileloopcallbackinfo.callback = filecallback;
	
	fileloopcallbackinfo.refcon = refcon;
	
	return (!hashsortedinversesearch (htable, clayfolderloopvisit, &fileloopcallbackinfo));
	} /*clayfolderloop*/

#endif


boolean claygetfileinfo (const tybrowserspec *fs, tybrowserinfo *info) {
	
	/*
	5.0a3 dmb: side effect: set flmayaffectdisplay flags
	*/
	
#if filebrowser
	
	tyfileinfo fileinfo;
	
	if (!filegetinfo (fs, &fileinfo))
		return (false);
	
	fileinfotobrowserinfo (&fileinfo, info);
	
	return (true);
	
#endif

#if odbbrowser

	tyvaluerecord val;
	hdlexternalvariable hv;
	hdlhashtable ht;
	hdlhashnode hnode;
	long ct;
	
	clearbytes (info, sizeof (tybrowserinfo));
	
	if ((*fs).parID == nil)	{ // a root
	
		(*info).flfolder = true;
		
		hashcountitems (roottable, &ct);
		
		(*info).filesize = ct;
		}
	else {
		if (claylookupvalue (fs, &val, &hnode)) {
			
			if (val.valuetype == externalvaluetype) {
				
				hv = (hdlexternalvariable) val.data.externalvalue;
				
				(**hv).flmayaffectdisplay = true;
				
				if (istablevariable (hv)) {
					
					(*info).flfolder = true;
					
					(*info).filesize = 1; // can expand; don't care about actual #
					
					if ((**hv).flinmemory) {
						
						if (langexternalvaltotable (val, &ht, hnode))
							(**ht).flmayaffectdisplay = true;
						}
					}
				}
			
	//		langgetvalsize (val, &(*info).filesize);
			}
		}
	
	/*		*** replace fields with odb-specific info
	(*fs).filetype = (*fileinfo).filetype;
	
	(*fs).filecreator = (*fileinfo).filecreator;

	(*fs).timecreated = (*fileinfo).timecreated;
	
	(*fs).timemodified = (*fileinfo).timemodified;
	*/
	
	(*info).vnum = (*fs).vRefNum;
	
	(*info).dirid = (*fs).parID;
	
	(*info).suffixnum = /*(*fileinfo).suffixnum*/ 0;
	
	/*
	(*fs).ixlabel = (*fileinfo).ixlabel;
	
	(*fs).folderview = (*fileinfo).folderview;
	
	if ((*fs).flfolder) 
		(*fs).filesize = (*fileinfo).ctfiles;
	else
		(*fs).filesize = (*fileinfo).sizedatafork + (*fileinfo).sizeresourcefork;
	*/
	
	return (true);
	
#endif
	} /*claygetfileinfo*/


boolean claygetfilename (const tybrowserspec *pfs, bigstring name) {

	//
	// 2006-06-23 creedon: FSRef-ized
	//
	
	#if filebrowser
		
		return (getfsfile (pfs, name));
		
	#endif
	
	#if odbbrowser
		
		copystring ((*pfs).name, name);
		
		return (true);
		
	#endif
	
	} // claygetfilename
	
	
boolean clayrenamefile (tybrowserspec *fs, hdlheadrecord headnode) {
	
	/*
	6.2b16 AR. tablesetitemname now takes a headnode param instead of a bigstring
	*/
	
#if filebrowser
	
	bigstring bsnew;
	
	opgetheadstring (headnode, bsnew);
	
	return (renamefile (fs, bsnew));
	
#endif

#if odbbrowser
	
	boolean fl;
	
	opstartinternalchange ();
	
	fl = tablesetitemname ((*fs).parID, (*fs).name, headnode, true);
	
	opendinternalchange ();
	
	return (fl);
#endif	
	} /*clayrenamefile*/


#if filebrowser

static boolean expanddiskvisit (bigstring bsitem, tyfileinfo *info, long refcon) {
	
	/*
	dmb 9/24/93: need this layer between the diskloop and browserexpandvisit 
	to completely fill out the info record. most clients of diskloop don't 
	care about the fields that can only be set from the root directory, but 
	we do.
	*/
	
	tybrowserspec fsdiskroot;
	
	if (filemakespec ((*info).vnum, (*info).dirid, bsitem, &fsdiskroot)) {
		
		if (filegetinfo (&fsdiskroot, info)) {
			
			tybrowserinfo browserinfo;
			
			fileinfotobrowserinfo (info, &browserinfo);
			
			browserexpandvisit (bsitem, &browserinfo, refcon);
			}
		}
	
	return (true);
	} /*expanddiskvisit*/


static void pushadjective (bigstring adjective, bigstring bs) {
	
	bigstring adj;
	
	copystring (adjective, adj); /*we might modify it*/
	
	if (stringlength (bs) > 0) {
	
		pushstring (BIGSTRING ("\x01" " "), bs);
		
		alllower (adj);
		}
		
	pushstring (adj, bs);
	} /*pushadjective*/
	

static void claygetfilekindstring (const tybrowserinfo *info, bigstring bs) {
	
//	FSSpec appspec;
	
	if ((*info).flvolume) {
		
		setstringlength (bs, 0);
	
		if ((*info).flejectable)
			pushadjective (BIGSTRING ("\x09" "Removable"), bs);
			
		if ((*info).flhardwarelock)
			pushadjective (BIGSTRING ("\x09" "Read-only"), bs);
			
		if ((*info).flremotevolume)
			pushadjective (BIGSTRING ("\x06" "Shared"), bs);
		
		if (stringlength (bs) > 0)
			pushstring (BIGSTRING ("\x05" " disk"), bs);
		else
			copystring (BIGSTRING ("\x04" "Disk"), bs);
		
		return;
		}
	
	if ((*info).flfolder) {
		
		copystring (BIGSTRING ("\x06" "Folder"), bs);
		
		return;
		}
	
	if ((*info).flalias) {
		
		copystring (BIGSTRING ("\x05" "Alias"), bs);
		
		return;
		}
		
	switch ((*info).filecreator) {
		
		case 'DMGR': case 'ERIK':
			copystring (BIGSTRING ("\x14" "Desktop Manager file"), bs);
		
			return;

		case 'MACS':
			copystring (BIGSTRING ("\x0f" "System Software"), bs);
			
			return;
		} /*switch*/
		
	switch ((*info).filetype) {
		
		case 'APPL':
			copystring (BIGSTRING ("\x13" "Application program"), bs);
		
			return;
			
		case 'cdev':
			copystring (BIGSTRING ("\x0d" "Control Panel"), bs);
		
			return;
			
		case 'INIT': case 'appe': case 'fext': case 'adev':
			copystring (BIGSTRING ("\x10" "System Extension"), bs);
		
			return;
		
		case 'thng': 
			copystring (BIGSTRING ("\x10" "System Component"), bs);
		
			return;
			
		case 'PRER': case 'PRES':
			copystring (BIGSTRING ("\x11" "Chooser Extension"), bs);
		
			return;
			
		case 'FFIL':
			copystring (BIGSTRING ("\x0d" "Font Suitcase"), bs);
			
			return;
			
		case 'dbgr':
			copystring (BIGSTRING ("\x08" "Debugger"), bs);
			
			return;
			
		case 'dfil':
			copystring (BIGSTRING ("\x0e" "Desk Accessory"), bs);
			
			return;
		} /*switch*/
	
	copystring (BIGSTRING ("\x08" "document"), bs);
	} /*claygetfilekindstring*/
	

static void claygetfatsizestring (const tybrowserinfo *browserinfo, bigstring bs) {
	
	setstringlength (bs, 0);
	
	if ((*browserinfo).flvolume) { 
		
		tybrowserspec fs;
		tyfileinfo fileinfo;
		
		if (!claymakespec ((*browserinfo).vnum, 0, "\0", &fs))
			return;
	
		if (!filegetinfo (&fs, &fileinfo))
			return;

		numbertostring (fileinfo.cttotalbytes, bs);
		
		return;				
		}
		
	if ((*browserinfo).flfolder) {
		
		short ctfiles = (*browserinfo).filesize;
		
		numbertostring (ctfiles, bs);
		
		pushstring (BIGSTRING ("\x05" " item"), bs);
		
		if (ctfiles != 1)
			pushstring (BIGSTRING ("\x01" "s"), bs);
	
		return;
		}
		
	numbertostring ((*browserinfo).filesize, bs);
	} /*claygetfatsizestring*/


#endif

boolean browsertextchanged (hdlheadrecord hnode, bigstring origname) {
	
	/*
	dmb 9/24/93: don't call browserloadnode on hnode itself. renaming a 
	file doesn't change its mod date, so it shouldn't need reloading. if 
	something else has changed, say because something SpaceSaver took some 
	other action in response to the rename, reloading the node isn't 
	thorough enough. it will just prevent the idle loop from picking up 
	the change.
	
	5.0d18 dmb: don't resort node based on text change

	5.0b18 dmb: don't check line length here, with bogus message. it will 
	be checked by rename file
	
	6.2b16 AR: clayrenamefile now takes a hdlheadrecord instead of a bigstring
	*/
	
	tybrowserspec fs;
	bigstring newname;
	tybrowserinfo info;
	
	claygetfilespec (hnode, &fs);
	
	if (!browsergetrefcon (hnode, &info)) // it's a new node
		return (browserlineinserted (hnode));
	
	opgetheadstring (hnode, newname);
	
	/*
	if (!browserchecklinelength (stringlength (newname), newname))
		return (false);
	*/

	if (!info.flvolume) 
		copystring (origname, fs.name);
	
	if (!clayrenamefile (&fs, hnode))
		return (false);
	
	(**hnode).tmpbit2 = false; //user gave it a name, no longer temp
	
	langexternalpleasesave (fs.parID, newname); // 5.1.4
	
//	if (clayprefs.addonnamechange)
//		addnodetohistorymenu (hnode);
	
	
	browserloadnode ((**hnode).headlinkleft); /*mod date changed*/
	
	return (true);
	} /*browsertextchanged*/


