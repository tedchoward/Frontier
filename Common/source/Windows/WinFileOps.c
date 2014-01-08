
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

#include "winregistry.h"

#include "filealias.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "timedate.h"
#include "frontierwindows.h"
#include "file.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "langinternal.h" /*for langbackgroundtask*/

static short applicationresnum = -1;

typedef struct tyfileinfo tyvolinfo;

void setfserrorparam ( const ptrfilespec fs ) {
	
	setoserrorparam((ptrstring) fsname(fs));
	
} // setfserrorparam

boolean endswithpathsep (bigstring bs) {
	/* return true if the ending character is a path sepatator character*/
	char ch;
	
	ch = getstringcharacter(bs, stringlength(bs)-1);
		
	if (ch == '\\')
		return (true);
	
	if (ch == '/')
		return (true);
	
	return (false);
} /*endswithpathsep*/

boolean cleanendoffilename (bigstring bs) {
	if (endswithpathsep(bs)) {
		setstringlength (bs, stringlength(bs) - 1);
		return (true);
	}
	
	return (false);
} /*cleanendoffilename*/

boolean getmachinename (bigstring bsname) {
	
	//
	// 2006-05-15 creedon: fix to work on Mac OS X
	//
	
	boolean fl;
	
	DWORD len;
	
	len = sizeof (bigstring) - 2;
	
	fl = GetComputerName (stringbaseaddress (bsname), &len);
	
	if (fl)
		setstringlength (bsname, len);
	else
		setemptystring (bsname);
	
	return (fl);
	
} // getmachinename

boolean winfileerror (const ptrfilespec fs) {
	
	DWORD err = GetLastError ();
	
	setfserrorparam (fs);
	
	return (oserror (err));
} /*winfileerror*/

static boolean filegetfsvolumeinfo (const ptrfilespec fs, tyfileinfo *info) {
	
	DWORD sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;
	UINT drivetype;
	bigstring volname;
	bigstring errmsg;
	
	clearbytes (info, sizeof (tyfileinfo)); // init all fields to zero
	
	
	if (fileisvolume (fs)) {
		info->flvolume = true;
		info->flbusy = true;
		
		copystring (fsname(fs), volname);
		cleanendoffilename (volname);
		pushchar ('\\', volname);
		nullterminate (volname);
		
		//		if (GetVolumeInformation (stringbaseaddress (volname), stringbaseaddress(volnamebuf), 
		//			sizeof(volnamebuf)-2, &volserial, &maxfilelen, &filesystemflags, 
		//			stringbaseaddress(filesystemnamebuffer), sizeof (filesystemnamebuffer)) {
		//				
		//			}
		
		if (GetDiskFreeSpace (stringbaseaddress (volname), &sectorsPerCluster, &bytesPerSector,
							  &numberOfFreeClusters, &totalNumberOfClusters)) {
			
			DWORD bytesPerCluster;
			DWORD numberOfClustersIn2GB;
			
			bytesPerCluster = bytesPerSector * sectorsPerCluster;
			
			numberOfClustersIn2GB = 0x7FFFFFFFL / bytesPerCluster;
			
			info->ctfreebytes = (numberOfFreeClusters <= numberOfClustersIn2GB)
			? (numberOfFreeClusters * bytesPerCluster)
			: (numberOfClustersIn2GB * bytesPerCluster);
			
			info->cttotalbytes = (totalNumberOfClusters <= numberOfClustersIn2GB)
			? (totalNumberOfClusters * bytesPerCluster)
			: (numberOfClustersIn2GB * bytesPerCluster);
			
			info->blocksize = bytesPerCluster;
		}
		
		drivetype = GetDriveType (stringbaseaddress(volname));
		
		if ((drivetype == DRIVE_REMOVABLE) || (drivetype == DRIVE_CDROM)) {
			info->flejectable = true;
		}
		
		if (drivetype == DRIVE_REMOTE) {
			info->flremotevolume = true;
		}
		
		return (true);
	}
	
	wsprintf (stringbaseaddress (errmsg), "Can't complete function because \"%s\" is not a valid volume name.",
			  stringbaseaddress (fsname (fs)));
	
	setstringlength (errmsg, strlen (stringbaseaddress (errmsg)));
	
	shellerrormessage (errmsg);
	
	return (false);
	
} // filegetfsvolumeinfo

