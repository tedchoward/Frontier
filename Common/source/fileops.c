
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

#ifdef WIN95VERSION
#include "winregistry.h"
#endif

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

#ifdef MACVERSION

	#if TARGET_API_MAC_CARBON
		#include "MoreFilesX.h"
	#else
		pascal OSErr XGetVInfo(short volReference, StringPtr volName, short *vRefNum,
												UInt64 *freeBytes, UInt64 *totalBytes);
	#endif
	
	#ifdef flcomponent
		#include "SetUpA5.h"
	#endif

	#define hasOpenDeny(volParms)		(((volParms).vMAttrib & (1L << bHasOpenDeny)) != 0)
	
	#ifndef __MOREFILESEXTRAS__
		enum { /*permissions used by MoreFile 1.1 code*/
			dmNone			= 0x0000,
			dmNoneDenyRd	= 0x0010,
			dmNoneDenyWr	= 0x0020,
			dmNoneDenyRdWr	= 0x0030,
			dmRd			= 0x0001,	/* Single writer, multiple readers; the readers */
			dmRdDenyRd		= 0x0011,
			dmRdDenyWr		= 0x0021,	/* Browsing - equivalent to fsRdPerm */
			dmRdDenyRdWr	= 0x0031,
			dmWr			= 0x0002,
			dmWrDenyRd		= 0x0012,
			dmWrDenyWr		= 0x0022,
			dmWrDenyRdWr	= 0x0032,
			dmRdWr			= 0x0003,	/* Shared access - equivalent to fsRdWrShPerm */
			dmRdWrDenyRd	= 0x0013,
			dmRdWrDenyWr	= 0x0023,	/* Single writer, multiple readers; the writer */
			dmRdWrDenyRdWr	= 0x0033	/* Exclusive access - equivalent to fsRdWrPerm */
			};
	#endif
	
#endif


#ifdef flsystem6

static short applicationvolnum = 0;

#endif

static short applicationresnum = -1;

#ifdef MACVERSION
static OSType specialfolders [] = {
	
	0, /*make 1-based*/
	
	#if TARGET_API_MAC_CARBON //macs is not working right now. This is not a good substitute, nice hack eh?
	'pref',
	#else
	'macs', /*system*/
	#endif
	'desk', /*desktop*/
	
	'trsh', /*trash*/
	
	'empt', /*shared trash*/
	
	'prnt', /*printmonitor documents*/
	
	'strt', /*startup items*/
	
	'amnu', /*apple menu items*/
	
	'ctrl', /*control panels*/
	
	'extn', /*extensions*/
	
	'pref', /*preferences*/
	
	'temp',  /*temporary items*/
	
	'font'  /*Fonts*/
	};
#endif


typedef struct tyfileinfo tyvolinfo;


void setfserrorparam (const tyfilespec *fs) {
	
	/*
	bigstring bs;
	
	if (!filespectopath (fs, bs))
		copystring ((ptrstring) (*fs).name, bs);
	
	setoserrorparam (bs);
	*/
	
	setoserrorparam ((ptrstring) fsname (fs));
	} /*setfserrorparam*/


boolean endswithpathsep (bigstring bs) {
	/* return true if the ending character is a path sepatator character*/
	char ch;

	ch = getstringcharacter(bs, stringlength(bs)-1);

#ifdef MACVERSION
	if (ch == ':')
		return (true);
#endif

#ifdef WIN95VERSION
	if (ch == '\\')
		return (true);

	if (ch == '/')
		return (true);
#endif

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

	boolean fl;
	
	#ifdef WIN95VERSION
		DWORD len;

		len = sizeof(bigstring) - 2;

		fl = GetComputerName (stringbaseaddress(bsname), &len);

		if (fl)
			setstringlength (bsname, len);
		else
			setemptystring (bsname);
	#endif
	
	#ifdef MACVERSION
		StringHandle hstring = GetString (-16413);
		
		fl = hstring != nil;
		
		if (fl)
			texthandletostring ((Handle) hstring, bsname);
		else
			setemptystring (bsname);
	#endif

	return (fl);
	} /*getmachinename*/


#ifdef MACVERSION
boolean foldertest (CInfoPBRec *pb) {
	
	/*
	return true if pb holds info describing a folder.
	
	4/26/96 dmb: use mask, not BitTst (a toolbox call)
	*/
	
	/*
	return (BitTst (&pb->dirInfo.ioFlAttrib, 3));
	*/
	return ((pb->dirInfo.ioFlAttrib & ioDirMask) != 0);
	} /*foldertest*/


static void filebeachball (void) {
	
	/*
	roll the beachball cursor if there is one.
	*/
	
	if (beachballcursor ())
		rollbeachball ();
	} /*filebeachball*/
	

boolean getmacfileinfo (const tyfilespec *fs, CInfoPBRec *pb) {
	
	/* 
	2.1b2 dmb: new fsspec-based version
	*/
	
	setoserrorparam ((ptrstring) (*fs).name); /*in case error message takes a filename parameter*/
	
	clearbytes (pb, sizeof (*pb));
	
	(*pb).hFileInfo.ioNamePtr = (StringPtr) (*fs).name;
	
	(*pb).hFileInfo.ioVRefNum = (*fs).vRefNum;
	
	(*pb).hFileInfo.ioDirID = (*fs).parID;
	
	return (!oserror (PBGetCatInfoSync (pb)));
	} /*getmacfileinfo*/


static boolean setmacfileinfo (const tyfilespec *fs, CInfoPBRec *pb) {
	
	/* 
	2.1b2 dmb: new fsspec-based version
	*/
	
	setoserrorparam ((ptrstring) (*fs).name); /*in case error message takes a filename parameter*/
	
	(*pb).hFileInfo.ioNamePtr = (StringPtr) (*fs).name;
	
	(*pb).hFileInfo.ioVRefNum = (*fs).vRefNum;
	
	(*pb).hFileInfo.ioDirID = (*fs).parID;
	
	return (!oserror (PBSetCatInfoSync (pb)));
	} /*setmacfileinfo*/


static boolean touchparentfolder (const tyfilespec *fs) {
	
	/*
	touch the file date of the parent folder of fs
	*/
	
	FSSpec fsfolder;
	
	if (FSMakeFSSpec ((*fs).vRefNum, (*fs).parID, nil, &fsfolder) != noErr)
		return (false);
	
	setfilemodified (&fsfolder, timenow ());
	
	return (true);
	} /*touchparentfolder*/
#endif


#ifdef WIN95VERSION

boolean winfileerror (const tyfilespec *fs) {
	
	DWORD err = GetLastError ();

	setfserrorparam (fs);
	
	return (oserror (err));
	} /*winfileerror*/

#endif

#ifdef MACVERSION // 1/29/97 dmb: adopt applet toolkit version

static boolean getinfofromvolpb (const HVolumeParam *pb, tyfileinfo *info) {
	
	/*
	dmb 9/21/93: for a volume, dirid should be fsRtParID, not fsRtDirID.
	*/
	
	short drivenum;
	#if !TARGET_API_MAC_CARBON
	QHdrPtr dqtop;
	QElemPtr dqelem;
	#endif

	
	clearbytes (info, sizeof (tyfileinfo));
	
	(*info).vnum = (*pb).ioVRefNum;
	
	(*info).dirid = fsRtParID; /*fsRtDirID*/
	
	(*info).flvolume = true;
	
	(*info).flfolder = true;
	
	(*info).timecreated = (*pb).ioVCrDate;
	
	(*info).timemodified = (*pb).ioVLsMod;
	
	(*info).fllocked = ((*pb).ioVAtrb & 0x8000) != 0;
	
	(*info).flhardwarelock = ((*pb).ioVAtrb & 0x0080) != 0;
	
	if ((*info).flhardwarelock)
		(*info).fllocked = true;
	
	(*info).ctfreebytes = (*pb).ioVAlBlkSiz * (*pb).ioVFrBlk;
	
	(*info).cttotalbytes = (*pb).ioVAlBlkSiz * (*pb).ioVNmAlBlks;
	
	(*info).blocksize = (*pb).ioVAlBlkSiz;
	
	(*info).ctfiles = (*pb).ioVFilCnt;
	
	(*info).ctfolders = (*pb).ioVDirCnt;
	
	drivenum = (*pb).ioVDrvInfo;
	
	//Code change by Timothy Paustian Sunday, June 25, 2000 9:17:36 PM
	//The below code is to figure out if a volume is ejectable. I don't think
	//you can do this in carbon, so just set it to false.
	#if TARGET_API_MAC_CARBON == 1
	(*info).flejectable = false;
	#else
		
	dqtop = GetDrvQHdr ();
	
	dqelem = (*dqtop).qHead;
	
	while (true) {
		
		if (dqelem == nil) { /*volume is no longer in a drive; it must be ejectable!*/
			
			(*info).flejectable = true;
			
			break;
			}
		
		if ((*(DrvQEl *)dqelem).dQDrive == drivenum) {
			
			byte driveflag = *((byte *) dqelem - 3);
			
			(*info).flejectable = ((driveflag != 8) && (driveflag != 0x48)); /*IM IV-181*/
			
			break;
			}
		
		dqelem = (*dqelem).qLink;
		} /*while*/
		#endif
		
	
	return (true);
	} /*getinfofromvolpb*/


