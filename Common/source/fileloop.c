
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
#include "error.h"
#include "file.h"
#include "oplist.h"
#include "fileloop.h"




#ifdef MACVERSION
typedef struct tyfilelooprecord {
	
	short vnum; /*the volume we're looping through*/
	
	long dirid; /*the folder we're looking in*/
	
	short ixdirectory; /*index for the next file to look at*/
	
	hdllistrecord hfilelist; /*a list of filenames to be looped over*/
	} tyfilelooprecord, *ptrfilelooprecord, **hdlfilelooprecord;
	
	


static boolean fileloopreleaseitem (Handle h) {
	
	/*
	callback routine that disposes of one of our items.  since there are no
	handles linked into our handle, there's nothing to release.
	
	I just wanted to have one of these around so the next guy that uses the
	list facilty knows how to link a releaseitem callback routine.
	*/
	
	return (true);
	} /*fileloopreleaseitem*/
	

boolean diskinitloop (tyfileloopcallback diskfilter, Handle *hdiskloop) {
	
	/*
	same as fileinitloop, but for volumes on line
	
	8/15/92 dmb: added, but commented out, diskfilter support
	*/
	
	tyfilelooprecord info;
	ParamBlockRec pb;
	register hdlfilelooprecord h;
	short ix;
	hdllistrecord hlist;
	
	clearbytes (&pb, sizeof (pb)); /*init all fields to zero*/
	
	clearbytes (&info, sizeof (info));
	
	info.ixdirectory = 1; /*start with the first file*/
	
	if (!newfilledhandle (&info, sizeof (info), hdiskloop))
		return (false);
	
	h = (hdlfilelooprecord) *hdiskloop; /*copy into register*/
	
	hlist = nil;
	
	if (!opnewlist (&hlist, false)) 
		goto error;
	
	(**h).hfilelist = hlist;
	
	(**hlist).releaseitemcallback = &fileloopreleaseitem;
	
	ix = 1; /*start with file index 1*/
	
	while (true) {
		FSSpec fsvol;
		OSErr errcode;
		Handle hstring;
		#if TARGET_API_MAC_CARBON == 1
		HFSUniStr255	theName;
		//Code change by Timothy Paustian Thursday, June 29, 2000 10:29:59 AM
		//Updated to modern call for volume infomation
		//Ii am checking this now against the original.
		FSVolumeRefNum	vRefNum;//this is just a SInt16
		
		//Watch out that the fsvol.name parameter works as expected.
		//this is still not working the way it should
		errcode = FSGetVolumeInfo(0, ix, &vRefNum, kFSVolInfoNone, nil, &theName, nil);
		if (errcode == nsvErr) /*not an error, just ran out of volumes*/
			return (true);
		
		{
		UnicodeToTextInfo	theInfo;
		UnicodeMapping		theMapping;
		theMapping.unicodeEncoding = kTextEncodingUnicodeDefault;
		theMapping.otherEncoding = kTextEncodingMacRoman;
		theMapping.mappingVersion = kUnicodeUseLatestMapping;
		errcode = CreateUnicodeToTextInfo(&theMapping,&theInfo);
		if (oserror (errcode)) // 1/29/97 dmb: call oserror
			goto error;
		//fsvol.name[0] = theName.length;
		errcode = ConvertFromUnicodeToPString (theInfo, theName.length * 2, theName.unicode, fsvol.name);
		if (oserror (errcode)) // 1/29/97 dmb: call oserror
			goto error;
		}
		ix++;
		fsvol.vRefNum = vRefNum;
		
		#else //not carbon
		pb.volumeParam.ioVolIndex = ix++;
		
		pb.volumeParam.ioNamePtr = fsvol.name;
		
		errcode = PBGetVInfoSync (&pb);
		fsvol.vRefNum = pb.volumeParam.ioVRefNum;
		
		if (errcode == nsvErr) /*not an error, just ran out of volumes*/
			return (true);
		#endif//end carbon
		
		if (errcode == nsvErr) /*not an error, just ran out of volumes*/
			return (true);
		
		if (oserror (errcode)) // 1/29/97 dmb: call oserror
			goto error;
		
		fsvol.parID = fsRtParID;
		
		if (!newfilledhandle (&fsvol, sizeof (fsvol), &hstring))
			goto error;
		
		if (!oppushhandle (hlist, nil, (Handle) hstring))
			goto error;

		} /*while*/
	
	error:
	
	opdisposelist (hlist);
	
	disposehandle (*hdiskloop);
	
	*hdiskloop = nil;
	
	return (false);
	} /*diskinitloop*/
	