void winsetfileinfo (WIN32_FIND_DATA * fileinfo, tyfileinfo *info) {
	
	FILETIME localFileTime;
	bigstring bs, bsext;
	
	clearbytes (info, sizeof (tyfileinfo)); /*init all fields to zero*/
	info->filecreator = '    ';		
	info->filetype = '    ';
	
	
	info->flfolder = fileinfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY?true:false;
	info->fllocked = fileinfo->dwFileAttributes & FILE_ATTRIBUTE_READONLY?true:false;
	info->flbundle = false;
	info->flbusy = false;
	info->flalias = false;
	info->flinvisible = fileinfo->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN?true:false;
	info->flsystem = fileinfo->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM?true:false;
	info->flarchive = fileinfo->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE?true:false;
	info->flcompressed = fileinfo->dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED?true:false;
	info->fltemp = fileinfo->dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY?true:false;
	
	FileTimeToLocalFileTime (&fileinfo->ftCreationTime, &localFileTime);
	info->timecreated = filetimetoseconds (&localFileTime);
	
	FileTimeToLocalFileTime (&fileinfo->ftLastWriteTime, &localFileTime);
	info->timemodified = filetimetoseconds (&localFileTime);
	
	FileTimeToLocalFileTime (&fileinfo->ftLastAccessTime, &localFileTime);
	info->timeaccessed = filetimetoseconds (&localFileTime);
	
	info->sizedatafork = fileinfo->nFileSizeLow;
	info->sizeresourcefork = 0;
	info->sizedataforkhigh = fileinfo->nFileSizeHigh;
	
	copyctopstring (fileinfo->cFileName, bs);
	
	lastword (bs, '.', bsext);
	
	if ((stringlength (bs) == stringlength (bsext)) || (stringlength (bsext) > 4)) {	/* no extension */
		stringtoostype ("\x04" "????", &info->filetype);
	}
	else {
		stringtoostype (bsext, &info->filetype);
	}
	
} /*winsetfileinfo*/