boolean filegetvolumeinfo (short vnum, tyfileinfo *info) {
	
	/*
	dmb 9/21/93: take vnum as parameter, not volname. otherwise, 
	we can't distinguish between two vols w/the same name.
	*/
	
	HVolumeParam pb;
	
	clearbytes (&pb, sizeof (pb)); /*init all fields to zero*/
	
	pb.ioVRefNum = vnum;
	
	if (oserror (PBHGetVInfoSync ((HParmBlkPtr) &pb)))
		return (false);
	
	clearbytes (info, sizeof (tyfileinfo)); /*init all fields to zero*/
	
	getinfofromvolpb (&pb, info);
	
	/*DW 9/7/93: determine if it's a network volume*/ {
		
		HParamBlockRec pb;
		GetVolParmsInfoBuffer buffer;
		OSErr ec;
		
		clearbytes (&pb, sizeof (pb)); /*init all fields to zero*/
		
		pb.ioParam.ioVRefNum = (*info).vnum;
		
		pb.ioParam.ioBuffer = (Ptr) &buffer;
		
		pb.ioParam.ioReqCount = sizeof (buffer);
		
		ec = PBHGetVolParmsSync (&pb); 
	
		if (ec == noErr) 
			(*info).flremotevolume = buffer.vMServerAdr != 0; /*see Apple TN-Files docviewer doc*/
		}
	
	return (true);
	} /*filegetvolumeinfo*/


void filegetinfofrompb (CInfoPBRec *pb, tyfileinfo *info) {
	
	/*
	dmb 9/24/93: handle volumes here, combining vol info with root 
	directory folder info. I'm not sure if a volume lock is always 
	reflected in the root directory, so don't set fllocked false 
	if the attribute isn't set in the pb. (it starts out cleared anyway.)
	
	5.1.4 dmb: set finderbits for folders too.
	*/
	
	short finderbits;
	
	clearbytes (info, sizeof (tyfileinfo)); /*init all fields to zero*/
	
	(*info).vnum = (*pb).hFileInfo.ioVRefNum;
	
	(*info).dirid = (*pb).hFileInfo.ioFlParID;
	
	if ((*info).dirid == fsRtParID)
		filegetvolumeinfo ((*info).vnum, info);
	else
		(*info).flvolume = false;
	
	if (BitTst (&(*pb).dirInfo.ioFlAttrib, 7)) /*if it's a volume, fllocked may already be set*/
		(*info).fllocked = true;
	
//	(*info).flfolder = BitTst (&(*pb).dirInfo.ioFlAttrib, 3);
	(*info).flfolder = (((*pb).dirInfo.ioFlAttrib & ioDirMask) != 0);
	
	(*info).ixlabel = ((*pb).hFileInfo.ioFlFndrInfo.fdFlags & 0x000E) >> 1;
	
	if ((*info).flfolder) {

		/*Folders are considered "busy" if there are any files within the folder */
		
		(*info).flbusy = (*pb).dirInfo.ioDrNmFls > 0;
		
		(*info).filecreator = (*info).filetype = '    ';
		
		if (!(*info).flvolume) { /*these aren't the same for a volume & its root dir*/
			
			(*info).timecreated = (*pb).dirInfo.ioDrCrDat;
			
			(*info).timemodified = (*pb).dirInfo.ioDrMdDat;
			
			(*info).ctfiles = (*pb).dirInfo.ioDrNmFls;
			}
		
		(*info).iconposition = (*pb).dirInfo.ioDrUsrWds.frLocation;
		
		(*info).folderview = (tyfolderview) ((*pb).dirInfo.ioDrUsrWds.frView >> 8);
		
		finderbits = (*pb).dirInfo.ioDrUsrWds.frFlags;
		}
	else { /*fill in fields for a file, somewhat different format than a folder*/
	
		(*info).flbusy = BitTst (&(*pb).hFileInfo.ioFlAttrib, 0);
			
		(*info).filecreator = (*pb).hFileInfo.ioFlFndrInfo.fdCreator;
		
		(*info).filetype = (*pb).hFileInfo.ioFlFndrInfo.fdType;
		
		(*info).timecreated = (*pb).hFileInfo.ioFlCrDat;

		(*info).timemodified = (*pb).hFileInfo.ioFlMdDat;
		
		(*info).sizedatafork = (*pb).hFileInfo.ioFlLgLen;
		
		(*info).sizeresourcefork = (*pb).hFileInfo.ioFlRLgLen;
		
		(*info).iconposition = (*pb).hFileInfo.ioFlFndrInfo.fdLocation;
		
		finderbits = (*pb).hFileInfo.ioFlFndrInfo.fdFlags;
		}

	/*copy from the finder bits into the record*/ {
	
		(*info).flalias = (finderbits & kIsAlias) != 0;
		
		(*info).flbundle = (finderbits & kHasBundle) != 0;
		
		(*info).flinvisible = (finderbits & kIsInvisible) != 0;
		
		(*info).flstationery = (finderbits & kIsStationery) != 0;
		
		(*info).flshared = (finderbits & kIsShared) != 0;
		
		(*info).flnamelocked = (finderbits & kNameLocked) != 0;
		
		(*info).flcustomicon = (finderbits & kHasCustomIcon) != 0;
		}
	} /*filegetinfofrompb*/
#endif	

#ifdef MACVERSION
#define filegetfsvolumeinfo(fs, info) filegetvolumeinfo((*fs).vRefNum, info)
#endif

#ifdef WIN95VERSION
static boolean filegetfsvolumeinfo (const tyfilespec *fs, tyfileinfo *info) {
	DWORD sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;
	UINT drivetype;
	bigstring volname;
	bigstring errmsg;

	clearbytes (info, sizeof (tyfileinfo)); /*init all fields to zero*/

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

	wsprintf (stringbaseaddress(errmsg), "Can't complete function because \"%s\" is not a valid volume name.", stringbaseaddress(fsname(fs)));

	setstringlength (errmsg, strlen(stringbaseaddress(errmsg)));

	shellerrormessage (errmsg);

	return (false);	
	} /*filegetfsvolumeinfo*/


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

#endif


boolean filegetinfo (const tyfilespec *fs, tyfileinfo *info) {

#ifdef MACVERSION	
	/*
	dmb 9/24/93: let filegetinfofrompb take care of volumes
	*/
	
	CInfoPBRec pb;
	
	/*
	if (isvolumefilespec (pfs)) {
		
		return (filegetfsvolumeinfo (pfs, info));
		}
	*/
	
	if (!getmacfileinfo (fs, &pb))
		return (false);
	
	filegetinfofrompb (&pb, info);
#endif
	
#ifdef WIN95VERSION
	HANDLE findHandle;
	HANDLE fref;
	WIN32_FIND_DATA	fileinfo;
	char fn[300];
	
	clearbytes (info, sizeof (tyfileinfo)); /*init all fields to zero*/
	info->filecreator = '    ';		
	info->filetype = '    ';

	if (fileisvolume (fs)) {
		return (filegetfsvolumeinfo (fs, info));
		}

	copystring (fsname (fs), fn);
	
	/*if ends with \ get ride of it... and handle the root*/

	cleanendoffilename (fn);

	nullterminate (fn);


	findHandle = FindFirstFile (stringbaseaddress(fn), &fileinfo);

	if (findHandle == INVALID_HANDLE_VALUE) {
		
		winfileerror (fs);

		return (false);
		}

	winsetfileinfo (&fileinfo, info);

	FindClose(findHandle);

	//Set the file busy flag (this should be in winsetfileinfo, but we do not have the filename there)
	if (info->flfolder) {
		strcat (stringbaseaddress (fn), "\\*");

		info->flbusy = false; //presume empty folder (not busy)

		findHandle = FindFirstFile (stringbaseaddress(fn), &fileinfo);

		if (findHandle != INVALID_HANDLE_VALUE) {
			info->flbusy = true;				//Found something

			while ((strcmp (fileinfo.cFileName, ".") == 0) || (strcmp (fileinfo.cFileName, "..") == 0)) {
				info->flbusy = false;			// Just . or ..

				if (FindNextFile (findHandle, &fileinfo))
					info->flbusy = true;		//Found something else...
				else
					break;		//exit while loop if FindNext fails (this is normal)
				}

			FindClose (findHandle);
			}
		}
	else {
		fref = (Handle) CreateFile (stringbaseaddress(fn), GENERIC_READ | GENERIC_WRITE, 0,
													NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fref == INVALID_HANDLE_VALUE) {
			info->flbusy = true;
			}
		else {
			info->flbusy = false;
			verify (CloseHandle (fref));
			}
		}

#endif
	return (true);
	} /*filegetinfo*/
		

	