boolean fileinitloop (const tyfilespec *fs, tyfileloopcallback filefilter, Handle *hfileloop) {
	
	/*
	a conglomeration of filemanager incantations which sets up a UserLand
	fileloop construct.  we create a new handle (allows fileloops to be
	nested inside other file loops, not statically allocated) and fill it
	with information that each iteration of a fileloop will need.  took
	a minimum of two days fiddling (3/16-17/90) to get this to work -- DW.
	
	12/6/91 dmb: generate dirNFErr when fileparsevolname fails
	
	8/15/92 dmb: added, but commented out, filefilter support
	
	2.1b2 dmb: updated to use filespecs
	
	3.0.2 dmb: make sure fs is a folder, not a file
	*/
	
	tyfilelooprecord info;
	CInfoPBRec pb;
	long dirid;
	register hdlfilelooprecord h;
	short ix;
	hdllistrecord hlist;
	
	clearbytes (&pb, sizeof (CInfoPBRec)); /*init all fields to zero*/
	
	setoserrorparam ((ptrstring) (*fs).name);
	
	pb.hFileInfo.ioVRefNum = (*fs).vRefNum;
	
	pb.hFileInfo.ioDirID = (*fs).parID;
	
	pb.hFileInfo.ioNamePtr = (StringPtr) (*fs).name;
	
	if (oserror (PBGetCatInfoSync (&pb)))
		return (false);
	
	if (!foldertest (&pb)) { /*3.0.2*/
		
		oserror (dirNFErr);
		
		return (false);
		}
	
	dirid = pb.dirInfo.ioDrDirID; /*must remember this for loop body*/
	
	clearbytes (&info, sizeof (info));
	
	info.vnum = pb.hFileInfo.ioVRefNum;
	
	info.dirid = pb.hFileInfo.ioDirID;
	
	info.ixdirectory = 1; /*start with the first file*/
	
	if (!newfilledhandle (&info, sizeof (info), hfileloop))
		return (false);
	
	h = (hdlfilelooprecord) *hfileloop; /*copy into register*/
	
	hlist = nil;
	
	if (!opnewlist (&hlist, false)) 
		goto error;
		
	(**h).hfilelist = hlist;
	
	(**hlist).releaseitemcallback = &fileloopreleaseitem;
	
	ix = 1; /*start with file index 1*/
	
	while (true) {
		
		bigstring bsfile;
		Handle hstring;
		OSErr errcode;
		
		pb.dirInfo.ioDrDirID = dirid; /*may be smashed by ioFlNum on previous loop*/
		
		pb.dirInfo.ioFDirIndex = ix++;
		
		pb.dirInfo.ioNamePtr = bsfile;
		
		errcode = PBGetCatInfoSync (&pb);
		
		if (errcode == fnfErr) /*not an error, just ran out of files*/
			return (true);
			
		#if TARGET_API_MAC_CARBON == 1
		
		if (errcode == dirNFErr) /*On OS X, this is what's returned for a read permission error. Weird.*/
			return (true);
				
		#endif
		
		if (oserror (errcode)) /*errcode != noErr*/
			goto error;
		
		/*if it's a folder, insert a colon at the end of the file name*/
		
		if (foldertest (&pb))
			if (!pushchar (':', bsfile))
				goto error;
		
		/*
		if (filefilter != nil) {
			
			if (!(*filefilter) (bspath, bsfile))
				continue;
			}
		*/
		
		if (!newtexthandle (bsfile, &hstring))
			goto error;
		
		if (!oppushhandle (hlist, nil, (Handle) hstring))
			goto error;
		} /*while*/
		
	error:
	
	opdisposelist (hlist);
	
	disposehandle (*hfileloop);
	
	*hfileloop = nil;
	
	return (false);
	} /*fileinitloop*/