boolean filegetinfo (const ptrfilespec fs, tyfileinfo *info) {
	
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	// 1993-09-24 dmb: for Mac, let filegetinfofrompb take care of volumes
	//
		
	
	HANDLE findHandle;
	HANDLE fref;
	WIN32_FIND_DATA	fileinfo;
	char fn[300];
	
	clearbytes (info, sizeof (tyfileinfo)); // init all fields to zero
	info->filecreator = '    ';		
	info->filetype = '    ';
	
	if (fileisvolume (fs)) {
		return (filegetfsvolumeinfo (fs, info));
	}
	
	copystring (fsname (fs), fn);
	
	// if ends with \ get ride of it... and handle the root
	
	cleanendoffilename (fn);
	
	nullterminate (fn);
	
	
	findHandle = FindFirstFile (stringbaseaddress(fn), &fileinfo);
	
	if (findHandle == INVALID_HANDLE_VALUE) {
		
		winfileerror (fs);
		
		return (false);
	}
	
	winsetfileinfo (&fileinfo, info);
	
	FindClose(findHandle);
	
	// Set the file busy flag (this should be in winsetfileinfo, but we do not have the filename there)
	
	if (info->flfolder) {
		strcat (stringbaseaddress (fn), "\\*");
		
		info->flbusy = false; //presume empty folder (not busy)
		
		findHandle = FindFirstFile (stringbaseaddress(fn), &fileinfo);
		
		if (findHandle != INVALID_HANDLE_VALUE) {
			info->flbusy = true;				// Found something
			
			while ((strcmp (fileinfo.cFileName, ".") == 0) || (strcmp (fileinfo.cFileName, "..") == 0)) {
				info->flbusy = false;			// Just . or ..
				
				if (FindNextFile (findHandle, &fileinfo))
					info->flbusy = true;		// Found something else...
				else
					break;		// exit while loop if FindNext fails (this is normal)
			}
			
			FindClose (findHandle);
		}
	}
	else {
		fref = (Handle) CreateFile (stringbaseaddress (fn), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (fref == INVALID_HANDLE_VALUE) {
			info->flbusy = true;
		}
		else {
			info->flbusy = false;
			verify (CloseHandle (fref));
		}
	}
	
	return (true);
	
} // filegetinfo

boolean filegetvolumename (short vnum, bigstring volname) {
	char szVol[30];
	DWORD serialNum, maxComponentLen, fileSystemFlags;
	boolean res;
	
	if ((vnum > 26) || (vnum < 0))
		return (false);
	
	if (vnum != 0) {
		strcpy (szVol, "A:\\");
		szVol[0] = szVol[0] + (vnum - 1);
	}
	
	res = GetVolumeInformation (vnum==0?NULL:szVol, stringbaseaddress(volname), sizeof(volname)-2,
								&serialNum, &maxComponentLen, &fileSystemFlags, NULL, 0);
	
	if (res) {
		setstringlength (volname, strlen(stringbaseaddress(volname)));
	}
	
	return (res);
} /*filegetvolumename*/

boolean fileisbusy (const ptrfilespec fs, boolean *flbusy) {
	
	/*
	 6/x/91 mao
	 */
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flbusy = info.flbusy;
	
	return (true);
} /*fileisbusy*/

boolean filehasbundle (const ptrfilespec fs, boolean *flbundle) {
	
	/*
	 6/x/91 mao
	 */
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flbundle = info.flbundle;
	
	return (true);
} /*filehasbundle*/

boolean filesetbundle (const ptrfilespec fs, boolean flbundle) {
	return (false);	
} // filesetbundle

boolean fileisalias (const ptrfilespec fs, boolean *flalias) {
	
	/*
	 6/x/91 mao
	 */
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flalias = info.flalias;
	
	return (true);
} /*fileisalias*/

boolean fileisvisible (const ptrfilespec fs, boolean *flvisible) {
	
	/*
	 6/9/92 dmb
	 */
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flvisible = !info.flinvisible;
	
	return (true);
} /*fileisvisible*/

boolean filesetvisible (const ptrfilespec fs, boolean flvisible) {
	
	//
	// 2006-06-13 creedon: for Mac, FSRef-ized
	//
	// 6/9/92 dmb: for Mac, created
	//
	
	tyfileinfo info;
	DWORD attr;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	if (info.flinvisible == flvisible) // check if we need to do anything first
	{
		attr = GetFileAttributes (stringbaseaddress (fsname (fs)));
		
		if (attr == 0xFFFFFFFF)
			return (false);
		
		attr = attr & (~FILE_ATTRIBUTE_HIDDEN);
		
		if (! flvisible)
			attr = attr | FILE_ATTRIBUTE_HIDDEN;
		
		return(SetFileAttributes (stringbaseaddress (fsname (fs)), attr));
	}
	
	return (true);
	
} // filesetvisible

boolean getfiletype (const ptrfilespec fs, OSType *type) {
	
	tyfileinfo info;
	
	*type = '    ';
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*type = info.filetype;
	
	return (true);
} /*getfiletype*/

boolean getfilecreator (const ptrfilespec fs, OSType *creator) {
	
	tyfileinfo info;
	
	*creator = '    ';
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*creator = info.filecreator;
	
	return (true);
} /*getfilecreator*/

boolean filesize (const ptrfilespec fs, long *size) {
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*size = info.sizedatafork + info.sizeresourcefork;
	
	return (true);
} /*filesize*/

boolean fileisfolder (const ptrfilespec fs, boolean *flfolder) {
	
	//
	// 2006-10-01 creedon: Mac OS X bundles/packages are not considered folders
	//
	
	tyfileinfo info;
	
	// special case the root directory on Windows
	
	char fn[300];
	
	copystring (fsname (fs), fn);
	
	if (fn[2] == ':' && isalpha(fn[1])) {
		
		if ((stringlength(fn) == 2) || ((stringlength(fn) == 3) && (fn[3] == '\\'))) {
			
			if (fileisvolume (fs)) {
				
				*flfolder = true;
				
				return (true);
			}
		}
	}
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flfolder = info.flfolder;
	
	return (true);
	
} // fileisfolder

boolean fileisvolume (const ptrfilespec fs) {
	
	bigstring bsvol;
	short drivenum;
	DWORD drivemap, drivemask;
	
	copystring (fsname(fs), bsvol);
	
	switch (stringlength (bsvol)) {
			
		case 3:
			if (getstringcharacter(bsvol, 2) != '\\')
				return (false);
			
		case 2:
			if (getstringcharacter(bsvol, 1) != ':')
				return (false);
			
			if (! isalpha (getstringcharacter(bsvol, 0)))
				return (false);
			
			drivenum = getlower(getstringcharacter(bsvol, 0)) - 'a';
			break;
			
		default:
			return (false);
	}
	
	drivemap = GetLogicalDrives();
	
	drivemask = 1 << drivenum;
	
	return ((drivemap & drivemask) == drivemask);
	
} // fileisvolume

boolean fileislocked (const ptrfilespec fs, boolean *fllocked) {
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*fllocked = info.fllocked;
	
	return (true);
} /*fileislocked*/

boolean getfiledates (const ptrfilespec fs, unsigned long *datecreated, unsigned long *datemodified) {
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*datecreated = info.timecreated;
	
	*datemodified = info.timemodified;
	
	return (true);
} /*getfiledates*/

boolean setfiledates (const ptrfilespec fs, unsigned long datecreated, unsigned long datemodified) {
	
	//
	// 2006-06-25 creedon: for Mac, work with UTC
	//
	// 1991-06 mao: modified to work with new getmacilfeinfo/setmacfileinfo.
	//
	
	HANDLE h;
	FILETIME modtime, createtime, temp;
	char fn[300];
	DWORD err;
	boolean fl;
	
	secondstofiletime (datecreated, &temp);
	LocalFileTimeToFileTime (&temp, &createtime);
	
	secondstofiletime (datemodified, &temp);
	LocalFileTimeToFileTime (&temp, &modtime);
	
	copystring (fsname (fs), fn);
	
	nullterminate (fn);
	
	h = CreateFile (stringbaseaddress(fn), GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (h == INVALID_HANDLE_VALUE) {
		
		winfileerror (fs);
		
		return (false);
	}
	
	
	//We do not alter the accessed time.
	fl = SetFileTime (h, &createtime, NULL, &modtime);
	
	if (!fl)
		err = GetLastError();
	
	verify (CloseHandle (h));
	
	if (fl)
		return (true);
	
	oserror(err);
	return (false);
	
} /*setfiledates*/

boolean setfilecreated (const ptrfilespec fs, long when) {
	
	/*
	 7/31/91 dmb: created
	 */
	
	unsigned long datecreated, datemodified;
	
	if (!getfiledates (fs, &datecreated, &datemodified))
		return (false);
	
	return (setfiledates (fs, when, datemodified));
} /*setfilecreated*/

boolean setfilemodified (const ptrfilespec fs, long when) {
	
	/*
	 7/31/91 dmb: created
	 */
	
	unsigned long datecreated, datemodified;
	
	if (!getfiledates (fs, &datecreated, &datemodified))
		return (false);
	
	return (setfiledates (fs, datecreated, when));
} /*setfilemodified*/

boolean largefilebuffer (Handle *hbuffer) {
	
	/*
	 allocate a "large" buffer for a file copy or some kind of transfer.
	 
	 we ask for a block the size of half of the current free space.  if we can't 
	 get it, we'll ask for the largest free block.  in any case, we fail if 
	 we can't get a block of at least a K
	 
	 3/16/92 dmb: total rewrite; old version _always_ compacted the heap, 
	 resulting in poor performance.
	 */
	
	register long ctbytes;
	register Handle h;
	
	*hbuffer = nil; /*default return*/
		
	DBG_UNREFERENCED_LOCAL_VARIABLE(h);
	
	ctbytes = 0x00020000L;		/* 128K buffer */
	
	ctbytes &= ~(1024 - 1); /*fast round down to nearest multiple of 1024*/
	
	return (newhandle (ctbytes, hbuffer)); /*shouldn't fail at this point, but let's be sure*/
} /*largefilebuffer*/

boolean copyfile ( const ptrfilespec fsource, const ptrfilespec fdest, boolean fldata, boolean flresources ) {
	
	//
	// create a copy of the indicated file in the destination folder or volume, with the indicated name.
	//
	// 2006-06-18 creedon:	for Mac, FSRef-ized
	//				combined the two definitions (Mac and Win) of this function into one, deleted old code, see
	//				revision 1329, use FSCopyObject code
	//
	// 5.0.1 dmb: for Windows, if the file exists, overwrite it.
	//
	
	char fn[300];
	char fn2[300];
	boolean flcreatefolder;
	boolean fldestfolder;
	
	copystring (fsname (fsource), fn);
	copystring (fsname (fdest), fn2);
	
	cleanendoffilename (fn);
	cleanendoffilename (fn2);
	
	nullterminate (fn);
	nullterminate (fn2);
	
	if (!fileisfolder (fsource, &flcreatefolder))
		return (false);
	
	if (flcreatefolder) {
		
		if (fileexists (fdest, &fldestfolder)) { /*file exists, delete it*/
			
			if (!deletefile (fdest))
				return (false);
		}
		
		if (CreateDirectoryEx (stringbaseaddress(fn), stringbaseaddress(fn2), NULL))
			return (true);
		
		winfileerror (fdest);
		
		return (false);
	}
	
	if (CopyFile (stringbaseaddress (fn), stringbaseaddress(fn2), false))
		return (true);
	
	winfileerror (fdest);
	
	return (false);
	
} // copyfile

short filegetapplicationrnum (void) {
	
	return (applicationresnum);
} /*filegetapplicationrnum*/

boolean getspecialfolderpath ( bigstring bsvol, bigstring bsfolder, boolean flcreate, ptrfilespec fs ) {
	
	//
	// 2006-08-24 creedon: for Mac, FSRef-ized
	//
	// 2006-04-11 creedon: windows now honors flcreate
	//
	//			       for windows added; CSIDL_PROGRAM_FILES, CSIDL_MYDOCUMENTS, CSIDL_MYMUSIC,
	//			       CSIDL_MYPICTURES, CSIDL_MYVIDEO
	//
	//			       for windows replaced more complex code with CSIDL_SYSTEM, CSIDL_WINDOWS
	//
	// 1992-09-01 dmb: last new verb for 2.0.  (?)
	//
		
	
	int nFolder = -1;
	// int res;
	ITEMIDLIST * il;
	// Global pointer to the shell's IMalloc interface.  
	LPMALLOC g_pMalloc = NULL;  
	
	/* if (equalidentifiers ("\x6" "system", bsfolder)) {
	 unsigned char str[MAX_PATH];
	 
	 res = GetSystemDirectory (str, MAX_PATH);
	 if (res == 0) {
	 return (! oserror(GetLastError()));
	 }
	 
	 if (res > 255)
	 res = 255;
	 
	 memmove (stringbaseaddress(fsname(fs)), str, res);
	 setstringlength(fsname(fs), res);
	 pushchar ('\\', fsname(fs));
	 nullterminate (fsname(fs));
	 return (true);
	 }
	 
	 if (equalidentifiers ("\x7" "windows", bsfolder)) {
	 unsigned char str[MAX_PATH];
	 
	 res = GetWindowsDirectory (str, MAX_PATH);
	 if (res == 0) {
	 return (! oserror(GetLastError()));
	 }
	 
	 if (res > 255)
	 res = 255;
	 
	 memmove (stringbaseaddress(fsname(fs)), str, res);
	 setstringlength(fsname(fs), res);
	 pushchar ('\\', fsname(fs));
	 nullterminate (fsname(fs));
	 return (true);
	 } */
	
	if (equalidentifiers ("\x9" "bitbucket", bsfolder))
		nFolder = CSIDL_BITBUCKET;
	else if (equalidentifiers ("\x8" "controls", bsfolder))
		nFolder = CSIDL_CONTROLS;
	else if (equalidentifiers ("\x7" "desktop", bsfolder))
		nFolder = CSIDL_DESKTOP;
	else if (equalidentifiers ("\x10" "desktopdirectory", bsfolder))
		nFolder = CSIDL_DESKTOPDIRECTORY;
	else if (equalidentifiers ("\x6" "drives", bsfolder))
		nFolder = CSIDL_DRIVES;
	else if (equalidentifiers ("\x5" "fonts", bsfolder))
		nFolder = CSIDL_FONTS;
	else if (equalidentifiers ("\x7" "nethood", bsfolder))
		nFolder = CSIDL_NETHOOD;
	else if (equalidentifiers ("\x7" "network", bsfolder))
		nFolder = CSIDL_NETWORK;
	else if (equalidentifiers ("\x8" "personal", bsfolder))
		nFolder = CSIDL_PERSONAL;
	else if (equalidentifiers ("\x8" "printers", bsfolder))
		nFolder = CSIDL_PRINTERS;
	else if (equalidentifiers ("\x8" "programs", bsfolder))
		nFolder = CSIDL_PROGRAMS;
	else if (equalidentifiers ("\x6" "recent", bsfolder))
		nFolder = CSIDL_RECENT;
	else if (equalidentifiers ("\x6" "sendto", bsfolder))
		nFolder = CSIDL_SENDTO;
	else if (equalidentifiers ("\x9" "startmenu", bsfolder))
		nFolder = CSIDL_STARTMENU;
	else if (equalidentifiers ("\x7" "startup", bsfolder))
		nFolder = CSIDL_STARTUP;
	else if (equalidentifiers ("\x9" "templates", bsfolder))
		nFolder = CSIDL_TEMPLATES;
	else if (equalidentifiers ("\xD" "program files", bsfolder))
		nFolder = CSIDL_PROGRAM_FILES;
	else if (equalidentifiers ("\x6" "system", bsfolder))
		nFolder = CSIDL_SYSTEM;
	else if (equalidentifiers ("\x7" "windows", bsfolder))
		nFolder = CSIDL_WINDOWS;
	else if (equalidentifiers ("\xC" "my documents", bsfolder))
		nFolder = CSIDL_MYDOCUMENTS;
	else if (equalidentifiers ("\x8" "my music", bsfolder))
		nFolder = CSIDL_MYMUSIC;
	else if (equalidentifiers ("\xB" "my pictures", bsfolder))
		nFolder = CSIDL_MYPICTURES;
	else if (equalidentifiers ("\x8" "my video", bsfolder))
		nFolder = CSIDL_MYVIDEO;
	
	if (flcreate)
		nFolder += CSIDL_FLAG_CREATE;
	
	if (nFolder != -1) {
		// Get the shell's allocator. 
		if (!SUCCEEDED(SHGetMalloc(&g_pMalloc))) {
			oserror (ERROR_INVALID_FUNCTION);
			return (false); 
		}
		
		if (SHGetSpecialFolderLocation (NULL, nFolder, &il) == NOERROR) {
			/*process itemlist */
			if (SHGetPathFromIDList (il, stringbaseaddress(fsname(fs)))) {
				setstringlength (fsname(fs), strlen(stringbaseaddress(fsname(fs))));
				pushchar ('\\', fsname(fs));
				nullterminate (fsname(fs));
				
				// Free the PIDL 
				g_pMalloc->lpVtbl->Free(g_pMalloc, il); 
				
				// Release the shell's allocator. 
				g_pMalloc->lpVtbl->Release(g_pMalloc); 
				return (true);
			}
		}
	}
	
	// Release the shell's allocator. mnbmj
	if (g_pMalloc != NULL)
		g_pMalloc->lpVtbl->Release(g_pMalloc); 
	
	oserror (ERROR_INVALID_FUNCTION);
	return (false);
	
} // getspecialfolderpath

boolean isejectable (const ptrfilespec fs, boolean *flejectable) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*flejectable = volinfo.flejectable;
	
	return (true);
} /*isejectable*/

boolean getfreespace (const ptrfilespec fs, long *ctfreebytes) {
	
	/*
	 6/x/91 mao
	 */
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*ctfreebytes = volinfo.ctfreebytes;
	
	return (true);
} /*getfreespace*/

static void volumeinfoerror (OSErr errnum) {
	
	bigstring bserr, bserrnum;
	
	copystring (BIGSTRING ("\x42" "Can't get volume information because an error of type ^0 occurred."), bserr);
	
	numbertostring (errnum, bserrnum);
	
	parsedialogstring (bserr, bserrnum, nil, nil, nil, bserr);
	
	shellerrormessage (bserr);
}/*volumeinfoerror*/

boolean langgetextendedvolumeinfo (const ptrfilespec fs, double *totalbytes, double *freebytes) {
	
	//
	// This code is supposed to work flawlessly with volumes over 2 GB.
	// 
	// For the Mac version, we delegate the actual work to XGetVInfo in
	// MoreFilesExtras.h.
	// 
	// For the Windows version, we use GetFreeDiskSpaceEx if available. Otherwise
	// we fall back to GetFreeDiskSpace. According to MSDN info that's probably
	// neccessary if we run on Win95 Release 1 which didn't support volume sizes
	// over 2 GB.
	//
	// flsupportslargevolumes is initialized in filestartup in fileverbs.c. (Win32 only)
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	// 6.1b16 AR: Return number of free bytes and total number of bytes for the volume.
	//
	
	
	bigstring volname;
	
	*totalbytes = 0.0;
	
	*freebytes = 0.0;
	
	getfsfile ( fs, volname);
	
	cleanendoffilename (volname);
	
	pushchar ('\\', volname);
	
	nullterminate (volname);
	
	if (flsupportslargevolumes) {
		
		ULARGE_INTEGER freeBytesAvailableToCaller, totalNumberOfBytes, totalNumberOfFreeBytes;
		
		if ((*adrGetDiskFreeSpaceEx) ((unsigned short *) stringbaseaddress (volname), &freeBytesAvailableToCaller,
									  &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
			
			*totalbytes = (double) (LONGLONG) totalNumberOfBytes.QuadPart;
			
			*freebytes = (double) (LONGLONG) freeBytesAvailableToCaller.QuadPart;
		}
		
		else
			if (GetLastError () == ERROR_CALL_NOT_IMPLEMENTED)
				flsupportslargevolumes = false;
	}
	
	if (!flsupportslargevolumes) {
		
		DWORD sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;
		
		if (GetDiskFreeSpace (stringbaseaddress (volname), &sectorsPerCluster, &bytesPerSector,
							  &numberOfFreeClusters, &totalNumberOfClusters)) {
			
			double bytespercluster = (double) sectorsPerCluster * (double) bytesPerSector;
			
			*totalbytes = bytespercluster * (double) totalNumberOfClusters;
			
			*freebytes = bytespercluster * (double) numberOfFreeClusters;
		}
	}
	
	return (true);
}/*getextendedvolumeinfo*/

boolean getvolumesize (const ptrfilespec fs, long *ctbytes) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*ctbytes = volinfo.cttotalbytes;
	
	return (true);
} /*getvolumesize*/

boolean getvolumeblocksize (const ptrfilespec fs, long *ctbytes) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*ctbytes = volinfo.blocksize;
	
	return (true);
} /*getvolumeblocksize*/