boolean filegetvolumename (short vnum, bigstring volname) {
#ifdef MACVERSION	
	HVolumeParam pb;
	OSErr ec;
	
	clearbytes (&pb, sizeof (pb)); /*init all fields to zero*/
	
	pb.ioNamePtr = volname;
	
	pb.ioVRefNum = vnum;
	
	ec = PBHGetVInfoSync ((HParmBlkPtr) &pb);
	
	if (ec != noErr) {
		
		setstringlength (volname, 0);
		
		return (false);
		}
		
	return (true);
#endif

#ifdef WIN95VERSION
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
#endif
	} /*filegetvolumename*/
	
	
	
boolean fileisbusy (const tyfilespec *fs, boolean *flbusy) {
	
	/*
	6/x/91 mao
	*/
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flbusy = info.flbusy;
	
	return (true);
	} /*fileisbusy*/
	

boolean filehasbundle (const tyfilespec *fs, boolean *flbundle) {
	
	/*
	6/x/91 mao
	*/
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flbundle = info.flbundle;
	
	return (true);
	} /*filehasbundle*/


boolean filesetbundle (const tyfilespec *fs, boolean flbundle) {
#ifdef MACVERSION
	/*
	8/10/92 dmb
	*/
	
	CInfoPBRec pb;
	
	if (!getmacfileinfo (fs, &pb)) 
		return (false);
	
	if (flbundle) 
		pb.hFileInfo.ioFlFndrInfo.fdFlags |= kHasBundle; /*turn on*/
	else
		pb.hFileInfo.ioFlFndrInfo.fdFlags &= ~kHasBundle; /*turn off*/
	
	return (setmacfileinfo (fs, &pb));
#endif

#ifdef WIN95VERSION
	return (false);
#endif
	} /*filesetbundle*/


boolean fileisalias (const tyfilespec *fs, boolean *flalias) {
	
	/*
	6/x/91 mao
	*/
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flalias = info.flalias;
	
	return (true);
	} /*fileisalias*/


boolean fileisvisible (const tyfilespec *fs, boolean *flvisible) {
	
	/*
	6/9/92 dmb
	*/
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flvisible = !info.flinvisible;
	
	return (true);
	} /*fileisvisible*/


boolean filesetvisible (const tyfilespec *fs, boolean flvisible) {
#ifdef MACVERSION
	/*
	6/9/92 dmb
	*/
	
	CInfoPBRec pb;
	
	if (!getmacfileinfo (fs, &pb)) 
		return (false);
	
	if (flvisible) 
		pb.hFileInfo.ioFlFndrInfo.fdFlags &= ~kIsInvisible; /*turn off*/
	else
		pb.hFileInfo.ioFlFndrInfo.fdFlags |= kIsInvisible; /*turn on*/
	
	if (!setmacfileinfo (fs, &pb))
		return (false);
	
	touchparentfolder (fs);
#endif	
	
#ifdef WIN95VERSION
	tyfileinfo info;
	DWORD attr;
	
	if (!filegetinfo (fs, &info))
		return (false);

	if (info.flinvisible == flvisible) /*check if we need to do anything first*/
		{
		attr = GetFileAttributes (stringbaseaddress (fsname (fs)));

		if (attr == 0xFFFFFFFF)
			return (false);

		attr = attr & (~FILE_ATTRIBUTE_HIDDEN);

		if (! flvisible)
			attr = attr | FILE_ATTRIBUTE_HIDDEN;

		return(SetFileAttributes (stringbaseaddress (fsname (fs)), attr));
		}
#endif
	return (true);
	} /*filesetvisible*/



boolean getfiletype (const tyfilespec *fs, OSType *type) {
	
	tyfileinfo info;
	
	*type = '    ';
	
	if (!filegetinfo (fs, &info))
		return (false);
		
	*type = info.filetype;
	
	return (true);
	} /*getfiletype*/

	
boolean getfilecreator (const tyfilespec *fs, OSType *creator) {
	
	tyfileinfo info;
	
	*creator = '    ';
	
	if (!filegetinfo (fs, &info))
		return (false);
		
	*creator = info.filecreator;
	
	return (true);
	} /*getfilecreator*/



boolean filesize (const tyfilespec *fs, long *size) {
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*size = info.sizedatafork + info.sizeresourcefork;
	
	return (true);
	} /*filesize*/
	
	
boolean fileisfolder (const tyfilespec *fs, boolean *flfolder) {
	
	tyfileinfo info;

	#ifdef WIN95VERSION
		/* special case the root directory on Windows */
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

	#endif
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*flfolder = info.flfolder;
	
	return (true);
	} /*fileisfolder*/


boolean fileisvolume (const tyfilespec *fs) {
#ifdef MACVERSION	
	if (isemptystring ((*fs).name))
		return (false);
	
	return ((*fs).parID == fsRtParID);
#endif

#ifdef WIN95VERSION
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
#endif
	} /*fileisvolume*/


boolean fileislocked (const tyfilespec *fs, boolean *fllocked) {
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*fllocked = info.fllocked;
	
	return (true);
	} /*fileislocked*/
	
	
boolean getfiledates (const tyfilespec *fs, unsigned long *datecreated, unsigned long *datemodified) {
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*datecreated = info.timecreated;
	
	*datemodified = info.timemodified;
	
	return (true);
	} /*getfiledates*/


boolean setfiledates (const tyfilespec *fs, unsigned long datecreated, unsigned long datemodified) {
#ifdef MACVERSION	
	/*
	6/x/91 mao: modified to work with new getmacilfeinfo/setmacfileinfo.
	*/
	
	CInfoPBRec pb;
	
	if (!getmacfileinfo (fs, &pb)) 
		return (false);
	
	if (foldertest (&pb)) {
		
		pb.dirInfo.ioDrCrDat = datecreated;
		
		pb.dirInfo.ioDrMdDat = datemodified;
		}
	else {
		
		pb.hFileInfo.ioFlCrDat = datecreated;
	
		pb.hFileInfo.ioFlMdDat = datemodified;
		}
	
	return (setmacfileinfo (fs, &pb));
#endif

#ifdef WIN95VERSION
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
#endif
	} /*setfiledates*/


#ifdef MACVERSION
boolean getfilepos (const tyfilespec *fs, Point *pt) {
	
	/*
	mao 6/x/91: modified to work with new getmacilfeinfo/setmacfileinfo.
	*/
	
	CInfoPBRec pb;
	
	if (!getmacfileinfo (fs, &pb)) 
		return (false);
	
	if (foldertest (&pb)) 
		*pt = pb.dirInfo.ioDrUsrWds.frLocation;
	else	
		*pt = pb.hFileInfo.ioFlFndrInfo.fdLocation;
	
	return (true);
	} /*getfilepos*/


boolean setfilepos (const tyfilespec *fs, Point pt) {

	/*
	6/x/91 mao: modified to work with new getmacilfeinfo/setmacfileinfo.
	*/
	
	CInfoPBRec pb;
	
	if (!getmacfileinfo (fs, &pb)) 
		return (false);
	
	if (foldertest (&pb)) 
		pb.dirInfo.ioDrUsrWds.frLocation = pt;
	else
		pb.hFileInfo.ioFlFndrInfo.fdLocation = pt;
	
	return (setmacfileinfo (fs, &pb));
	} /*setfilepos*/
#endif

boolean setfilecreated (const tyfilespec *fs, long when) {
	
	/*
	7/31/91 dmb: created
	*/
	
	unsigned long datecreated, datemodified;
	
	if (!getfiledates (fs, &datecreated, &datemodified))
		return (false);
	
	return (setfiledates (fs, when, datemodified));
	} /*setfilecreated*/


boolean setfilemodified (const tyfilespec *fs, long when) {
	
	/*
	7/31/91 dmb: created
	*/
	
	unsigned long datecreated, datemodified;
	
	if (!getfiledates (fs, &datecreated, &datemodified))
		return (false);
	
	return (setfiledates (fs, datecreated, when));
	} /*setfilemodified*/


#ifdef MACVERSION
boolean setfiletype (const tyfilespec *fs, OSType filetype) {
	
	/*
	6/x/91 mao: modified to work with new getmacilfeinfo/setmacfileinfo.
	*/
	
	CInfoPBRec pb;
	
	if (!getmacfileinfo (fs, &pb)) 
		return (false);
	
	if (foldertest (&pb)) 
		return (false);
	
	pb.hFileInfo.ioFlFndrInfo.fdType = filetype;
	
	return (setmacfileinfo (fs, &pb));
	} /*setfiletype*/


