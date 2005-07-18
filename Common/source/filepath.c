
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

#include "error.h"
#include "memory.h"
#include "strings.h"
#include "ops.h"
#include "file.h"

#define flaux false /*if true, we're running under the A/UX operating system*/

#ifdef MACVERSION

static FSSpec fsdefault; // = {0}; /*we maintain our own default directory*/

boolean directorytopath (long DirID, short vnum, bigstring path) {
	
	CInfoPBRec block;
	bigstring bsdirectory;
	OSErr errcode;
	
	setemptystring (path);
	
	clearbytes (&block, longsizeof (block));
	
	block.dirInfo.ioNamePtr = bsdirectory;
	
	block.dirInfo.ioDrParID = DirID;
	
	do {
		block.dirInfo.ioVRefNum = vnum;
		
		block.dirInfo.ioFDirIndex = -1;
		
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		
		errcode = PBGetCatInfoSync (&block);
		
		if (errcode != noErr)
			return (false);
		
		if (flaux) {
			if (bsdirectory[1] != '/')
				if (!pushchar ('/', bsdirectory))
					return (false);
			} 
		else 
			if (!pushchar (':', bsdirectory))
				return (false);
			
		if (!pushstring (path, bsdirectory))
			return (false);
		
		copystring (bsdirectory, path);
		} while (block.dirInfo.ioDrDirID != fsRtDirID);
	
	return (true);
	} /*directorytopath*/

#endif

boolean filegetdefaultpath (tyfilespec *fs) {
	
	#ifdef MACVERSION
		return (!oserror (FSMakeFSSpec (fsdefault.vRefNum, fsdefault.parID, nil, fs)));
	#endif

	#ifdef WIN95VERSION
		DWORD sz;

		sz = GetCurrentDirectory (257, stringbaseaddress (fsname (fs)));
		
		if (sz == 0) {
			
			oserror (GetLastError ());
			
			return (false);
			}
		
		strcat (stringbaseaddress (fsname (fs)), "\\");

		setstringlength(fsname (fs), sz + 1);
		
		return (true);
	#endif		
	} /*filegetdefaultpath*/


boolean filesetdefaultpath (const tyfilespec *fs) {
	
	#ifdef MACVERSION
		CInfoPBRec pb;
		
		clearbytes (&fsdefault, longsizeof (fsdefault));
		
		if (isemptystring ((*fs).name))
			return (true);
		
		setoserrorparam ((ptrstring) (*fs).name);
		
		clearbytes (&pb, longsizeof (pb));
		
		pb.dirInfo.ioNamePtr = (StringPtr) (*fs).name;
		
		pb.dirInfo.ioVRefNum = (*fs).vRefNum;
		
		pb.dirInfo.ioDrDirID = (*fs).parID;
		
		if (oserror (PBGetCatInfoSync (&pb)))
			return (false);
		
		fsdefault.vRefNum = pb.dirInfo.ioVRefNum;
		
		fsdefault.parID = pb.dirInfo.ioDrDirID;

		return (true);
	#endif

	#ifdef WIN95VERSION
		if (isemptystring (fsname (fs)))
			return (true);
		
		if (!SetCurrentDirectory (stringbaseaddress(fsname (fs)))) {
			
			oserror (GetLastError ());
			
			return (false);
			}

		return (true);
	#endif
	} /*filesetdefaultpath*/


boolean filespectopath (const tyfilespec *fs, bigstring bspath) {
	
	/*
	6/28/91 dmb: when you resolve an alias of a volume, the fsspec has a 
	parent dirid of 1.  we catch this as a special case here, and return 
	the empty string as the path
	
	10/14/91 dmb: make sure folder paths end in :
	
	12/17/91 dmb: don't check for folderness if file doesn't exist-- we 
	dont want to generate any errors here
	
	2.1a7 dmb: if it's a null filespec, return the empty string
	*/
	
	#ifdef MACVERSION
		boolean flfolder;
		
		if ((*fs).parID == fsRtParID) { /*it's a volume*/
			
			copystring ((ptrstring) (*fs).name, bspath);
			
			pushchar (':', bspath);
			
			return (true);
			}
		
		setemptystring (bspath);
		
		if (((*fs).parID == 0) && ((*fs).vRefNum == 0) && (isemptystring ((*fs).name)))
			return (true);
		
		if (!directorytopath ((*fs).parID, (*fs).vRefNum, bspath))
			return (false);
		
		pushstring ((ptrstring) (*fs).name, bspath);
		
		if (fileexists (fs, &flfolder))
			if (flfolder)
				assurelastchariscolon (bspath);
		
		return (true);
	#endif

	#ifdef WIN95VERSION
		// 5.0d12 dmb: use GetFullPath to clean up 8.3 names
		char * fileptr;
		
		copyptocstring (fsname (fs), bspath);
		
		GetFullPathName (bspath, lenbigstring, bspath, &fileptr);

		convertcstring (bspath);
		
		nullterminate (bspath);

		return (true);
	#endif
	} /*filespectopath*/