boolean filesonvolume ( const ptrfilespec fs, long *ctfiles ) {
	
	//
	// 6/x/91 mao
	//
	
	tyvolinfo volinfo;
	
	if ( ! filegetfsvolumeinfo ( fs, &volinfo ) )
		return ( false );
	
	*ctfiles = volinfo.ctfiles;
	
	return ( true );
	
} // filesonvolume

boolean foldersonvolume (const ptrfilespec fs, long *ctfolders) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*ctfolders = volinfo.ctfolders;
	
	return (true);
} /*foldersonvolume*/

boolean isvolumelocked (const ptrfilespec fs, boolean *fllocked) {
	
	/*
	 6/x/91 mao
	 */
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*fllocked = volinfo.fllocked;
	
	return (true);
} /*isvolumelocked*/

boolean volumecreated (const ptrfilespec fs, unsigned long *createdate) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*createdate = volinfo.timecreated;
	
	return (true);
} /*volumecreated*/

boolean lockvolume (const ptrfilespec fs, boolean fllock) {
	return (false);
} /*lockvolume*/

boolean fileparsevolname ( bigstring bspath, ptrfilespec fs ) {
	
	//
	// convert a full path, which might contain a volume name at the beginning
	// to a path with no volume name, and it's associated volume number in vnum.
	//
	// example: "Rover™:MORE Work" will return with bspath = "MORE Work" and
	// vnum = -2 (the Macintosh vrefnum for the second mounted drive).  
	//
	// this combination of information plugs nicely into a lot of the file
	// manager routines.
	//
	// 2006-10-16 creedon: for Mac, FSRef-ized
	//
	// 5.0.2 rab: initialize ix to zero
	//
	// 2.1b11 dmb: return the vol name in bsvol if non-nil, along with the vnum
	//
	// 2.1b8 dmb: handle drive numbers
	//
	
	short ix = 0;
	
	if ( isemptystring ( bspath ) )
		return ( false );
	
	if ( ! scanstring ( ':', bspath, &ix ) ) // no colon, No volume?
		return ( false );
	else
		midstring ( bspath, 1, ix, ( *fs ).fullSpecifier ); // pick off the vol name and the colon
	
	return ( true );
	
} // fileparsevolname