boolean setfilecreator (const tyfilespec *fs, OSType filecreator) {
	
	/*
	6/x/91 mao: modified to work with new getmacilfeinfo/setmacfileinfo. Also, makes sure
	folders aren't operated on
	*/
	
	CInfoPBRec pb;
	
	if (!getmacfileinfo (fs, &pb)) 
		return (false);
		
	if (foldertest (&pb)) 
		return (false);
	
	pb.hFileInfo.ioFlFndrInfo.fdCreator = filecreator;
	
	return (setmacfileinfo (fs, &pb));
	} /*setfilecreator*/
#endif

#ifdef MACVERSION
static boolean copyfork (hdlfilenum fsource, hdlfilenum fdest, Handle hbuffer) {
	
	/*
	copy either the data fork or resource fork of the indicated file.
	
	return true iff the file copy was successful.
	
	5/19/92 dmb: call langbackgroundtask when a script is running to make it more 
	likely that we'll yield the processor to another app.  also, only allow 
	background tasks if fork is larger than a single buffer.  note that if a script 
	is copying a bunch of files in a loop, the interpreter is already allowing 
	backgrounding between files.
	*/

	register long buffersize = GetHandleSize (hbuffer);
	long ctbytes;
	register OSErr errcode;
	
	SetFPos (fsource, fsFromStart, 0L); 
	
	SetFPos (fdest, fsFromStart, 0L);
	
	while (true) { 
		
		ctbytes = buffersize;
		
		HLock (hbuffer);
		
		errcode = FSRead (fsource, &ctbytes, *hbuffer);
		
		HUnlock (hbuffer);
		
		if ((errcode != noErr) && (errcode != eofErr)) {
			
			oserror (errcode);
			
			return (false);
			}
			
		if (ctbytes == 0) /*last read got no bytes*/
			return (true);
		
		HLock (hbuffer);
		
		errcode = FSWrite (fdest, &ctbytes, *hbuffer);
		
		HUnlock (hbuffer);
		
		if (oserror (errcode))
			return (false);
		
		if (ctbytes < buffersize) /*copy of fork is finished*/
			return (true);
		
		if (flscriptrunning)
			langbackgroundtask (false);
		else
			shellbackgroundtask (); /*give background processes a chance*/
		
		filebeachball (); /*roll the beachball cursor if there is one*/
		} /*while*/
	} /*copyfork*/


#endif

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

#ifdef MACVERSION
	ctbytes = FreeMem () / 2;
	
	if (ctbytes < 1024) { /*no enough memory to work with*/
		
		memoryerror ();
		
		return (false);
		}
	
	ctbytes &= ~(1024 - 1); /*fast round down to nearest multiple of 1024*/
	
	h = NewHandle (ctbytes);
	
	if (h != nil) {
		
		*hbuffer = h;
		
		return (true);
		}
	
	/*allocation failed; heap has been compacted; find largest block size*/
	
	ctbytes = CompactMem (maxSize); /*recompact the heap, finding out size of largest block*/
	
	if (ctbytes < 1024) { /*largest block is too small to use*/
		
		memoryerror ();
		
		return (false);
		}
	
#endif

#ifdef WIN95VERSION
	DBG_UNREFERENCED_LOCAL_VARIABLE(h);
	
	ctbytes = 0x00020000L;		/* 128K buffer */
#endif

	ctbytes &= ~(1024 - 1); /*fast round down to nearest multiple of 1024*/
	
	return (newhandle (ctbytes, hbuffer)); /*shouldn't fail at this point, but let's be sure*/
	} /*largefilebuffer*/


#ifdef MACVERSION

static pascal OSErr FSpCreateMinimum (const FSSpec *fs)
{
	HParamBlockRec pb;
	
	pb.fileParam.ioNamePtr = (StringPtr)(*fs).name;
	
	pb.fileParam.ioVRefNum = (*fs).vRefNum;
	
	pb.ioParam.ioVersNum = 0;
	
	pb.fileParam.ioDirID = (*fs).parID;
	
	return (PBHCreateSync (&pb));
	} /*FSpCreateMinimum*/


static pascal OSErr FSpOpenAware (const tyfilespec *fs, boolean flresource, short denyModes, hdlfilenum *refNum) {
	
	/*
	3.0.2 dmb: customized version of HOpenAware from MoreFiles 1.1
	*/
	
	HParamBlockRec pb;
	OSErr err;
	GetVolParmsInfoBuffer volParmsInfo;
	long infoSize = sizeof (GetVolParmsInfoBuffer);
	
	pb.ioParam.ioNamePtr = (StringPtr) (*fs).name;
	pb.ioParam.ioVRefNum = (*fs).vRefNum;
	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioMisc = nil;
	
	/* 
	get volume attributes
	this preflighting is needed because Foreign File Access based file systems don't
	return the correct error result to the OpenDeny call
	*/
	pb.ioParam.ioBuffer = (Ptr) &volParmsInfo;
	pb.ioParam.ioReqCount = infoSize;
	
	err = PBHGetVolParmsSync (&pb);
	
	pb.fileParam.ioDirID = (*fs).parID;
	
	/*
	err = HGetVolParms ((StringPtr) (*fs).name, (*fs).vRefNum, &volParmsInfo, &infoSize);
	*/
	
	if (err == noErr) {
		
		/* if volume supports OpenDeny, use it and return */
		if (hasOpenDeny (volParmsInfo)) {
			
			pb.accessParam.ioDenyModes = denyModes;
			
			if (flresource)
				err = PBHOpenRFDenySync (&pb);
			else
				err = PBHOpenDenySync (&pb);
			
			*refNum = pb.ioParam.ioRefNum;
			
			return (err);
			}
		}
	else
		if (err != paramErr)	/* paramErr is OK, it just means this volume doesn't support GetVolParms */
			return (err);
	
	/*
	OpenDeny isn't supported, so try File Manager Open functions
	Set File Manager permissions to closest thing possible
	*/
	pb.ioParam.ioPermssn = ((denyModes == dmWr) || (denyModes == dmRdWr)) ? (fsRdWrShPerm) : (denyModes % 4);
	
	if (flresource)
		err = PBHOpenRFSync (&pb);
	
	else {
		
		err = PBHOpenDFSync (&pb);				/* Try OpenDF */
		
		if (err == paramErr)
			err = PBHOpenSync (&pb);			/* OpenDF not supported, so try Open */
		}
	
	*refNum = pb.ioParam.ioRefNum;
	
	return (err);
	} /*FSpOpenAware*/


static boolean copyfileattrs (const tyfilespec *fsource, const tyfilespec *fdest, boolean flcopylockbit) {
	
	CInfoPBRec pb;
	OSErr err;
	boolean flfolder;
	
	pb.hFileInfo.ioVRefNum = (*fsource).vRefNum;
	pb.hFileInfo.ioDirID = (*fsource).parID;
	pb.hFileInfo.ioNamePtr = (StringPtr) (*fsource).name;
	pb.hFileInfo.ioFDirIndex = 0;
	
	err = PBGetCatInfoSync(&pb);
	
	if (err == noErr) {
		
		flfolder = (pb.hFileInfo.ioFlAttrib & 0x10);
		
		pb.hFileInfo.ioVRefNum = (*fdest).vRefNum;
		pb.hFileInfo.ioDirID = (*fdest).parID;
		pb.hFileInfo.ioNamePtr = (StringPtr) (*fdest).name;
		
		/* don't copy the hasBeenInited bit */
		pb.hFileInfo.ioFlFndrInfo.fdFlags = (pb.hFileInfo.ioFlFndrInfo.fdFlags & 0xfeff);
		
		err = PBSetCatInfoSync(&pb);
		
		if ((err == noErr) && (flcopylockbit) && (pb.hFileInfo.ioFlAttrib & 0x01)) {
			
			err = PBHSetFLockSync ((HParmBlkPtr) &pb);
			
			if ((err != noErr) && (flfolder))
				err = noErr; /* ignore lock errors if destination is directory */
			}
		}
	
	return (err);
	} /*copyfileattrs*/


