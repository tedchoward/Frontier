
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Folders.h>
#include <Script.h>

#include "appletdefs.h"
#include "appletmemory.h"
#include "appletstrings.h"
#include "appletfiles.h"
#include "appletfilesinternal.h"
#include "appletfilealias.h"
#include "appletfiledelete.h"
#include "appletfolder.h"



static struct {
	
	FSSpec *fsfile; 
	
	FSSpec fsalias;
	
	boolean fl;
} foldercontains;


boolean diskloop (tyfileloopcallback diskcallback, long refcon) {
	
	/*
	dmb 9/21/93: filegetvolumeinfo takes a vRefNum, not a string.
	*/
	
	HVolumeParam pb;
	short ix;
	
	clearbytes (&pb, longsizeof (pb)); /*init all fields to zero*/
	
	ix = 1; /*start with file index 1*/
	
	while (true) {
		
		bigstring bsvolume;
		tyfileinfo info;
		OSErr errcode;
		
		pb.ioVolIndex = ix++;
		
		pb.ioNamePtr = bsvolume;
		
		errcode = PBHGetVInfoSync ((HParmBlkPtr) &pb);
		
		if (errcode == nsvErr) /*not an error, just ran out of volumes*/
			return (true);
			
		if (fileerror (nil, errcode)) 
			return (false);
		
		filegetvolumeinfo (pb.ioVRefNum, &info);
		
		if (!(*diskcallback) (bsvolume, &info, refcon))
			return (false);
		} /*while*/
	} /*diskloop*/
	
	
boolean folderloop (ptrfilespec pfs, boolean flreverse, tyfileloopcallback filecallback, long refcon) {
	
	/*
	loop through all of the files in the folder at fs, and call filecallback 
	for each one.
	
	if flreverse is true, loop through files backwards to allow for deletions
	
	DW 8/28/93: inexplicably, getmacfileinfo is determining that ctfiles is 1
	greater than the actual number of files in our folder (it's the History
	folder in clayhistorymenu.c). the way the loop was structured, we visit
	no files in the folder in this case. changed it so that on fnfErr we 
	continue the loop. the next time thru it will find a file. hope this
	doesn't break anything else (it shouldn't).
	*/
	
	FSSpec fs = *pfs; /*work with a copy*/
	CInfoPBRec pb;
	long dirid;
	bigstring bsfile;
	short ix;
	long ctfiles;
	tyfileinfo info;
	OSErr ec;
	
	setfileerrorfile (pfs); 
	
	if (!getmacfileinfo (&fs, &pb))
		return (false);
		
	assert (BitTst (&pb.dirInfo.ioFlAttrib, 3)); /*be sure it's a folder*/
	
	dirid = pb.dirInfo.ioDrDirID; /*must remember this for loop body*/
	
	ctfiles = pb.dirInfo.ioDrNmFls;
	
	if (flreverse)
		ix = ctfiles;
	else
		ix = 1; /*start with file index 1*/
	
	while (--ctfiles >= 0) {
		
		pb.dirInfo.ioDrDirID = dirid; /*may be smashed by ioFlNum on previous loop*/
		
		pb.dirInfo.ioFDirIndex = ix;
		
		if (flreverse)
			--ix;
		else
			++ix;
		
		pb.dirInfo.ioNamePtr = bsfile;
		
		ec = PBGetCatInfoSync (&pb);
		
		if (ec == fnfErr) /*DW 8/28/93: continue instead of returning true*/
			continue; 
			
		if (fileerror (nil, ec)) 
			return (false);
		
		filegetinfofrompb (&pb, &info);
		
		if (!(*filecallback) (bsfile, &info, refcon))
			return (false);
		} /*while*/
		
	return (true);
	} /*folderloop*/