boolean filefrompath (bigstring path, bigstring fname) {
	
	/*
	 return all the characters to the right of the colon in the path.
	 
	 example: "Work Disk #1:MORE Work:Status Center" returns "Status Center".
	 */
	
	char fn[300];
	char * fileptr;
	tyfilespec fs;
	
	copystring (path, fsname (&fs));
	
	if (stringlength (path) >= 255) {		/* RAB 4/27/98 fix for to long a filename conversion attempt*/
		setemptystring (fname);
		return (true);
	}
	
	if (isemptystring (path) || fileisvolume (&fs)) {
		
		copystring (path, fname);
		
		return (true);
	}
	
	nullterminate (path);
	
	GetFullPathName (stringbaseaddress(path), 298, fn, &fileptr);	
	
	/*the comparison of fileptr within the legal range of fn is a kludge
	 but if fixes GetFullPathName which goes wild on some strings and 
	 there is no other way of determining that it has gone wild.
	 RAB 4/27/98 */
	
	if ((fileptr == NULL) || (fileptr < fn) || (fileptr > (fn + 298)))
		setemptystring (fname);
	else
		copyctopstring (fileptr, fname);
	
	//	strcpy (stringbaseaddress(fname), fileptr);
	//	setstringlength (fname, strlen (fileptr));
	
	return (true);
} /*filefrompath*/