boolean copyfile (const tyfilespec *fsource, const tyfilespec *fdest, boolean fldata, boolean flresources) {

	/*
	create a copy of the indicated file in the destination volume, with the indicated
	name.  
	
	we allocate a good-sized buffer in the heap, then open and copy the data fork then
	open and copy the resource fork of the source file.
	
	return true if the operation was successful, false otherwise.
	
	7/27/90 DW: add fldata, flresources -- allows selective copying of the two
	forks of each file.  we assume one of these two booleans is true.
	
	3/16/92 dmb: maintain all public Finder flags, not just creator/type & dates
	
	3.0.2 dmb: use CreateMinimum and do things in special order for drop-box copies.
	For details, see MoreFiles:FileCopy.c on the Reference Library CD. note that 
	there are more details handled by the sample code that we're ignoring here. one 
	is using CopyFile if src and dst are on the same server and the server supports 
	that function. another is to open forks with deny modes set. maybe next time...
	also, we now handle folders, so file.copy can retain folder attributes.
	*/
	
	Handle hbuffer = nil;
	short sourcefnum = 0, sourcernum = 0;
	short destfnum = 0, destrnum = 0;
	boolean fldestcreated = false;
	boolean flsourcefolder, fldestfolder;
	
	if (!fileisfolder (fsource, &flsourcefolder))
		return (false);
	
	if (flsourcefolder)
		fldata = flresources = false;
	
	else {
		
		if (!largefilebuffer (&hbuffer)) 
			return (false);
		}
	
	if (fldata) {
		
		if (oserror (FSpOpenAware (fsource, false, dmRdDenyWr, &sourcefnum)))
			goto error;
		}
	
	if (flresources) {
		
		if (oserror (FSpOpenAware (fsource, true, dmRdDenyWr, &sourcernum)))
			goto error;
		}
	
	if (fileexists (fdest, &fldestfolder)) { /*file exists, delete it*/
		
		if (!deletefile (fdest))
			goto error;
		}
	
	setfserrorparam (fdest); /*in case error message takes a filename parameter*/
	
	if (flsourcefolder) {
		
		if (!newfolder (fdest))
			goto error;
		}
	else {
		
		if (oserror (FSpCreateMinimum (fdest)))
			goto error;
		}
	
	fldestcreated = true;
	
	if (oserror (copyfileattrs (fsource, fdest, false)))
		goto error;
	
	if (fldata) {
		
		if (oserror (FSpOpenAware (fdest, false, dmWrDenyRdWr, &destfnum)))
			goto error;
		}
	
	if (flresources) {
		
		if (oserror (FSpOpenAware (fdest, true, dmWrDenyRdWr, &destrnum)))
			goto error;
		}
	
	if (fldata) {
		
		if (!copyfork (sourcefnum, destfnum, hbuffer)) /*copy data fork*/
			goto error;
		
		closefile (sourcefnum); 
		
		closefile (destfnum); 
		
		sourcefnum = 0;
		
		destfnum = 0;
		}
	
	if (flresources) {
		
		if (!copyfork (sourcernum, destrnum, hbuffer)) /*copy resource fork*/
			goto error;
		
		closefile (sourcernum);
			
		closefile (destrnum); 
		
		sourcernum = 0;
		
		destrnum = 0;
		}
	
	disposehandle (hbuffer);
	
	hbuffer = nil; /*if error don't dispose of it again*/
	
	copyfileattrs (fsource, fdest, true); /*ignore error*/
	
	return (true); /*the file copy was successful*/
	
	error: /*goto here to release the buffer, close files and return false*/
	
	if (hbuffer != nil)
		disposehandle (hbuffer);
	
	if (sourcefnum != 0)
		closefile (sourcefnum);
	
	if (sourcernum != 0)
		closefile (sourcernum);
	
	if (destfnum != 0)
		closefile (destfnum);
	
	if (destrnum != 0)
		closefile (destrnum);
	
	if (fldestcreated)
		deletefile (fdest); /*no file created on error*/
	
	return (false);
	} /*copyfile*/
#endif


#ifdef WIN95VERSION
boolean copyfile (const tyfilespec *fsource, const tyfilespec *fdest, boolean fldata, boolean flresources) {
	
	/*
	5.0.1 dmb: if the file exists, overwrite it.
	*/

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
	} /*copyfile*/
#endif

short filegetapplicationrnum (void) {

	return (applicationresnum);
	} /*filegetapplicationrnum*/


#ifdef flsystem6

short filegetapplicationvnum (void) {

	return (applicationvolnum);
	} /*filegetapplicationvnum*/


short filegetsystemvnum (void) {

	SysEnvRec env;
	
	SysEnvirons (1, &env);
	
	return (env.sysVRefNum);
	} /*filegetsystemvnum*/

#endif

#ifdef MACVERSION
static boolean pathtovolume (bigstring bspath, short *vnum) {
	
	if (countwords (bspath, chpathseparator) > 1) {
		
		oserror (errorVolume); /*no such volume*/
		
		return (false);
		}
	
	if (!fileparsevolname (bspath, vnum, nil)) {
		
		oserror (errorVolume); /*no such volume*/
		
		return (false);
		}
	
	return (true);
	} /*pathtovolume*/
#endif

//Timothy Paustian's comments
//warning the values returned by getspecialfolderpath on OS X are very different
//than classic Mac OS
boolean getspecialfolderpath (bigstring bsvol, bigstring bsfolder, boolean flcreate, tyfilespec *fs) {
	
#ifdef MACVERSION
	/*
	9/1/92 dmb: last new verb for 2.0.  (?)
	*/
	
	short vnum;
	short ixlist;
	long dirid;
	OSType foldertype;
	long attrs;
	OSErr errcode = errorNone;
	bigstring bsfirst;
	
	if (!(gestalt (gestaltFindFolderAttr, &attrs) && (attrs & (1 << gestaltFindFolderPresent))))
		return (false);
	
	if (isemptystring (bsvol))

		#if TARGET_API_MAC_CARBON
		vnum = kUserDomain;
		#else
		vnum = kOnSystemDisk;
		#endif
	else {
		if (!pathtovolume (bsvol, &vnum))
			return (false);
		}
	
	setoserrorparam (bsfolder);
	
	if (stringlength (bsfolder) == sizeof (OSType)) /*received folder type code*/
		stringtoostype (bsfolder, &foldertype);
	
	else {
		firstword (bsfolder, chspace, bsfirst);
		
		if (findstringlist (bsfirst, specialfolderlistnumber, &ixlist))
		{
			foldertype = specialfolders [ixlist];
			#if TARGET_API_MAC_CARBON
			//temp items, we want to redirect to the users domain.
			//if we don't do this then it is a read only directory at the root.
			if (ixlist == 11)
				vnum = kUserDomain;
			#endif
		}
		else
			errcode = dirNFErr;
	}
	
	if (errcode == noErr)
		errcode = FindFolder (vnum, foldertype, flcreate, &vnum, &dirid);

	
	if (errcode == noErr) {
		
		errcode = FSMakeFSSpec (vnum, dirid, nil, fs);
		
		/*
		if (!directorytopath (dirid, vnum, bspath)) /%shouldn't fail%/
			errcode = dirNFErr;
		*/
		}
	
	return (!oserror (errcode));
#endif

#ifdef WIN95VERSION
	int nFolder = -1;
	int res;
	ITEMIDLIST * il;
	// Global pointer to the shell's IMalloc interface.  
	LPMALLOC g_pMalloc = NULL;  


	if (equalidentifiers ("\x6" "system", bsfolder)) {
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
		}

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

	if(nFolder != -1) {
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
#endif
	} /*getspecialfolderpath*/


#ifdef MACVERSION
boolean ejectvol (const tyfilespec *fs) {

	/*
	6/x/91 mao
	*/

	#if TARGET_API_MAC_CARBON
	return false;
	#else
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
		
	if (!volinfo.flejectable)
		return (false);
		
	return (!oserror (Eject (nil, (*fs).vRefNum)));
	#endif
} /*ejectvol*/
#endif
	
	
boolean isejectable (const tyfilespec *fs, boolean *flejectable) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
		
	*flejectable = volinfo.flejectable;
	
	return (true);
	} /*isejectable*/

	
boolean getfreespace (const tyfilespec *fs, long *ctfreebytes) {

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

	copystring ("\x42" "Can't get volume information because an error of type ^0 occurred.", bserr);

	numbertostring (errnum, bserrnum);

	parsedialogstring (bserr, bserrnum, nil, nil, nil, bserr);

	shellerrormessage (bserr);
	}/*volumeinfoerror*/