boolean pathtofilespec (bigstring bspath, tyfilespec *fs) {
	
	/*
	7/5/91 dmb: use FSMakeFSSpec if it's available.  since it only returns 
	noErr if the file exists, and we want to handle non-existant files, we 
	don't give up right away.
	
	12/17/91 dmb: dont append path to default directory if it's a full path
	
	6/11/93 dmb: if FSMakeFSSpec returns fnfErr, the spec is cool (but file 
	doesn't exist)
	
	2.1b2 dmb: added special case for empty string.  also, added drive number 
	interpretation here.
	
	2.1b2 dmb: use new fsdefault for building filespec. note that if bspath 
	isn't a partial path, the vref and dirid will be ignored.
	*/
	
	bigstring bsfolder;
	#ifdef MACVERSION
	OSErr errcode;
	short ix = 1;
	#endif

	clearbytes (fs, sizeof (tyfilespec));	// 5,0d8 dmb

	if (isemptystring (bspath))
		return (true);
	
	#ifdef MACVERSION
	
		#if TARGET_API_MAC_CARBON
			/* 2005-07-17 creedon - don't use FSMakeFSSpec to fill in default directory and volume, see < http://developer.apple.com/technotes/tn/tn2015.html >, 
			not sure if this is best place to do this, seems like it might be better handled at application launch */
			GetApplicationPackageFSSpecFromBundle (&fsdefault);
			#endif

		errcode = FSMakeFSSpec (fsdefault.vRefNum, fsdefault.parID, bspath, fs);
		
		//for some reason if there is a trailing : you get a dirNFErr and it doesn't work
		//This little hack saves us. It still is a fnfErr, but it works.
		#if TARGET_API_MAC_CARBON
		if(errcode == dirNFErr)
		{
			poptrailingchars(bspath, ':');
			errcode = FSMakeFSSpec (fsdefault.vRefNum, fsdefault.parID, bspath, fs);
		}
		#endif	
		if ((errcode == noErr) || (errcode == fnfErr))
			return (true);
		
		if (scanstring (':', bspath, &ix) && (ix > 1)) { /*includes a colon, not the first thing*/
			
			short drivenum;
			
			midstring (bspath, 1, ix - 1, bsfolder); /*pull out volume name*/
			
			if (isallnumeric (bsfolder) && stringtoshort (bsfolder, &drivenum)) { /*it's a number*/
				
				midstring (bspath, ix, stringlength (bspath) - ix + 1, bsfolder);
				
				errcode = FSMakeFSSpec (drivenum, 0, bsfolder, fs);
				
				if ((errcode == noErr) || (errcode == fnfErr))
					return (true);
				}
			}	
		
		return (false);
	#endif

	#ifdef WIN95VERSION
		copystring (bspath, fsname (fs));

		folderfrompath (bspath, bsfolder);

		if ((isemptystring (bsfolder)) && (! fileisvolume(fs))) {

			filegetdefaultpath (fs);

			pushstring (bspath, fsname (fs));
			}
		
		nullterminate (fsname (fs));
		
		return (true);
	#endif

	} /*pathtofilespec*/


boolean setfsfile (tyfilespec *fs, bigstring bsfile) {

	/*
	2004-10-26 aradke: Since the getmacfileinfo/foldertest gymnastics do not
	seem to fit any particular purpose and since none of our callers
	seem to rely it since they usually pass in a file rather than a directory,
	I commented it out.
		
	The only time we get called with a directory is apparently by
	shellopendefaultfile on startup in the Carbon/Mach-O build.
	getapplicationfilespec returns a directory in that case and
	the code below somehow screwed up when called to set the
	filename to Frontier.root so that it wouldn't be found.
	*/
	
	#ifdef MACVERSION
		/*
		CInfoPBRec pb;

		if (getmacfileinfo (fs, &pb) && foldertest (&pb)) {
		
			FSMakeFSSpec ((*fs).vRefNum, pb.dirInfo.ioDrDirID, bsfile, fs);
			
			return (false);
			}
		*/
		
		copystring (bsfile, (*fs).name);

		return (true);
	#endif

	#ifdef WIN95VERSION
		bigstring bsfolder;
		
		folderfrompath (fsname (fs), bsfolder);
		
		pushstring (bsfile, bsfolder);

		copystring (bsfolder, fsname (fs));

		nullterminate (fsname (fs));

		return (true);
	#endif
	} /*setfsfile*/


boolean getfsfile (const tyfilespec *fs, bigstring bsfile) {
	
	/*
	5.0b9 dmb: we're needing this in a few places. better late
	than never
	*/
	
	#ifdef WIN95VERSION
		bigstring bspath;

		filespectopath (fs, bspath);
		
		filefrompath (bspath, bsfile);

		return (true);
	#endif
	
	#ifdef MACVERSION
		copystring (fsname (fs), bsfile);

		return (true);
	#endif
	} /*getfsfile*/


boolean getfsvolume (const tyfilespec *fs, long *vnum) {
	
	/*
	5.1.5b11 dmb: get the volume that is actually specified in the fspec.

	don't expand partial paths using the default directory.
	*/

	#ifdef WIN95VERSION
		return (fileparsevolname ((ptrstring) fsname (fs), vnum, nil));
	#endif
	
	#ifdef MACVERSION
		HVolumeParam pb;
		
		*vnum = (*fs).vRefNum;
		
		if (*vnum == 0)
			return (false);
		
		clearbytes (&pb, sizeof (pb)); /*init all fields to zero*/
		
		pb.ioVRefNum = *vnum;
		
		return (PBHGetVInfoSync ((HParmBlkPtr) &pb) == noErr);
	#endif
	} /*getfsfile*/

OSErr GetApplicationPackageFSSpecFromBundle(FSSpecPtr theFSSpecPtr) {
	/* 2005-07-17 creedon - see , < http://developer.apple.com/technotes/tn/tn2015.html > */
	OSErr err = fnfErr;
	CFBundleRef myAppsBundle = CFBundleGetMainBundle();
	if (myAppsBundle == NULL) return err;
	CFURLRef myBundleURL = CFBundleCopyBundleURL(myAppsBundle);
	if (myBundleURL == NULL) return err;

	FSRef myBundleRef;
	Boolean ok = CFURLGetFSRef(myBundleURL, &myBundleRef);
	CFRelease(myBundleURL);
	if (!ok) return err;

	return FSGetCatalogInfo(&myBundleRef, kFSCatInfoNone, NULL, NULL, theFSSpecPtr, NULL);
	}