boolean folderfrompath (bigstring path, bigstring folder) {
	
	/*
	 return all the characters to the left of the colon, and the colon.
	 
	 example: "Work Disk #1:MORE Work:Status Center" returns "Work Disk #1:MORE Work:".
	 
	 5.0.2 dmb: call filefrompath instead of lastword to maintain symmetry on Win
	 */
	
	bigstring bs;
	
	filefrompath (path, bs); /*kind of inefficient, but ensures symmetry*/
	
	copystring (path, folder);
	
	setstringlength (folder, stringlength (folder) - stringlength (bs));
	
	return (true);
} /*folderfrompath*/

boolean movefile (const ptrfilespec fs, const ptrfilespec fsto) {
	
	char fn1[300];
	char fn2[300];
	boolean fl;
	bigstring filename;
	
	copystring (fsname (fs), fn1);
	copystring (fsname (fsto), fn2);
	
	if (endswithpathsep (fn1))
		setstringlength (fn1, stringlength (fn1) - 1);
	
	nullterminate (fn1);
	nullterminate (fn2);
	
	if (!fileisfolder (fsto, &fl))
		return (false);
	
	if (fl) {
		filefrompath (fn1, filename);
		
		cleanendoffilename(fn2);
		
		appendcstring (fn2, "\\");
		
		pushstring (filename, fn2);
		
		nullterminate (fn2);
	}
	
	if (MoveFile (stringbaseaddress (fn1), stringbaseaddress(fn2)))
		return (true);
	
	winfileerror (fs);
	
	return (false);
	
} /*movefile*/