boolean langgetextendedvolumeinfo (const tyfilespec *fs, double *totalbytes, double *freebytes) {

	/*
	6.1b16 AR: Return number of free bytes and total number of bytes for the volume.
	This code is supposed to work flawlessly with volumes over 2 GB.
	
	For the Mac version, we delegate the actual work to XGetVInfo in MoreFilesExtras.h.
	
	For the Windows version, we use GetFreeDiskSpaceEx if available. Otherwise we fall
	back to GetFreeDiskSpace. According to MSDN info that's probably neccessary if we
	run on Win95 Release 1 which didn't support volume sizes over 2 GB.
	
	flsupportslargevolumes is initialized in filestartup in fileverbs.c. (Win32 only)
	*/

	OSErr errnum = noErr;
	bigstring volname;
#ifdef MACVERSION
	UInt64 ui64totalbytes, ui64freebytes;
	#if !TARGET_API_MAC_CARBON
		short vrefnum;
	#endif
#endif

	*totalbytes = 0.0;

	*freebytes = 0.0;

	copystring (fsname(fs), volname);


/*	JES 12/09/2002 -- 9.1b2 -- Don't do this test, since it fails on builds made with CW8

	if (!fileisvolume (fs)) {
	
		bigstring bserr;

		copystring ("\x45" "Can't get volume information because \"^0\" is not a valid volume name.", bserr);

		parsedialogstring (bserr, volname, nil, nil, nil, bserr);

		shellerrormessage (bserr);
		
		return (false);
		}
*/

#ifdef MACVERSION

	 /* 2005-01-24 creedon - reversed free and total parameters to match FSGetVInfo and XGetVInfo functions < http://groups.yahoo.com/group/frontierkernel/message/846 > */
	#if TARGET_API_MAC_CARBON
		errnum = FSGetVInfo (fs->vRefNum, nil, &ui64freebytes, &ui64totalbytes);
	#else
		errnum = XGetVInfo (fs->vRefNum, nil, &vrefnum, &ui64freebytes, &ui64totalbytes);
	#endif

	if (errnum != noErr) {
		volumeinfoerror (errnum);
		return (false);
		}

	*totalbytes = (double) ui64totalbytes;
	*freebytes = (double) ui64freebytes;

#endif


#ifdef WIN95VERSION

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
#endif

	return (true);
	}/*getextendedvolumeinfo*/


boolean getvolumesize (const tyfilespec *fs, long *ctbytes) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*ctbytes = volinfo.cttotalbytes;
	
	return (true);
	} /*getvolumesize*/


boolean getvolumeblocksize (const tyfilespec *fs, long *ctbytes) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*ctbytes = volinfo.blocksize;
	
	return (true);
	} /*getvolumeblocksize*/
	
 	
boolean filesonvolume (const tyfilespec *fs, long *ctfiles) {

	/*
	6/x/91 mao
	*/

	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*ctfiles = volinfo.ctfiles;
	
	return (true);
	} /*filesonvolume*/


boolean foldersonvolume (const tyfilespec *fs, long *ctfolders) {
	
	tyvolinfo volinfo;
		
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*ctfolders = volinfo.ctfolders;
	
	return (true);
	} /*foldersonvolume*/


boolean isvolumelocked (const tyfilespec *fs, boolean *fllocked) {
	
	/*
	6/x/91 mao
	*/
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*fllocked = volinfo.fllocked;
	
	return (true);
	} /*isvolumelocked*/


boolean volumecreated (const tyfilespec *fs, unsigned long *createdate) {
	
	tyvolinfo volinfo;
	
	if (!filegetfsvolumeinfo (fs, &volinfo))
		return (false);
	
	*createdate = volinfo.timecreated;
	
	return (true);
	} /*volumecreated*/


boolean lockvolume (const tyfilespec *fs, boolean fllock) {
#ifdef MACVERSION
	//Code change by Timothy Paustian Sunday, June 25, 2000 9:19:49 PM
	//can't lock volumes in carbon.
	#if TARGET_API_MAC_CARBON == 1
	return false;
	#else
		
	short vnum;
	QHdrPtr vqtop;
	QElemPtr vqelem;
	
	vnum = (*fs).vRefNum;
	
	vqtop = GetVCBQHdr ();
	
	vqelem = (*vqtop).qHead;
	
	while (true) {
		
		if (((VCB *)vqelem)->vcbVRefNum == vnum) {
			
			if (fllock)
				BitSet (&(*(VCB *)vqelem).vcbAtrb, 0);
			else
				BitClr (&(*(VCB *)vqelem).vcbAtrb, 0);
			
			return (true);
			}
		
		if (vqelem == (*vqtop).qTail) { /*reached end of list -- shouldn't happen*/
			
			oserror (errorVolume); /*no such volume*/
			
			return (false);
			}
		
		vqelem = (*vqelem).qLink;
		
		}
		#endif//end non-carbon code
#endif
#ifdef WIN95VERSION
	return (false);
#endif
	} /*lockvolume*/


#ifdef WIN95VERSION
boolean findapplication (OSType creator, tyfilespec *fsapp) {
	
	/*
	5.0.1 dmb: implemented for Win using the registry
	*/

	byte bsextension [8];
	bigstring bsregpath, bsoptions;

	ostypetostring (creator, bsextension);

	poptrailingwhitespace (bsextension);

	insertchar ('.', bsextension);
	
	pushchar (chnul, bsextension);
	
//	copyctopstring ("software\\Microsoft\\Windows\\CurrentVersion", bsregpath);

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
	} /*findapplication*/
#endif


#ifdef MACVERSION
boolean unmountvolume (const tyfilespec *fs) {
	
	/*
	12/5/91 dmb
	*/
	
	return (!oserror (UnmountVol (nil, (*fs).vRefNum)));
	} /*unmountvolume*/


boolean drivenumtovolname (short drivenum, bigstring bsvol) {
	
	HParamBlockRec pb;
	
	clearbytes (&pb, sizeof (pb));
	
	pb.volumeParam.ioNamePtr = bsvol;
	
	pb.volumeParam.ioVRefNum = drivenum;
	//Code change by Timothy Paustian Sunday, June 25, 2000 9:21:31 PM
	//Updated call for carbon
	return (PBHGetVInfoSync (&pb) == noErr);
	
	/*
	return (GetVInfo (drivenum, bsvol, &vnum, &freebytes) == noErr);
	*/
	} /*drivenumtovolname*/


static boolean hasdesktopmanager (short vnum) {
	
	GetVolParmsInfoBuffer volparms;
	HParamBlockRec pb;
	
	pb.volumeParam.ioVRefNum = vnum;
	
	pb.volumeParam.ioNamePtr = nil;
	
	pb.ioParam.ioBuffer = (Ptr) &volparms;
	
	pb.ioParam.ioReqCount = sizeof (volparms);
	
	if (PBHGetVolParmsSync (&pb) != noErr)
		return (false);
	
	return ((volparms.vMAttrib & (1 << bHasDesktopMgr)) != 0);
	} /*hasdesktopmanager*/


boolean findapplication (OSType creator, tyfilespec *fsapp) {
	
	/*
	12/5/91 dmb.
	
	9/7/92 dmb: make two passes, skipping volumes mounted with a foreign 
	file system the first time through. note: if this turns out not to be 
	the best criteria, we could check for shared volumes instead by testing  
	vMLocalHand in hasdesktopmanager.
	
	also: since we don't maintain the desktop DB when we delete files, we 
	need to verify that the file we locate still exists. added fileexists 
	call before returning true.
	
	2.1b11 dmb: loop through all files in each db if necessary to find one 
	that's actually an application
	*/
	
	bigstring bsapp;
	DTPBRec dt;
	HParamBlockRec pb;
	OSType type;
	register OSErr errcode;
	boolean fltryremote = false;
	boolean flremote;
	
	clearbytes (&pb, sizeof (pb));
	
	while (true) {
		
		++pb.volumeParam.ioVolIndex;
		
		errcode = PBHGetVInfoSync (&pb);
		
		if (errcode == nsvErr) { /*index out of range*/
			
			if (fltryremote) /*we've tried everything*/
				return (false);
			
			pb.volumeParam.ioVolIndex = 0; /*reset*/
			
			fltryremote = true;
			
			continue;
			}
		
		if (errcode != noErr)
			return (false);
		
		flremote = pb.volumeParam.ioVFSID != 0; /*actually means foreign file system*/
		
		if (fltryremote != flremote)
			continue;
		
		if (!hasdesktopmanager (pb.volumeParam.ioVRefNum))
			continue;
		
		dt.ioNamePtr = NULL;
		dt.ioVRefNum = pb.volumeParam.ioVRefNum;
		
		if (PBDTGetPath (&dt) != noErr)
			return (false);
		
		dt.ioNamePtr = (StringPtr) &bsapp;
		dt.ioIndex = 0;
		dt.ioFileCreator = creator;
		
		for (dt.ioIndex = 0; ; ++dt.ioIndex) {
			
			if (PBDTGetAPPLSync (&dt) != noErr) /*go to next volume*/
				break;
			
			if (FSMakeFSSpec (pb.volumeParam.ioVRefNum, dt.ioAPPLParID, bsapp, fsapp) != noErr)
				continue;
			
			if (!getfiletype (fsapp, &type))
				return (false);
			
			if (type == 'APPL') /*desktop db can contain references to non-apps*/
				return (true);
			}
		
		/*
		if (PBDTGetAPPLSync (&dt) == noErr) {
			
			if (FSMakeFSSpec (pb.volumeParam.ioVRefNum, dt.ioAPPLParID, bsapp, fsapp) == noErr)
				return (true);
			}
		*/
		}
	} /*findapplication*/


#if 0

static boolean getdesktopdatabasepath (short vnum, DTPBRec *dt) {
	
	if (!hasdesktopmanager (vnum))
		return (false);
	
	dt.ioVRefNum = vnum;
	
	return (PBDTGetPath (&dt) == noErr);
	} /*getdesktopdatabasepath*/