void fileendloop (Handle hfileloop) {
	
	register hdlfilelooprecord h = (hdlfilelooprecord) hfileloop;
	
	opdisposelist ((**h).hfilelist);
	
	disposehandle ((Handle) h);
	} /*fileendloop*/


boolean filenextloop (Handle hfileloop, tyfilespec *fsfile, boolean *flfolder) {	
	
	/*
	8/17/92 dmb: we're using text handles now instead of string handles.
	
	2.1b2 dmb: now we're using filespecs!
	
	2.1b11 dmb: for volumes, whole fsspec is now stashed in handle
	
	3.0.2b1 dmb: must set flfolder to true for disks
	
	3.0.4b1 dmb: return true of fnfErr; script will have to handle error anyway
	
	5.0.2b21 dmb: don't call FSMakeFSSpec for each file; just the first.
	*/
	
	register hdlfilelooprecord h = (hdlfilelooprecord) hfileloop;
	Handle hdata;
	bigstring bs;
	short vnum;
	long dirid;
	OSErr err;
	
	if (!opgetlisthandle ((**h).hfilelist, (**h).ixdirectory++, nil, &hdata))
		return (false);
	
	vnum = (**h).vnum;
	
	if (vnum == 0) { /*item is a volume, hdata contains refnum*/
		
		/*
		vnum = **(short **) hdata;
		
		setemptystring (bs);
		*/
		
		*fsfile = **(FSSpec **) hdata;
		
		*flfolder = true; /*3.0.2b1*/
		
		return (true);
		}
	else {
		
		dirid = (**h).dirid;
		
		texthandletostring (hdata, bs);
		
		*flfolder = bs [*bs] == ':';
		
		if (*flfolder)
			--*bs;
		
		if ((**h).ixdirectory == 2) { //first time
			
			err = FSMakeFSSpec (vnum, dirid, bs, fsfile);
			
			(**h).vnum = (*fsfile).vRefNum;
			
			(**h).dirid = (*fsfile).parID;
			
			if ((err == noErr) || (err == fnfErr))	/* 3.0.4b1 dmb */
				return (true);
			
			return (!oserror (err));	/* false */
			}
		else {
			
			(*fsfile).vRefNum = vnum;
			
			(*fsfile).parID = dirid;
			
			copystring (bs, (*fsfile).name);
			
			return (true);
			}
		}
	} /*filenextloop*/


boolean diskloop (tyfileloopcallback diskcallback, long refcon) {
	
	/*
	dmb 9/21/93: filegetvolumeinfo takes a vRefNum, not a string.
	*/
	
	HVolumeParam pb;
	short ix;
	
	clearbytes (&pb, sizeof (pb)); /*init all fields to zero*/
	
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
		
		if (oserror (errcode)) 
			return (false);
		
		filegetvolumeinfo (pb.ioVRefNum, &info);
		
		if (!(*diskcallback) (bsvolume, &info, refcon))
			return (false);
		} /*while*/
	} /*diskloop*/
	
	
boolean folderloop (const tyfilespec *pfs, boolean flreverse, tyfileloopcallback filecallback, long refcon) {
	
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
	
	setoserrorparam ((ptrstring) fs.name);
	
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
			
		if (oserror (ec)) 
			return (false);
		
		filegetinfofrompb (&pb, &info);
		
		if (!(*filecallback) (bsfile, &info, refcon))
			return (false);
		} /*while*/
		
	return (true);
	} /*folderloop*/