void filenotfounderror (bigstring bs) {
	
	/*
	 generate a "file not found" or a "folder not found" error
	 */
	
	OSErr errcode;
	
	setoserrorparam (bs);
	
	errcode = errorFileNotFound;
	
	oserror (errcode);
} /*filenotfounderror*/

boolean surefile (const ptrfilespec fs) {
	
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	
	boolean flfolder;
	bigstring bs;
	
	if (fileexists (fs, &flfolder))
		return (true);
	
	getfsfile ( fs, bs );
	
	filenotfounderror ( bs );
	
	return (false);
	
} // surefile

boolean renamefile (const ptrfilespec fs, bigstring bsnew) {
	
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	
	char fn [300];
	bigstring bsfolder;
	
	if (!surefile (fs)) /*file doesn't exist -- catch error*/
		return (false);
	
	filefrompath (bsnew, bsnew);
	
	copystring (fsname (fs), fn);
	
	nullterminate (fn);
	
	folderfrompath (fn, bsfolder);
	
	pushstring (bsnew, bsfolder);
	
	nullterminate (bsfolder);
	
	if (MoveFile (stringbaseaddress (fn), stringbaseaddress (bsfolder)))
		return (true);
	
	winfileerror (fs);
	
	return (false);
} /*renamefile*/