#endif


boolean getfilecomment (const tyfilespec *fs, bigstring bscomment) {
	
	/*
	12/5/91 dmb
	
	2.1b1 dmb: set reqCount; w/out it, works locally but not remotely
	*/
	
	DTPBRec dt;
	
	clearbytes (&dt, sizeof (dt));
	
	setemptystring (bscomment); /*default return*/
	
	if (!surefile (fs))
		return (false);
	
	if (!hasdesktopmanager ((*fs).vRefNum))
		return (false);
	
	dt.ioVRefNum = (*fs).vRefNum;
	
	if (PBDTGetPath (&dt) != noErr)
		return (false);
	
	dt.ioNamePtr = (StringPtr) (*fs).name;
	
	dt.ioDirID = (*fs).parID;
	
	dt.ioDTBuffer = (Ptr) bscomment + 1;
	
	dt.ioDTReqCount = lenbigstring;
	
	if (PBDTGetCommentSync (&dt) != noErr)
		return (false);
	
	setstringlength (bscomment, dt.ioDTActCount);
	
	return (true);
	} /*getfilecomment*/


boolean setfilecomment (const tyfilespec *fs, bigstring bscomment) {
	
	/*
	12/5/91 dmb
	*/
	
	DTPBRec dt;
	
	clearbytes (&dt, sizeof (dt));
	
	if (!surefile (fs))
		return (false);
	
	if (!hasdesktopmanager ((*fs).vRefNum))
		return (false);
	
	dt.ioVRefNum = (*fs).vRefNum;
	
	if (PBDTGetPath (&dt) != noErr)
		return (false);
	
	dt.ioNamePtr = (StringPtr) (*fs).name;
	
	dt.ioDirID = (*fs).parID;
	
	dt.ioDTBuffer = (Ptr) bscomment + 1;
	
	dt.ioDTReqCount = stringlength (bscomment);
	
	if (PBDTSetCommentSync (&dt) != noErr)
		return (false);
	
	PBDTFlushSync (&dt);
	
	return (true);
	} /*setfilecomment*/


static boolean index2label (short ixlabel, bigstring bslabel) {
	
	/*
	new: pascal OSErr GetLabel(short labelNumber, RGBColor *labelColor, Str255 labelString)
       =  {0x303c, 0x050B, 0xABC9};
	*/
	
	Handle hlabel;
	
	setemptystring (bslabel);
	
	if ((ixlabel < 0) || (ixlabel > 7))
		return (false);
	
	hlabel = GetResource ('lstr', -16392 + ixlabel);
	
	if (hlabel == nil)
		return (false);
	
	copyheapstring ((hdlstring) hlabel, bslabel);
	
	return (true);
	} /*index2label*/


static boolean label2index (bigstring bslabel, short *ixlabel) {
	
	register short ix;
	Handle hlabel;
	
	for (ix = 0; ix < 8; ++ix) {
		
		hlabel = GetResource ('lstr', -16392 + ix);
		
		if (hlabel == nil)
			continue;
		
		if (RelString ((ptrstring) *hlabel, bslabel, false, false) == 0) {
			
			*ixlabel = ix;
			
			return (true);
			}
		}
	
	return (false);
	} /*label2index*/


boolean getfilelabel (const tyfilespec *fs, bigstring bslabel) {
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	return (index2label (info.ixlabel, bslabel));
	} /*getfilelabel*/


boolean setfilelabel (const tyfilespec *fs, bigstring bslabel) {
	
	CInfoPBRec pb;
	register short flags;
	short ixlabel;
	
	if (!label2index (bslabel, &ixlabel))
		return (false);
	
	if (!getmacfileinfo (fs, &pb)) 
		return (false);
	
	flags = pb.hFileInfo.ioFlFndrInfo.fdFlags;
	
	flags &= 0xFFF1; /*clear out old index*/
	
	flags |= ixlabel << 1; /*slam in new index*/
	
	pb.hFileInfo.ioFlFndrInfo.fdFlags = flags;
	
	return (setmacfileinfo (fs, &pb));
	} /*setfilelabel*/


boolean mountvolume (bigstring volumepath, bigstring username, bigstring password) {
	
	/*
	2/20/92 dmb: initial code.  this really should be "mount server volume";
	mounting a volume is something else, really.
	
	4/16/92 dmb: check for already-mounted error code, and treat it as no error.
	
	2.1b12 dmb: if volumepath is improperly formed, generate informative message 
	instead of failing silently
	*/
	
	AFPVolMountInfo info;
	bigstring bsitem;
	short ixdata;
	ParamBlockRec pb;
	OSErr errcode;
	
	clearbytes (&info, sizeof (info));
	
	ixdata = (char *) &info.AFPData - (char *) &info;
	
	info.media = AppleShareMediaType;
	
	if (isemptystring (username))
		info.uamType = kNoUserAuthentication;
	else
		info.uamType = kPassword;
	
	
	firstword (volumepath, ':', bsitem); /*get the zone name*/
	
	info.zoneNameOffset = ixdata;
	
	copystring (bsitem, (ptrstring) &info + ixdata);
	
	ixdata += stringsize (bsitem);
	
	
	nthword (volumepath, 2, ':', bsitem); /*get the server name*/
	
	info.serverNameOffset = ixdata;
	
	copystring (bsitem, (ptrstring) &info + ixdata);
	
	ixdata += stringsize (bsitem);
	
	
	nthword (volumepath, 3, ':', bsitem); /*get the volume name*/
	
	info.volNameOffset = ixdata;
	
	copystring (bsitem, (ptrstring) &info + ixdata);
	
	ixdata += stringsize (bsitem);
	
	
	info.userNameOffset = ixdata; /*set the user name*/
	
	copystring (username, (ptrstring) &info + ixdata);
	
	ixdata += stringsize (username);
	
	
	info.userPasswordOffset = ixdata; /*set the password*/
	
	copystring (password, (ptrstring) &info + ixdata);
	
	ixdata += stringsize (password);
	
	
	info.length = ixdata;
	
	
	clearbytes (&pb, sizeof (pb));
	
	pb.ioParam.ioBuffer = (Ptr) &info;
	
	errcode = PBVolumeMount (&pb);
	
	if (errcode == -5062) /*volume is already mounted*/
		return (true);
	
	return (!oserror (errcode));
	} /*mountvolume*/
#endif


#ifdef NEWFILESPECTYPE
boolean fileparsevolname (bigstring bspath, long *vnum, bigstring bsvol)
#else
boolean fileparsevolname (bigstring bspath, short *vnum, bigstring bsvol)
#endif
	{
	/*
	convert a full path, which might contain a volume name at the beginning
	to a path with no volume name, and it's associated volume number in vnum.
	
	example: "Roverª:MORE Work" will return with bspath = "MORE Work" and
	vnum = -2 (the Macintosh vrefnum for the second mounted drive).  
	
	this combination of information plugs nicely into a lot of the file 
	manager routines.
	
	2.1b8 dmb: handle drive numbers
	
	2.1b11 dmb: return the vol name in bsvol if non-nil, along with the vnum

	5.0.2 rab: initialize ix to zero
	*/
	
#ifdef MACVERSION
	
	short ix = 1;
	bigstring bsvolname;
	HParamBlockRec pb;
	short drivenum;
	bigstring bs;
	OSErr err;
	
	copystring (bspath, bs); /*work on a copy*/
	
	if (isemptystring (bs))
		return (false);
	
	if (!scanstring (chpathseparator, bs, &ix)) { /*no colon, the whole thing is a volname*/
		
		copystring (bs, bsvolname);
		
		pushchar (chpathseparator, bsvolname);
		
		setemptystring (bs);
		}
	else {
		midstring (bs, 1, ix, bsvolname); /*pick off the vol name and the colon*/
		
		deletestring (bs, 1, ix);
		}
	
	clearbytes (&pb, sizeof (pb));
	
	pb.volumeParam.ioNamePtr = bsvolname;
	
	pb.volumeParam.ioVolIndex = -1; /*force him to use the name pointer only*/
	//Code change by Timothy Paustian Sunday, June 25, 2000 9:21:59 PM
	//Updated call for carbon
	err = PBHGetVInfoSync (&pb);
	
	if (err != noErr) {
		
		setstringlength (bsvolname, stringlength (bsvolname) - 1); /*pop last char -- the colon*/
		
		if (isallnumeric (bsvolname) && stringtoshort (bsvolname, &drivenum)) { /*it's a number*/
			
			/*
			pb.volumeParam.ioNamePtr = nil;
			*/
			
			pb.volumeParam.ioVRefNum = drivenum;
			//Code change by Timothy Paustian Sunday, June 25, 2000 9:24:35 PM
			//updated for carbon
			err = PBHGetVInfoSync (&pb);
			}
		
		if (err != noErr)
			return (false);
		}
	
	*vnum = pb.volumeParam.ioVRefNum;
	
	if (bsvol != nil)
		copystring (bsvolname, bsvol);
#endif
	
#ifdef WIN95VERSION
	bigstring bsvolname, bs;
	short ix = 0;

	*vnum = 0;
	copystring (bspath, bs); /*work on a copy*/
	
	if (isemptystring (bs))
		return (false);
	
	if (!scanstring (':', bs, &ix)) { /*no colon, No volume?*/
		return (false);		
		}
	else {
		midstring (bs, 1, ix, bsvolname); /*pick off the vol name and the colon*/
		
		deletestring (bs, 1, ix);
		}
	
	if (bsvol != nil)
		{
		copystring (bsvolname, bsvol);
		
		nullterminate (bsvol);
		}

#endif
	return (true);
	} /*fileparsevolname*/