#endif




#ifdef WIN95VERSION
// start windows version......
typedef struct tyfindloopinfo
	{
	tyfilespec fs;
	HANDLE findhandle;
	tyfileloopcallback ffilter;
	boolean doingDrives;
	short drivenum;
	} findloopinfo;

static boolean initfileloopspec (tyfilespec *fs)
	{
	HANDLE findHandle;
	WIN32_FIND_DATA	fileinfo;
	char fn[300];
	char * filename;
	short isFolder;

	copystring (fsname (fs), fn);
	filename = fsname (fs);
	
	if (fileisvolume (fs)) {
		cleanendoffilename (fsname (fs));
		appendcstring (filename, "\\*");
		return (true);
		}

	cleanendoffilename (fn);
	nullterminate (fn);
	
	findHandle = FindFirstFile (stringbaseaddress(fn), &fileinfo);

	if (findHandle == INVALID_HANDLE_VALUE)
		{
		oserror (GetLastError());
		return (false);						//no match
		}

	isFolder = (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?true:false;

	if (FindNextFile (findHandle, &fileinfo))
		{
		FindClose (findHandle);
		return (true);						//wild card must be present to find second
		}

	FindClose (findHandle);
	if (isFolder) {
		cleanendoffilename (filename);

		appendcstring (filename, "\\*");

		nullterminate (filename);
		}

	return (true);
	}

boolean fileinitloop (const tyfilespec *pfs, tyfileloopcallback filefilter, Handle *hfileloop) {
	
	findloopinfo ** fi;
	
	*hfileloop = NULL;
	fi = (findloopinfo **) NewHandle (sizeof(findloopinfo));

	if (fi == NULL)
		return (false);

	(**fi).fs = *pfs;
	(**fi).findhandle = NULL;
	(**fi).ffilter = filefilter;
	(**fi).doingDrives = false;
	(**fi).drivenum = 0;		/*start with A*/

	if (isemptystring ((**fi).fs.fullSpecifier)) {	/* do disks */
		(**fi).doingDrives = true;
		}
	else {
		initfileloopspec (&((**fi).fs));
		}

	*hfileloop = (Handle)fi;
	return (true);
	}

void fileendloop (Handle hfileloop) {
	
	findloopinfo ** fi;
	fi = (findloopinfo **) hfileloop;

	if (fi != NULL)
		{
		if ((**fi).findhandle != NULL)
			FindClose ((**fi).findhandle);
	
		disposehandle ((Handle) fi);
		}
	} /*fileendloop*/


boolean filenextloop (Handle hfileloop, tyfilespec *fsfile, boolean *flfolder) {	
	findloopinfo ** fi;
	HANDLE findHandle;
	WIN32_FIND_DATA	fileinfo;
	char fn[300];
	char pathname[300];
	char * endofpath;
	int errCode;
	short drivenum;
	DWORD drivemap, drivemask;

	if (hfileloop == NULL)
		return (false);

	if (fsfile == NULL)
		return (false);

	fi = (findloopinfo **) hfileloop;

	if ((**fi).doingDrives) {				/*We are looping over each volume - not files*/
		while (true) {
			drivenum = (**fi).drivenum;

			++((**fi).drivenum);

			if (drivenum >= 26)
				return (false);

			drivemap = GetLogicalDrives();
		
			drivemask = 1 << drivenum;

			if ((drivemap & drivemask) == drivemask) { /* we found one */
				/*convert drivenum to filespec */
				wsprintf (stringbaseaddress(fsname(fsfile)), "%c:\\", drivenum + 'A');
				setstringlength (fsname(fsfile), strlen(stringbaseaddress(fsname(fsfile))));
				*flfolder = true;
				return (true);
				}
			}
		}

	if ((**fi).findhandle == NULL)			/*we are looking for the first one...*/
		{
		copystring (fsname (&(**fi).fs), fn);

		cleanendoffilename (fn);

		nullterminate (fn);
		
		findHandle = FindFirstFile (stringbaseaddress(fn), &fileinfo);

		if (findHandle == INVALID_HANDLE_VALUE)
			{
			oserror (GetLastError());
			return (false);
			}

		GetFullPathName (stringbaseaddress(fn), 300, stringbaseaddress(pathname), &endofpath);

		*endofpath = 0;  /*terminates path name*/

		setstringlength(pathname, strlen(stringbaseaddress(pathname)));

		copystring (pathname, fsname (&(**fi).fs));
		buildfilename (fsname (&(**fi).fs), 0, fileinfo.cFileName, 1, fsname (fsfile));

		if (flfolder != NULL)
			*flfolder = (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?true:false;

		(**fi).findhandle = findHandle;

		if (strcmp (fileinfo.cFileName, ".") == 0)
			goto SkippingDotandDotDot;

		if (strcmp (fileinfo.cFileName, "..") == 0)
			goto SkippingDotandDotDot;

		if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			cleanendoffilename (fsname(fsfile));
			appendcstring (fsname(fsfile), "\\");
			}

		return (true);
		}
	else
		{
SkippingDotandDotDot:
		if (FindNextFile ((**fi).findhandle, &fileinfo))
			{
			buildfilename (fsname (&(**fi).fs), 0, fileinfo.cFileName, 1, fsname (fsfile));

			if (flfolder != NULL)
				*flfolder = (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?true:false;

			if (strcmp (fileinfo.cFileName, ".") == 0)
				goto SkippingDotandDotDot;

			if (strcmp (fileinfo.cFileName, "..") == 0)
				goto SkippingDotandDotDot;

			if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				cleanendoffilename (fsname(fsfile));
				appendcstring (fsname(fsfile), "\\");
				}

			return (true);
			}

		errCode = GetLastError();

//		FindClose((**fi).findhandle);

		if (errCode != ERROR_NO_MORE_FILES)
			oserror(errCode);
		}

	return (false);
	}


boolean folderloop (const tyfilespec *pfs, boolean flreverse, tyfileloopcallback filecallback, long refcon) {
	HANDLE findHandle;
	WIN32_FIND_DATA	fileinfo;
	char fn[300];
	char pathname[300];
	char * endofpath;
	bigstring bsfile;
	tyfileinfo info;
	int errCode;


	copystring (fsname (pfs), fn);

	cleanendoffilename (fn);

	nullterminate (fn);
	
	findHandle = FindFirstFile (stringbaseaddress(fn), &fileinfo);

	if (findHandle == INVALID_HANDLE_VALUE)
		{
		oserror (GetLastError());
		return (false);
		}

	winsetfileinfo (&fileinfo, &info);

	GetFullPathName (stringbaseaddress(fn), 300, stringbaseaddress(pathname), &endofpath);

	*endofpath = 0;  /*terminates path name*/

	setstringlength(pathname, strlen(stringbaseaddress(pathname)));

	buildfilename (pathname, 0, fileinfo.cFileName, 1, bsfile);

	if (!(*filecallback) (bsfile, &info, refcon))
		{
		FindClose(findHandle);
		return (false);
		}

	while (FindNextFile (findHandle, &fileinfo))
		{
		winsetfileinfo (&fileinfo, &info);

		buildfilename (pathname, 0, fileinfo.cFileName, 1, bsfile);

		if (!(*filecallback) (bsfile, &info, refcon))
			{
			FindClose(findHandle);
			return (false);
			}
		}

	errCode = GetLastError();

	FindClose(findHandle);

	if (errCode == ERROR_NO_MORE_FILES)
		return (true);

	oserror(errCode);
	return (false);
	}

#endif