boolean lockfile ( const ptrfilespec fs ) {
	
	DWORD attr;
	
	attr = GetFileAttributes (stringbaseaddress (fsname (fs)));
	
	if (attr == 0xFFFFFFFF)
		goto error;
	
	attr = attr | FILE_ATTRIBUTE_READONLY;
	
	if (SetFileAttributes (stringbaseaddress (fsname (fs)), attr))
		return (true);
	
error:
	
	winfileerror (fs);
	
	return (false);
	
} // lockfile

boolean unlockfile (const ptrfilespec fs) {
	
	DWORD attr;
	
	attr = GetFileAttributes (stringbaseaddress (fsname (fs)));
	
	if (attr == 0xFFFFFFFF)
		goto error;
	
	attr = attr & ~FILE_ATTRIBUTE_READONLY;
	
	if (SetFileAttributes (stringbaseaddress(fsname (fs)), attr))
		return (true);
	
error:
	
	winfileerror (fs);
	
	return (false);
	
} // unlockfile

boolean newfolder ( const ptrfilespec fs ) {	
	
	if (CreateDirectory (stringbaseaddress (fsname (fs)), NULL))
		return (true);
	
	winfileerror (fs);
	
	return (false);
	
} // newfolder

boolean newfile ( const ptrfilespec fs, OSType creator, OSType filetype ) {
	
	HANDLE f;
	char fn[300];
	
	copystring (fsname (fs), fn);
	
	nullterminate (fn);
	
	f = CreateFile (stringbaseaddress(fn), GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (f == INVALID_HANDLE_VALUE) {
		
		winfileerror (fs);
		
		return (false);
		
	}
	
	verify (CloseHandle (f));
	
	return (true);
	
} // newfile

boolean initfile (void) {
	
	/*
	 call this before you do any setvol's.
	 
	 get me the volume id of the application that's running.
	 
	 useful for opening data files that are located in the same folder as
	 the application itself.
	 
	 11/8/90 dmb: new implementation
	 */
			
	return (true);
} /*initfile*/

boolean findapplication (OSType creator, ptrfilespec fsapp) {
	
	//
	// 2007-06-12 creedon: for Mac, use getfilespecparent
	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	// 2006-04-10 creedon: deleted old code, see revision 1246 for old code
	//
	// 2006-04-09 creedon: use LSFindApplicationForInfo if available
	//
	// 5.0.1 dmb: implemented for Win using the registry
	//
	// 2.1b11 dmb: loop through all files in each db if necessary to find one
	//			that's actually an application
	//
	// 1992-09-07 dmb: make two passes, skipping volumes mounted with a foreign 
	//			    file system the first time through. note: if this turns
	//			    out not to be the best criteria, we could check for
	//			    shared volumes instead by testing vMLocalHand in
	//			    hasdesktopmanager.
	//
	//			    also: since we don't maintain the desktop DB when we
	//			    delete files, we need to verify that the file we locate
	//			    still exists. added fileexists call before returning
	//			    true.
	//
	// 1991-12-05 dmb: created
	//
	
	byte bsextension [8];
	bigstring bsregpath, bsoptions;
	
	ostypetostring (creator, bsextension);
	
	poptrailingwhitespace (bsextension);
	
	insertchar ('.', bsextension);
	
	pushchar (chnul, bsextension);
	
	// copyctopstring ("software\\Microsoft\\Windows\\CurrentVersion", bsregpath);
	
	if (!getRegKeyString ((Handle) HKEY_CLASSES_ROOT, bsextension, NULL, bsregpath))
		return (false);
	
	pushstring ("\x13\\shell\\open\\command", bsregpath);
	
	if (!getRegKeyString ((Handle) HKEY_CLASSES_ROOT, bsregpath, NULL, bsregpath))
		return (false);
	
	if (getstringcharacter (bsregpath, 0) == '"') {
		
		popleadingchars (bsregpath, '"');
		
		firstword (bsregpath, '"', bsregpath);
		
	}
	
	else {
		
		lastword (bsregpath, ' ', bsoptions);
		
		if (stringlength (bsoptions) < stringlength (bsregpath))
			deletestring (bsregpath, stringlength (bsregpath) - stringlength (bsoptions) + 1, stringlength (bsoptions));
	}
	
	return (pathtofilespec (bsregpath, fsapp));
	
} // findapplication

void clearfilespec (ptrfilespec fs) {
	
	/*
	 2009-09-04 aradke: init to nil filespec
	 */
	
	clearbytes (fs, sizeof (tyfilespec));
	
} /*clearfilespec*/