#ifdef MACVERSION

boolean fileresolvealias (tyfilespec *fs) {
	
	Boolean flfolder, flalias;
	OSErr err;
	
	setoserrorparam ((ptrstring) (*fs).name);
	
	err = ResolveAliasFile (fs, true, &flfolder, &flalias);
	
	return (!oserror (err));
	} /*fileresolvealias*/
#endif


boolean filefrompath (bigstring path, bigstring fname) {
	
	/*
	return all the characters to the right of the colon in the path.
	
	example: "Work Disk #1:MORE Work:Status Center" returns "Status Center".
	*/

	#ifdef MACVERSION	
		return (lastword (path, chpathseparator, fname));
	#endif

	#ifdef WIN95VERSION
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
	#endif
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


#ifdef MACVERSION
boolean getfileparentfolder (const tyfilespec *fs, tyfilespec *fsparent) {
	
	long dirid = (*fs).parID;
	
	if (isemptystring ((*fs).name) || (dirid == fsRtParID)) { /*null or disk spec*/
		
		clearbytes (fsparent, sizeof (tyfilespec));
		
		return (true);
		}
	
	return (!oserror (FSMakeFSSpec ((*fs).vRefNum, dirid, nil, fsparent)));
	} /*getfileparentfolder*/


#if 0

boolean getdefaultpath (bigstring bs) {

	ParamBlockRec pb;
	OSErr errcode;
	
	setstringlength (bs, 0);
	
	clearbytes (&pb, sizeof (pb));
		
	errcode = PBGetVolSync (&pb);
	
	if (oserror (errcode))
		return (false);
	
	return (filegetpath (pb.fileParam.ioVRefNum, bs));
	} /*getdefaultpath*/

#endif


#endif

boolean movefile (const tyfilespec *fs, const tyfilespec *fsto) {
#ifdef MACVERSION
	/*
	moves a file or folder speied in fs to the fcifolder specified by fsto.
	make sure that the new path is really a folder.
	
	8/2/91 dmb: corrected error message params
	
	3.0.2 dmb: setoserrorparam to source file before catmove
	*/

	CInfoPBRec pb; 
	
	setfserrorparam (fsto); /*in case error message takes a filename parameter*/
	
	if (!getmacfileinfo (fsto, &pb))
		return (false);
	
	if (!pb.dirInfo.ioFlAttrib & ioDirMask) { /*if newpath isn't a folder, get out*/
		
		oserror (errorParam); /*not the best error message, but...*/
		
		return (false);
		}
	
	setfserrorparam (fs); /*3.0.2*/
	
	return (!oserror (FSpCatMove (fs, fsto)));
#endif
	
#ifdef WIN95VERSION
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
#endif
	} /*movefile*/


void filenotfounderror (bigstring bs) {
	
	/*
	generate a "file not found" or a "folder not found" error
	*/
	
	OSErr errcode;
	
	setoserrorparam (bs);
#ifdef MACVERSION	
	if (lastchar (bs) == chpathseparator)
		errcode = errorDirNotFound;
	else
		errcode = errorFileNotFound;
#endif
#ifdef WIN95VERSION
	errcode = errorFileNotFound;
#endif
	oserror (errcode);
	} /*filenotfounderror*/


boolean surefile (const tyfilespec *fs) {
	
	boolean flfolder;
	
	if (fileexists (fs, &flfolder))
		return (true);
	
	filenotfounderror ((ptrstring) fsname (fs));
	
	return (false);
	} /*surefile*/


boolean renamefile (const tyfilespec *fs, bigstring bsnew) {

	#ifdef MACVERSION	
		/*
		we expect a full path specifying the file to be renamed.  bsnew must not
		hold a path -- it's the new name of the file.  the Mac OS wants a full path
		for the new name, it makes no sense to us.  but we are obedient!
 		
		6/x/91 mao: PBHRename used, will now rename folders
		
		7/30/91 dmb: reworked code to work properly with volumes and folder paths 
		ending in colons
		
		9/4/91 dmb: only look at filename part of bsnew
		
		12/1/92 dmb: if bsnew is a valid path (same folder), use it as is. this 
		fixes bug when it's a path to a folder.
		*/
		
		if (!surefile (fs)) /*file doesn't exist -- catch error*/
			return (false);
		
		filefrompath (bsnew, bsnew);
		
		setoserrorparam (bsnew); /*only likely errors from here on relate to new name*/
		
		if (oserror (FSpRename (fs, bsnew)))
			return (false);
		
		touchparentfolder (fs);
		
		return (true);
	#endif

	#ifdef WIN95VERSION
		char fn[300];
		bigstring bsfolder;

		if (!surefile (fs)) /*file doesn't exist -- catch error*/
			return (false);
		
		filefrompath (bsnew, bsnew);
		
		copystring (fsname (fs), fn);

		nullterminate (fn);
		
		folderfrompath (fn, bsfolder);
		
		pushstring (bsnew, bsfolder);

		nullterminate (bsfolder);

		if (MoveFile (stringbaseaddress (fn), stringbaseaddress(bsfolder)))
			return (true);
		
		winfileerror (fs);

		return (false);
	#endif
	} /*renamefile*/


boolean lockfile (const tyfilespec *fs) {

	#ifdef MACVERSION	
		setfserrorparam (fs); /*in case error message takes a filename parameter*/
		
		return (!oserror (FSpSetFLock (fs)));
	#endif

	#ifdef WIN95VERSION
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
	#endif
	} /*lockfile*/


boolean unlockfile (const tyfilespec *fs) {
#ifdef MACVERSION	
	setfserrorparam (fs);
	
	return (!oserror (FSpRstFLock (fs)));
#endif

#ifdef WIN95VERSION
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
#endif
	} /*unlockfile*/


boolean newfolder (const tyfilespec *fs) {
#ifdef MACVERSION	
	/*
	2.1b2 dmb: use filespecs.
	*/
	
	long dirid;
	
	setfserrorparam (fs);
	
	return (!oserror (FSpDirCreate (fs, smSystemScript, &dirid)));
#endif

#ifdef WIN95VERSION
	if (CreateDirectory (stringbaseaddress (fsname (fs)), NULL))
		return (true);
	
	winfileerror (fs);

	return (false);
#endif
	} /*newfolder*/


boolean newfile (const tyfilespec *fs, OSType creator, OSType filetype) {
#ifdef MACVERSION	
	setfserrorparam (fs);
	
	return (!oserror (FSpCreate (fs, creator, filetype, smSystemScript)));
#endif

#ifdef WIN95VERSION
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
#endif
	} /*newfile*/


#ifdef MACVERSION

boolean getfullfilepath (bigstring bspath) {
	
	FSSpec fs;
	
	setoserrorparam (bspath); /*in case error message takes a filename parameter*/
	
	if (oserror (FSMakeFSSpec (0, 0, bspath, &fs)))
		return (false);
	
	return (filespectopath (&fs, bspath));
	} /*getfullfilepath*/


boolean filemakespec (short vnum, long dirid, bigstring fname, ptrfilespec pfs) {

	OSErr ec;
	
	ec = FSMakeFSSpec (vnum, dirid, fname, pfs);
	
	return ((ec == noErr) || (ec == fnfErr));
	} /*filemakespec*/

#endif /*MACVERSION*/


boolean initfile (void) {

	/*
	call this before you do any setvol's.
	
	get me the volume id of the application that's running.
	
	useful for opening data files that are located in the same folder as
	the application itself.
	
	11/8/90 dmb: new implementation
	*/
	
	#ifdef MACVERSION
	
	initfiledialog ();
	
	#endif
	
	#ifdef flsystem6
	
	GetVol (nil, &applicationvolnum);
	
	#endif

#ifdef MACVERSION
	#ifdef flcomponent
	
	#if !TARGET_API_MAC_CARBON
	RememberA5 ();
	#endif /*for hook*/
	
	#endif
	
	applicationresnum = CurResFile ();
#endif	
	return (true);
	} /*initfile*/