static boolean foldercontainsaliascallback (bigstring fname, tyfileinfo *info, long pxinfo) {
	
	FSSpec fs, fsalias;
	
	if (!(*info).flalias) /*keep searching*/
		return (true);
		
	filemakespec ((*info).vnum, (*info).dirid, fname, &fs);
	
	fsalias = fs;
	
	fileresolvealias (&fs, true);
	
	if (equalfilespecs (&fs, foldercontains.fsfile)) {
		
		foldercontains.fl = true;
		
		foldercontains.fsalias = fsalias;
		
		return (false);
		}
		
	return (true);
	} /*foldercontainsaliascallback*/
	
	
boolean foldercontainsalias (FSSpec *fsfolder, FSSpec *fsfile, FSSpec *fsalias) {
	
	/*
	return true if the folder contains an alias to the file, or if the file
	itself is in the folder.
	*/
	
	clearbytes (&foldercontains, longsizeof (foldercontains));
	
	foldercontains.fl = false;
	
	foldercontains.fsfile = fsfile;
	
	folderloop (fsfolder, false, &foldercontainsaliascallback, 0);
	
	if (foldercontains.fl) {
	
		*fsalias = foldercontains.fsalias;
		
		return (true);
		}
		
	return (false);
	} /*foldercontainsalias*/
	

boolean getfolderfilespec (short vnum, long dirid, FSSpec *fsfolder) {
	
	return (filemakespec (vnum, dirid, "\p", fsfolder));
	} /*getfolderfilespec*/
	
	
boolean filegetparentfolderspec (FSSpec *fsfile, FSSpec *fsfolder) {
	
	return (getfolderfilespec ((*fsfile).vRefNum, (*fsfile).parID, fsfolder));
	} /*filegetparentfolderspec*/
	
	
boolean getsystemfolderfilespec (OSType subfolderid, FSSpec *fsfolder) {

	OSErr ec;
	short vnum;
	long dirid;
	
	ec = FindFolder (kOnSystemDisk, subfolderid, kDontCreateFolder, &vnum, &dirid);
	
	if (fileerror (nil, ec)) 
		return (false);
		
	return (getfolderfilespec (vnum, dirid, fsfolder));
	} /*getsystemfolderfilespec*/
	
	
boolean systemfoldercontainsalias (OSType subfolderid, FSSpec *fsfile) {
	
	/*
	return true if the specified sub-folder of the system folder contains
	an alias to the file or the file itself.
	*/
	
	FSSpec fsfolder, fsalias;
	
	if (!getsystemfolderfilespec (subfolderid, &fsfolder))
		return (false);
		
	return (foldercontainsalias (&fsfolder, fsfile, &fsalias));
	} /*systemfoldercontainsalias*/


boolean addaliastospecialfolder (FSSpec *fsorig, OSType subfolderid) {

	FSSpec fsfolder, fsalias;
	bigstring fname;
	long folderid;
	
	if (!getsystemfolderfilespec (subfolderid, &fsfolder))
		return (false);

	filegetfilename (fsorig, fname);
	
	filegetfolderid (&fsfolder, &folderid);
	
	filemakespec (fsfolder.vRefNum, folderid, fname, &fsalias);
		
	return (filemakealias (fsorig, &fsalias));
	} /*addaliastospecialfolder*/
	

static boolean deletealiasescallback (bigstring fname, tyfileinfo *info, long refcon) {
	
	FSSpec *fslookfor = (FSSpec *) refcon;
	FSSpec fs, fsdereferenced;
	
	if (!(*info).flalias) /*keep searching*/
		return (true);
		
	filemakespec ((*info).vnum, (*info).dirid, fname, &fs);
	
	fsdereferenced = fs;
	
	fileresolvealias (&fsdereferenced, true);
	
	if (equalfilespecs (&fsdereferenced, fslookfor)) 		
		filedelete (&fs);
		
	return (true);
	} /*deletealiasescallback*/
	
	
boolean deletealiasesfromfolder (FSSpec *fsorig, FSSpec *fsfolder) {
	
	/*
	delete all aliases that point to the file from the indicated folder.
	*/
	
	folderloop (fsfolder, true, &deletealiasescallback, (long) fsorig);
	
	return (true);
	} /*deletealiasesfromfolder*/

	
boolean deletealiasesfromspecialfolder (FSSpec *fsorig, OSType subfolderid) {

	FSSpec fsfolder;
	
	if (!getsystemfolderfilespec (subfolderid, &fsfolder))
		return (false);
	
	return (deletealiasesfromfolder (fsorig, &fsfolder));
	} /*deletealiasesfromspecialfolder*/

	
boolean filecreatefolder (ptrfilespec fsfolder) {
	
	OSErr ec;
	long folderid;
	
	ec = FSpDirCreate (fsfolder, smSystemScript, &folderid);
	
	return (true);
	} /*filecreatefolder*/
	
	
boolean filegetfolderid (ptrfilespec fsfolder, long *dirid) {
	
	CInfoPBRec pb;

	if (!getmacfileinfo (fsfolder, &pb))
		return (false);
		
	*dirid = pb.dirInfo.ioDrDirID; 
	
	return (true);
	} /*filegetfolderid*/


boolean filegetsubitemspec (FSSpec *fsfolder, bigstring fname, FSSpec *fssubitem) {
	
	/*
	dmb 9/20/93: must return false if filemakespec returns false
	*/
	
	long dirid;
	
	if (!filegetfolderid (fsfolder, &dirid))
		return (false);
	
	return (filemakespec ((*fsfolder).vRefNum, dirid, fname, fssubitem));
	} /*filegetsubitemspec*/
	
	
boolean filesetfolderview (FSSpec *fsfolder, short folderview) {
	
	/*
	DW 9/6/93: the parameter is a short, to save space in clayinfowindow.c. it 
	allows us to use the same callback routine as for other shorts.
	*/

	CInfoPBRec pb;

	if (!getmacfileinfo (fsfolder, &pb))
		return (false);
		
	pb.dirInfo.ioDrUsrWds.frView = (pb.dirInfo.ioDrUsrWds.frView & 0xFF) + (folderview << 8);
	
	return (setmacfileinfo (fsfolder, &pb));
	} /*filesetfolderview*/
	
	
boolean fileemptyfolder (FSSpec *fsfolder) {
	
	return (folderloop (fsfolder, true, &filedeletevisit, 0));
	} /*fileemptyfolder*/
	
	
boolean foldercontainsfile (FSSpec *fsfolder, FSSpec *fsfile) {
	
	long dirid;
	
	if ((*fsfolder).vRefNum != (*fsfile).vRefNum)
		return (false);
		
	filegetfolderid (fsfolder, &dirid);
	
	if ((*fsfile).parID == dirid)
		return (true);
		
	return (false);
	} /*foldercontainsfile*/
	

static boolean flushvolumevisit (bigstring bsitem, tyfileinfo *info, long refcon) {
	
	OSErr ec;
	
	ec = FlushVol (nil, (*info).vnum);
	
	return (true); /*even if error, want to visit all the volumes*/
	} /*flushvolumevisit*/
	
	
boolean fileflushvolumes (void) {
	
	return (diskloop (&flushvolumevisit, 0));
	} /*fileflushvolumes*/
	

boolean filesurefolder (ptrfilespec pfs) {
	
	bigstring bs;
	extern void alertdialog (bigstring);
	
	if (fileexists (pfs))
		return (true);
	
	if (filecreatefolder (pfs))
		return (true);
	
	copystring ("\pFile system error -- can't create a folder named \"", bs);
	
	pushstring ((*pfs).name, bs);
	
	pushstring ("\p\"", bs);
	
	alertdialog (bs);
	
	return (false);
	} /*filesurefolder*/
	
	
boolean filesuresubfolder (ptrfilespec pfsfolder, bigstring foldername, ptrfilespec pfssubfolder) {

	tyfilespec fsfolder = *pfsfolder; /*caller can have the two pfs's be the same thing*/

	if (!filegetsubitemspec (&fsfolder, foldername, pfssubfolder))
		return (false);
	
	return (filesurefolder (pfssubfolder));
	} /*filesuresubfolder*/
	
	


	
	
	
	
	
	
