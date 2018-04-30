
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


#import <Foundation/Foundation.h>

	#include "FSCopyObject.h"
	#include <sys/param.h> // 2006-08-11 creedon

		#include "SetUpA5.h"

	#define hasOpenDeny(volParms)		(((volParms).vMAttrib & (1L << bHasOpenDeny)) != 0)
	
	#ifndef __MOREFILESEXTRAS__
		enum { /*permissions used by MoreFile 1.1 code*/
			dmNone		= 0x0000,
			dmNoneDenyRd	= 0x0010,
			dmNoneDenyWr	= 0x0020,
			dmNoneDenyRdWr	= 0x0030,
			dmRd			= 0x0001,	/* Single writer, multiple readers; the readers */
			dmRdDenyRd		= 0x0011,
			dmRdDenyWr		= 0x0021,	/* Browsing - equivalent to fsRdPerm */
			dmRdDenyRdWr	= 0x0031,
			dmWr			= 0x0002,
			dmWrDenyRd		= 0x0012,
			dmWrDenyWr	= 0x0022,
			dmWrDenyRdWr	= 0x0032,
			dmRdWr		= 0x0003,	/* Shared access - equivalent to fsRdWrShPerm */
			dmRdWrDenyRd	= 0x0013,
			dmRdWrDenyWr	= 0x0023,	/* Single writer, multiple readers; the writer */
			dmRdWrDenyRdWr	= 0x0033	/* Exclusive access - equivalent to fsRdWrPerm */
			};
	#endif
	


#ifdef flsystem6

static short applicationvolnum = 0;

#endif

static short applicationresnum = -1;

static OSType specialfolders [] = {
	
	0, /*make 1-based*/
	
	'pref',
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


typedef struct tyfileinfo tyvolinfo;


void setfserrorparam ( const ptrfilespec fs ) {

	//
	// 2006-07-05 creedon; for Mac, FSRef-ized
	//
	
	
		bigstring bs;
		
		macgetfilespecnameasbigstring(fs, bs);
	
		setoserrorparam(bs);
		

	
	} // setfserrorparam


boolean endswithpathsep (bigstring bs) {
	/* return true if the ending character is a path sepatator character*/
	char ch;

	ch = getstringcharacter(bs, stringlength(bs)-1);

	if (ch == ':')
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
	
		CFStringRef machineName = CSCopyMachineName();
	
		fl = CFStringGetPascalString (machineName, bsname, sizeof (bigstring), kCFStringEncodingMacRoman);
	
		CFRelease(machineName);

		if (!fl)
			setemptystring (bsname);
			

	
	return (fl);
	
	} // getmachinename



	boolean foldertest ( FSRefParamPtr pb ) {
		
		//
		// return true if pb holds info describing a folder.
		//
		// 2006-06-25 creedon: FSRef-ized
		//
		// 1996-04-26 dmb: use mask, not BitTst (a toolbox call)
		//
		
		// return (BitTst (&pb->dirInfo.ioFlAttrib, 3));

		return ( ( pb -> catInfo -> nodeFlags & kFSNodeIsDirectoryMask ) != 0 );
		
		} // foldertest


	boolean foldertestcipbr (CInfoPBRec *pb) {
		
		/*
		return true if pb holds info describing a folder.
		
		4/26/96 dmb: use mask, not BitTst (a toolbox call)
		*/
		
		return ( ( pb -> dirInfo.ioFlAttrib & kioFlAttribDirMask ) != 0 );
		
		} // foldertest


	/*
	static void filebeachball (void) {
		
		//
		roll the beachball cursor if there is one.
		//
		
		if (beachballcursor ())
			rollbeachball ();
		} // filebeachball
	*/

	boolean getmacfileinfo ( const ptrfilespec fs, FSRefParamPtr pb, FSCatalogInfoPtr catinfo ) {
		
		/* 
		2.1b2 dmb: new fsspec-based version
		*/
		
		OSErr err;
		FSRef fsref;
		
		setfserrorparam ( fs ); // in case error message takes a filename parameter
					
		err = macgetfsref(fs, &fsref);
		
		if (err == noErr) {
		
			clearbytes(pb, sizeof(*pb));
			clearbytes(catinfo, sizeof(*catinfo));
			
			pb->catInfo = catinfo;
			
			pb->ref = &fsref;
			pb->whichInfo = kFSCatInfoGettableInfo;
			
			err = PBGetCatalogInfoSync(pb);
			
			pb->ref = NULL;		// fsref is a local about to disappear
			}
		
		return (!oserror(err));
		} /* getmacfileinfo */


	boolean getmacfileinfocipbr (const FSSpecPtr fs, CInfoPBRec *pb) {
		
		/* 
		2.1b2 dmb: new fsspec-based version
		*/
		
		setoserrorparam ((ptrstring) (*fs).name); /*in case error message takes a filename parameter*/
		
		clearbytes (pb, sizeof (*pb));
		
		pb->hFileInfo.ioNamePtr = (StringPtr) (*fs).name;
		
		pb->hFileInfo.ioVRefNum = (*fs).vRefNum;
		
		pb->hFileInfo.ioDirID = (*fs).parID;
		
		return (!oserror (PBGetCatInfoSync (pb)));
		} /*getmacfileinfocipbr*/


	static boolean setmacfileinfo ( const ptrfilespec fs, FSRefParam *pb ) {
		
		/* 
		2.1b2 dmb: new fsspec-based version
		*/
		
		OSErr err;
		FSRef fsref;
		
		setfserrorparam(fs); /*in case error message takes a filename parameter*/
		
		err = macgetfsref(fs, &fsref);
		
		if (err == noErr) {
			
			pb->ref = &fsref;
			
			pb->whichInfo = kFSCatInfoSettableInfo;

			err = PBSetCatalogInfoSync(pb);
			
			pb->ref = NULL;		// fsref is a local about to disappear
			}

		return (!oserror(err));
		} /* setmacfileinfo */


	static boolean touchparentfolder(const ptrfilespec fs) {
		
		//
		// touch the file date of the parent folder of fs
		//
		// 2006-06-18 creedon: FSRef-ized
		//
		
		tyfilespec fsfolder;
		
		if (macgetfilespecparent(fs, &fsfolder) != noErr)
			return (false);
		
		setfilemodified(&fsfolder, timenow());
		
		return (true);
		
		} // touchparentfolder






	static boolean getinfofromvolpb ( const FSVolumeInfoParamPtr pb, tyfileinfo *info ) {
		
		//
		// 2006-10-17 creedon: FSRef-ized
		//
		// 1993-09-21 dmb: for a volume, dirid should be fsRtParID, not fsRtDirID.
		//
		
		short drivenum;
		
		clearbytes ( info, sizeof ( tyfileinfo ) );
		
		( *info ).vnum = (*pb).ioVRefNum;
		
		( *info ).dirid = fsRtParID;
		
		( *info ).flvolume = true;
		
		( *info ).flfolder = true;
		
		/* date time */ {
		
			CFAbsoluteTime oCFTime;
			OSStatus status;
			
			 status = UCConvertUTCDateTimeToCFAbsoluteTime ( &( *( *pb ).volumeInfo ).createDate, &oCFTime );
			 
			 status = UCConvertCFAbsoluteTimeToSeconds ( oCFTime, &( *info ).timecreated );

			 status = UCConvertUTCDateTimeToCFAbsoluteTime ( &( *( *pb ).volumeInfo ).modifyDate, &oCFTime );
			 
			 status = UCConvertCFAbsoluteTimeToSeconds ( oCFTime, &( *info ).timemodified );
			 
			 }

		( *info ).flhardwarelock = BitTst ( &( *( *pb ).volumeInfo ).flags, 7 );
		
		if (( *info ).flhardwarelock)
			( *info ).fllocked = true;
		else
			( *info ).fllocked = BitTst ( &( *( *pb ).volumeInfo ).flags, 15 );
		
		( *info ).ctfreebytes = ( *( *pb ).volumeInfo ).blockSize * ( *( *pb ).volumeInfo ).freeBlocks;
		
		( *info ).cttotalbytes = ( *( *pb ).volumeInfo ).blockSize * ( *( *pb ).volumeInfo ).totalBlocks;
		
		( *info ).blocksize = ( *( *pb ).volumeInfo ).blockSize;
		
		( *info ).ctfiles = ( *( *pb ).volumeInfo ).fileCount;
		
		( *info ).ctfolders = ( *( *pb ).volumeInfo ).folderCount;
		
		drivenum = ( *( *pb ).volumeInfo ).driveNumber;
		
		//Code change by Timothy Paustian Sunday, June 25, 2000 9:17:36 PM
		//The below code is to figure out if a volume is ejectable. I don't think
		//you can do this in carbon, so just set it to false.

		( *info ).flejectable = false;
				
		return (true);
		
		} // getinfofromvolpb


	boolean filegetvolumeinfo ( short vnum, tyfileinfo *info ) {
		
		//
		// 1993-09-21 dmb: take vnum as parameter, not volname. otherwise, we can't distinguish between two vols w/the
		//			    same name.
		//
		// 1993-09-07 DW: determine if it's a network volume
		//
		
		FSVolumeInfoParam pb;
		FSVolumeInfo volinfo;
		
		clearbytes ( &pb, sizeof ( pb ) ); // init all fields to zero
		clearbytes ( &volinfo, sizeof ( volinfo ) ); // init all fields to zero
		
		pb.volumeInfo = &volinfo;
		
		pb.ioVRefNum = vnum;
		pb.whichInfo = kFSVolInfoGettableInfo;
		
		if ( oserror ( PBGetVolumeInfoSync ( &pb ) ) )
			return ( false );
		
		// clearbytes (info, sizeof (tyfileinfo)); /*init all fields to zero*/
		
		getinfofromvolpb ( &pb, info );
		
		/* network volume */ {
			
			HParamBlockRec lpb;
			GetVolParmsInfoBuffer buffer;
			OSErr err;
			
			clearbytes (&lpb, sizeof (lpb)); /*init all fields to zero*/
			
			lpb.ioParam.ioVRefNum = (*info).vnum;
			
			lpb.ioParam.ioBuffer = ( Ptr ) &buffer;
			
			lpb.ioParam.ioReqCount = sizeof (buffer);
			
			err = PBHGetVolParmsSync (&lpb);
		
			if (err == noErr) 
				( *info ).flremotevolume = (&buffer)->vMServerAdr != 0;
			}
		
		return (true);
		} /*filegetvolumeinfo*/


	void filegetinfofrompb ( FSRefParam *pb, tyfileinfo *info ) {
		
		//
		// 2007-06-11 creedon: fix for Mac OS X bundles/packages that are
		//				   applications not setting creator/type
		//
		//				   fix for flbundle not being set to true for Mac
		//				   OS X bundles/packages
		//
		//				   fix for file names that don't have extensions
		//
		// 2006-06-24 creedon: FSRef-ized
		//
		// 5.1.4 dmb: set finderbits for folders too.
		//
		// 1993-09-24 dmb: handle volumes here, combining vol info with root
		//			    directory folder info. I'm not sure if a volume
		//			    lock is always reflected in the root directory, so
		//			    don't set fllocked false if the attribute isn't set
		//			    in the pb. (it starts out cleared anyway.)
		//
		
		unsigned short finderbits;
		
		clearbytes ( info, sizeof ( tyfileinfo ) ); // init all fields to zero
		
		( *info ).dirid = ( *pb ).catInfo -> parentDirID;
		
		/* date time */ {
		
			CFAbsoluteTime oCFTime;
			OSStatus status;
			
			 status = UCConvertUTCDateTimeToCFAbsoluteTime ( &( *pb ).catInfo -> createDate, &oCFTime );
			 
			 status = UCConvertCFAbsoluteTimeToSeconds ( oCFTime, &info -> timecreated );

			 status = UCConvertUTCDateTimeToCFAbsoluteTime ( &( *pb ).catInfo -> contentModDate, &oCFTime );
			 
			 status = UCConvertCFAbsoluteTimeToSeconds ( oCFTime, &info -> timemodified );
			 
			 } // date time

		( *info ).vnum = ( *( *pb ).catInfo ).volume;
		
		if ( ( *info ).dirid == fsRtParID )
			filegetvolumeinfo ( ( *info ).vnum, info );
		else
			( *info ).flvolume = false;
			
		if ( ( pb -> catInfo -> nodeFlags & kFSNodeForkOpenMask ) != 0 ) // if it's a volume, fllocked may already be set
			( *info ).fllocked = true;
		else
			( *info ).fllocked = ( ( *pb ).catInfo -> nodeFlags & kFSNodeLockedMask ) != 0;
		
		( *info ).flfolder = ( ( pb -> catInfo -> nodeFlags & kFSNodeIsDirectoryMask ) != 0 );
		
		if ( ( *info ).flfolder ) {
		
			( *info ).flbusy = pb -> catInfo -> valence > 0; // folders are considered "busy" if there are any files within
												    // the folder
			
			( *info ).iconposition = ( ( FolderInfo * ) pb -> catInfo -> finderInfo ) -> location;
			
			if ( ! ( *info ).flvolume ) { // these aren't the same for a volume & its root dir
			
				( *info ).ctfiles = pb -> catInfo -> valence;
				
				} // if
				
			( *info ).folderview = ( tyfolderview ) 0; // I can't find a way to get this info from FSRefParamPtr, I thought
											   // about trying to fake a DInfo structure and getting it from there
											   // but it may not even have the right value, I did find a reference
											   // to DRMacWindowView // dinfo.frView >> 8;
			
			finderbits = ( ( FolderInfo * ) pb -> catInfo -> finderInfo ) -> finderFlags;
			
			}
			
		else { // fill in fields for a file, somewhat different format than a folder
		
			( *info ).ixlabel = ( ( ( FileInfo * ) pb -> catInfo -> finderInfo ) -> finderFlags & kColor ) >> 1;
			
			( *info ).flbusy = ( pb -> catInfo -> nodeFlags & kFSNodeForkOpenMask ) != 0;
			
			( *info ).filecreator = ( ( FileInfo * ) pb -> catInfo -> finderInfo ) -> fileCreator;
			
			( *info ).filetype = ( ( FileInfo * ) pb -> catInfo -> finderInfo ) -> fileType;
			
			( *info ).sizedatafork = pb -> catInfo -> dataLogicalSize;
			
			( *info ).sizeresourcefork = pb -> catInfo -> rsrcLogicalSize;
			
			( *info ).iconposition = ( ( FileInfo * ) pb -> catInfo -> finderInfo ) -> location;
			
			finderbits = ( ( FileInfo * ) pb -> catInfo -> finderInfo ) -> finderFlags;
			
			} // if
			
		 /* copy many of the finder bits into the record */ {
	
			( *info ).flalias = ( finderbits & kIsAlias ) != 0;
			
			( *info ).flbundle = ( finderbits & kHasBundle ) != 0;
			
			( *info ).flinvisible = ( finderbits & kIsInvisible ) != 0;
			
			( *info ).flstationery = ( finderbits & kIsStationery ) != 0;
			
			( *info ).flshared = ( finderbits & kIsShared ) != 0;
			
			( *info ).flnamelocked = ( finderbits & kNameLocked ) != 0;
			
			( *info ).flcustomicon = ( finderbits & kHasCustomIcon ) != 0;
			
			} // finder bits
			
		if ( ( *info ).flfolder ) {
		
			boolean flIsApplication, flIsBundle;
			
			LSIsApplication ( ( *pb ).ref, &flIsApplication, &flIsBundle );
			
			if ( flIsApplication || flIsBundle ) { // Mac OS X bundles/packages are not considered folders
			
				( *info ).flfolder = false;
				
				( *info ).flbundle = true;
				
				} // if
				
			if ( flIsApplication ) { // for Mac OS X bundles/packages that are applications we need to grab creator/type
			
				OSStatus status;
				LSItemInfoRecord itemInfo;
				
				status = LSCopyItemInfoForRef ( ( *pb ).ref, kLSRequestTypeCreator, &itemInfo );
				
				( *info ).filecreator = itemInfo.creator;
				
				( *info ).filetype = itemInfo.filetype;
			
				} // if
				
			}
			
		else {
		
			if ( ! ( *info ).filetype ) { // try to get from file extension
			
				bigstring bsext;
				LSItemInfoRecord iteminfo;
				OSStatus status;
				
				clearbytes ( &iteminfo, sizeof ( iteminfo ) );
				
				status = LSCopyItemInfoForRef ( ( *pb ).ref, kLSRequestExtension, &iteminfo );
				
				if ( iteminfo.extension != NULL )
					cfstringreftobigstring ( iteminfo.extension, bsext );
					
				if ( isemptystring ( bsext ) || ( stringlength ( bsext ) > 4 ) ) // no extension
				
					stringtoostype ( BIGSTRING("\x04" "????"), &( *info ).filetype );
				else
					stringtoostype ( bsext, &( *info ).filetype );
					
				} // if
				
			} // if
			
		} // filegetinfofrompb


	void filegetinfofrompbcipbr (CInfoPBRec *pb, tyfileinfo *info) {
		
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
		} /* filegetinfofrompbcipbr */


static boolean filegetfsvolumeinfo (const ptrfilespec fs, tyfileinfo *info) {


	clearbytes (info, sizeof (tyfileinfo)); // init all fields to zero

	
		long vnum;
		
		getfsvolume ( fs, &vnum );

		if ( ! filegetvolumeinfo ( vnum, info ) )
			return ( false );
			
		return ( true );
		

	
	} // filegetfsvolumeinfo




boolean filegetinfo (const ptrfilespec fs, tyfileinfo *info) {

	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	// 1993-09-24 dmb: for Mac, let filegetinfofrompb take care of volumes
	//


		FSRef fsref;
		FSCatalogInfo catinfo;
		FSRefParam pb;

		setfserrorparam(fs); // in case error message takes a filename parameter
		
		if (oserror(macgetfsref(fs, &fsref)))
			return (false);

		clearbytes(&pb, sizeof(pb));
			
		pb.catInfo = &catinfo;
		pb.ref = &fsref;
		pb.whichInfo = kFSCatInfoGettableInfo;
		
		if (oserror(PBGetCatalogInfoSync(&pb)))
			return (false);
			
		filegetinfofrompb(&pb, info);
		
		
	
	return (true);
	
	} // filegetinfo
		
	
boolean filegetvolumename (short vnum, bigstring volname) {
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

	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	// 1992-08-10 dmb: for Mac, created
	//
		
	
		FSCatalogInfo catinfo;
		FSRefParam pb;
		
		if ( ! getmacfileinfo ( fs, &pb, &catinfo ) ) 
			return ( false );
		
		if ( BitTst ( &pb.catInfo -> nodeFlags, 4 ) ) { // is directory
		
			if (flbundle)
			
				( ( FolderInfo * ) pb.catInfo -> finderInfo ) -> finderFlags |= kHasBundle; // turn on
				
			else
			
				( ( FolderInfo * ) pb.catInfo -> finderInfo ) -> finderFlags &= ~kHasBundle; // turn off
				
			}
		else {
		
			if (flbundle)
			
				( ( FileInfo * ) pb.catInfo -> finderInfo ) -> finderFlags |= kHasBundle; // turn on
				
			else
			
				( ( FileInfo * ) pb.catInfo -> finderInfo ) -> finderFlags &= ~kHasBundle; // turn off
		
			}
		
		return (setmacfileinfo (fs, &pb));
		

	
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
	
	
		FSRef fsref;
		
		if (oserror (macgetfsref (fs, &fsref)))
			return (false);
	
        @autoreleasepool {
            CFURLRef fileUrl = CFURLCreateFromFSRef(kCFAllocatorDefault, &fsref);
            [(NSURL *)fileUrl setResourceValue:@(!flvisible) forKey:NSURLIsHiddenKey error:NULL];
            CFRelease(fileUrl);
        }
		
		touchparentfolder (fs);
		
		
	
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
	
	*size = (SInt32) (info.sizedatafork + info.sizeresourcefork);
	
	return (true);
	} /*filesize*/
	
	
boolean fileisfolder (const ptrfilespec fs, boolean *flfolder) {

	//
	// 2006-10-01 creedon: Mac OS X bundles/packages are not considered folders
	//
	
	tyfileinfo info;

	
		// Mac OS X bundles/packages are not considered folders
	
		FSRef fsref;
		boolean flisapplication, flisbundle;
		
		if (macgetfsref (fs, &fsref) == noErr) {
			
			if (LSIsApplication (&fsref, &flisapplication, &flisbundle) == noErr) {

				if (flisapplication || flisbundle) {
				
					*flfolder = false;
					
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

	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//

		
		FSCatalogInfo catalogInfo;
		FSRef fsref;
			
		if (!fs->flags.flvolume)
			return (false);
		
		if (macgetfsref (fs, &fsref) != noErr)
			return (false);
			
		clearbytes (&catalogInfo, longsizeof (catalogInfo));

		if (FSGetCatalogInfo (&fsref, kFSCatInfoGettableInfo, &catalogInfo, NULL, NULL, NULL) != noErr)
			return (false);
			
		return (catalogInfo.parentDirID == fsRtParID);
	

	
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
		
		
		CFAbsoluteTime oCFTime;
		FSCatalogInfo catinfo;
		FSRefParam pb;
		OSStatus status;
		
		if ( ! getmacfileinfo ( fs, &pb, &catinfo ) ) 
			return ( false );
	
		status = UCConvertSecondsToCFAbsoluteTime ( datecreated, &oCFTime );
		
		status = UCConvertCFAbsoluteTimeToUTCDateTime ( oCFTime, &pb.catInfo -> createDate );
		
		status = UCConvertSecondsToCFAbsoluteTime ( datemodified, &oCFTime );
		
		status = UCConvertCFAbsoluteTimeToUTCDateTime ( oCFTime, &pb.catInfo -> contentModDate );
				
		return ( setmacfileinfo ( fs, &pb ) );
		
	

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


	


	boolean getfilepos (const ptrfilespec fs, Point *pt) {
		
		//
		// 2006-06-25 creedon: FSRef-ized
		//
		// 1991-06 mao: modified to work with new getmacilfeinfo/setmacfileinfo.
		//
		
		FSCatalogInfo catinfo;
		FSRefParam pb;
		
		if ( ! getmacfileinfo ( fs, &pb, &catinfo ) ) 
			return ( false );
		
		if (foldertest (&pb)) 
			*pt = ( ( FolderInfo * ) pb.catInfo -> finderInfo ) -> location;
		else	
			*pt = ( ( FileInfo * ) pb.catInfo -> finderInfo ) -> location;
		
		return (true);
		} /*getfilepos*/


	boolean setfilepos (const ptrfilespec fs, Point pt) {

		//
		// 2006-06-25 creedon: FSRef-ized
		//
		// 1991-06 mao: modified to work with new getmacilfeinfo/setmacfileinfo.
		//
		
		FSCatalogInfo catinfo;
		FSRefParam pb;
		
		if ( ! getmacfileinfo ( fs, &pb, &catinfo ) ) 
			return ( false );
		
		if (foldertest (&pb)) 
			( ( FolderInfo * ) pb.catInfo -> finderInfo ) -> location = pt;
		else
			( ( FileInfo * ) pb.catInfo -> finderInfo ) -> location = pt;
		
		return (setmacfileinfo (fs, &pb));
		} /*setfilepos*/
		
	
	boolean setfilemodified (const ptrfilespec fs, const long when) {
		
		/*
		7/31/91 dmb: created
		*/
		
		unsigned long datecreated, datemodified;
		
		if (!getfiledates (fs, &datecreated, &datemodified))
			return (false);
		
		return (setfiledates (fs, datecreated, when));
		} /*setfilemodified*/


	boolean setfiletype (const ptrfilespec fs, OSType filetype) {
		
		/*
		6/x/91 mao: modified to work with new getmacilfeinfo/setmacfileinfo.
		*/
		
		FSCatalogInfo catinfo;
		FSRefParam pb;
		
		// clearbytes ( &pb, sizeof ( pb ) );
		// clearbytes ( &catinfo, sizeof ( catinfo ) );
		
		// pb.catInfo = &catinfo;
		
		if ( ! getmacfileinfo ( fs, &pb, &catinfo ) ) 
			return ( false );
		
		if (foldertest (&pb)) 
			return (false);
		
		( ( FileInfo * ) pb.catInfo -> finderInfo ) -> fileType = filetype;
		
		return (setmacfileinfo (fs, &pb));
		} /*setfiletype*/


	boolean setfilecreator (const ptrfilespec fs, OSType filecreator) {
		
		/*
		6/x/91 mao: modified to work with new getmacilfeinfo/setmacfileinfo. Also, makes sure
		folders aren't operated on
		*/
		
		FSCatalogInfo catinfo;
		FSRefParam pb;
		
		if ( ! getmacfileinfo ( fs, &pb, &catinfo ) ) 
			return ( false );
			
		if (foldertest (&pb)) 
			return (false);
		
		( ( FileInfo * ) pb.catInfo -> finderInfo ) -> fileCreator = filecreator;
		
		return (setmacfileinfo (fs, &pb));
		} /*setfilecreator*/


	/*
	static boolean copyfork (hdlfilenum fsource, hdlfilenum fdest, Handle hbuffer) {
		
		//
		copy either the data fork or resource fork of the indicated file.
		
		return true iff the file copy was successful.
		
		5/19/92 dmb: call langbackgroundtask when a script is running to make it more 
		likely that we'll yield the processor to another app.  also, only allow 
		background tasks if fork is larger than a single buffer.  note that if a script 
		is copying a bunch of files in a loop, the interpreter is already allowing 
		backgrounding between files.
		//

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
				
			if (ctbytes == 0) // last read got no bytes
				return (true);
			
			HLock (hbuffer);
			
			errcode = FSWrite (fdest, &ctbytes, *hbuffer);
			
			HUnlock (hbuffer);
			
			if (oserror (errcode))
				return (false);
			
			if (ctbytes < buffersize) // copy of fork is finished
				return (true);
			
			if (flscriptrunning)
				langbackgroundtask (false);
			else
				shellbackgroundtask (); // give background processes a chance
			
			filebeachball (); // roll the beachball cursor if there is one
			} // while
		} // copyfork
	*/
		


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
	


	ctbytes &= ~(1024 - 1); /*fast round down to nearest multiple of 1024*/
	
	return (newhandle (ctbytes, hbuffer)); /*shouldn't fail at this point, but let's be sure*/
	} /*largefilebuffer*/



	OSStatus LSIsApplication (const FSRef *inRef, Boolean *outIsApplication, Boolean *outIsBundled) {

		LSItemInfoRecord info;
		OSStatus err;

		err = LSCopyItemInfoForRef (inRef, kLSRequestBasicFlagsOnly, &info);

		if (err == noErr) {
			*outIsApplication = (kLSItemInfoIsApplication & info.flags) != 0;
			*outIsBundled = (kLSItemInfoIsPackage & info.flags) != 0;
			}

		return (err);
		}/*LSIsApplication*/



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
	
	
		boolean flsourcefolder;
		FSRef fsrefsource;
		
		if ( ! fileisfolder ( fsource, &flsourcefolder ) )
			return ( false );
		
		if ( flsourcefolder ) {
		
			setfserrorparam ( fdest ); // in case error message takes a filename parameter		

			if ( ! newfolder ( fdest ) )
				return ( false );
				
			return ( true );
			}
		
		if ( oserror ( macgetfsref ( fsource, &fsrefsource ) ) )
			return ( false );
		
		if ( oserror ( FSCopyObject ( &fsrefsource, &fdest->ref, 0, kFSCatInfoNone, kDupeActionReplace, &fdest->name,
							false, false, NULL, NULL, NULL, NULL ) ) )
			return ( false );
			
		return ( true ); // the file copy was successful
	

	
	} // copyfile


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


	static boolean pathtovolume (bigstring bspath, short *vnum) {
	
		//
		// 2006-06-25 creedon: FSRef-ized
		//
		
		if (countwords (bspath, chpathseparator) > 1) {
			
			oserror (errorVolume); // no such volume
			
			return (false);
			}
		
		tyfilespec fs;
		
		if (!fileparsevolname (bspath, &fs )) {
			
			oserror (errorVolume); // no such volume
			
			return (false);
			}
		
		getfsvolume ( &fs, ( long * ) vnum );
		
		return (true);
		
		} // pathtovolume
	

//Timothy Paustian's comments
//warning the values returned by getspecialfolderpath on OS X are very different
//than classic Mac OS

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

	
		short vnum;
		short ixlist;
		OSType foldertype;
		long attrs;
		bigstring bsfirst;
		FSRef fsref;
		
		if (!(gestalt (gestaltFindFolderAttr, &attrs) && (attrs & (1 << gestaltFindFolderPresent))))
			return (false);
		
		if (isemptystring (bsvol))

			vnum = kUserDomain;

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

				//temp items, we want to redirect to the users domain.
				//if we don't do this then it is a read only directory at the root.
				if (ixlist == 11) {
			
					vnum = kUserDomain;
					}
				}
			else {
			
				oserror ( dirNFErr );
			
				return ( false );
				}
		}
		
		if ( oserror ( FSFindFolder ( vnum, foldertype, flcreate, &fsref ) ) )
			return ( false );

		if ( oserror ( macmakefilespec ( &fsref, fs ) ) )
			return ( false );
		
		return ( true );
		

	
	} // getspecialfolderpath


boolean ejectvol (const ptrfilespec fs) {
#pragma unused (fs)
	return false;
} /*ejectvol*/
	
	
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


	*totalbytes = 0.0;

	*freebytes = 0.0;


		 /*
		 2005-01-24 creedon - reversed free and total parameters to match FSGetVInfo and XGetVInfo functions
			< http://groups.yahoo.com/group/frontierkernel/message/846 >
		*/
		
		OSErr err = noErr;
		FSVolumeInfo volinfo;
		long vnum;
		
		getfsvolume ( fs, &vnum );
		
		err = FSGetVolumeInfo ( vnum, 0, NULL, kFSVolInfoSizes, &volinfo, NULL, NULL );

		if ( err != noErr ) {
		
			volumeinfoerror ( err );
			
			return ( false );
			}

		*totalbytes = (double) volinfo.totalBytes;
		*freebytes = (double) volinfo.freeBytes;



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
#pragma unused (fs, fllock)
	//Code change by Timothy Paustian Sunday, June 25, 2000 9:19:49 PM
	//can't lock volumes in carbon.
	return false;
	} /*lockvolume*/



boolean unmountvolume (const ptrfilespec fs) {
	
	//
	// 2006-06-25 creedon: replace UnmountVol with FSEjectVolumeSync
	//
	// 1991-12-05 dmb: created
	//
	
	long vnum;
	OptionBits flags = 0;
	pid_t dissenter;
	
	getfsvolume ( fs, &vnum );
	
	return ( ! oserror ( FSEjectVolumeSync ( vnum, flags, &dissenter ) ) );
	
	} // unmountvolume


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


#if 0

static boolean getdesktopdatabasepath (short vnum, DTPBRec *dt) {
	
	if (!hasdesktopmanager (vnum))
		return (false);
	
	dt.ioVRefNum = vnum;
	
	return (PBDTGetPath (&dt) == noErr);
	} /*getdesktopdatabasepath*/

#endif


boolean getfilecomment (const ptrfilespec fs, bigstring bscomment) {
	
	// 2009-09-06 aradke: FIXME: apparently, the proper way to get and set comments on OS X is to send Apple Events to the Finder (slow!?)
	//
	// 2006-06-25 creedon: minimally FSRef-ized
	//
	// 2.1b1 dmb: set reqCount; w/out it, works locally but not remotely
	//
	// 1991-12-05 dmb: created
	//	
	
	DTPBRec dt;
	FSSpec fss;
	
	clearbytes (&dt, sizeof (dt));
	
	setemptystring (bscomment); /*default return*/
	
	if (!surefile(fs))
		return (false);

	if (oserror (macgetfsspec (fs, &fss)))
		return (false);
		
	if (!hasdesktopmanager (fss.vRefNum))
		return (false);
	
	dt.ioVRefNum = fss.vRefNum;
	
	if (PBDTGetPath (&dt) != noErr)
		return (false);
	
	dt.ioNamePtr = (StringPtr) fss.name;
	
	dt.ioDirID = fss.parID;
	
	dt.ioDTBuffer = (Ptr) bscomment + 1;
	
	dt.ioDTReqCount = lenbigstring;
	
	if (PBDTGetCommentSync (&dt) != noErr)
		return (false);
	
	setstringlength (bscomment, dt.ioDTActCount);
	
	return (true);
	
	} // getfilecomment


boolean setfilecomment (const ptrfilespec fs, bigstring bscomment) {
	
	//
	// 2006-06-25 creedon: minimally FSRef-ized
	//
	// 2.1b1 dmb: set reqCount; w/out it, works locally but not remotely
	//
	// 1991-12-05 dmb: created
	//	
	
	DTPBRec dt;
	FSSpec fss;
	
	clearbytes (&dt, sizeof (dt));
	
	if (!surefile (fs))
		return (false);

	if (oserror (macgetfsspec (fs, &fss)))
		return (false);
	
	if (!hasdesktopmanager (fss.vRefNum))
		return (false);
	
	dt.ioVRefNum = fss.vRefNum;
	
	if (PBDTGetPath (&dt) != noErr)
		return (false);
	
	dt.ioNamePtr = (StringPtr) fss.name;
	
	dt.ioDirID = fss.parID;
	
	dt.ioDTBuffer = (Ptr) bscomment + 1;
	
	dt.ioDTReqCount = stringlength (bscomment);
	
	if (PBDTSetCommentSync (&dt) != noErr)
		return (false);
	
	PBDTFlushSync (&dt);
	
	return (true);
	
	} // setfilecomment


static boolean index2label (short ixlabel, bigstring bslabel) {
	
	/*
	2006-04-24 creedon: use GetLabel instead of looking for resources that don't exists on Mac OS X
				   deleted old code see revision 1268
	*/
	
	if ((ixlabel < 0) || (ixlabel > 7))
		return (false);
	
	RGBColor labelcolor; // we don't use the value we put into here
	
	if (GetLabel (ixlabel, &labelcolor, bslabel) != noErr)
		return (false); 
		
	return (true);
	} /* index2label */


static boolean label2index (bigstring bslabel, short *ixlabel) {
	
	/*
	2006-04-24 creedon: use GetLabel instead of looking for resources that don't exists on Mac OS X
				   deleted old code see revision 1268
	*/
	
	register short ix;
	RGBColor labelcolor; // we don't use the value we put into here
	bigstring bs;
	
	for (ix = 0; ix < 8; ++ix) {
		
		if (GetLabel (ix, &labelcolor, bs) != noErr)
		
			return (false);
			
		if (RelString (bslabel, bs, false, false) == 0) {
			
			*ixlabel = ix;
			
			return (true);
			}
		}
	
	return (false);
	} /* label2index */


boolean getfilelabel (const ptrfilespec fs, bigstring bslabel) {
	
	tyfileinfo info;
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	return (index2label (info.ixlabel, bslabel));
	} /*getfilelabel*/


boolean setfilelabel (const ptrfilespec fs, bigstring bslabel) {
	
	/*
	2006-04-24 creedon: factored most of the code into setfilelabelindex function
				   code deleted see revision 1268
	*/
	
	short ixlabel;
	
	if (!label2index (bslabel, &ixlabel))
		return (false);
	
	return (setfilelabelindex (fs, ixlabel, false));
	} /* setfilelabel */


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


short getfilelabelindex (const ptrfilespec fs, short *ixlabel) {

	/*
	2006-04-23 creedon: created, cribbed from getfilelabel function
	*/
	
	tyfileinfo info;
	short maptouserinterfaceindex [8] = {0, 7, 4, 6, 5, 3, 1, 2};
	
	if (!filegetinfo (fs, &info))
		return (false);
	
	*ixlabel = maptouserinterfaceindex [info.ixlabel];
	
	return (true);
	} /* getfilelabelindex */


boolean setfilelabelindex (const ptrfilespec fs, short ixlabel, boolean flmapfromuserinterfaceindex) {

	/*
	2006-04-24 creedon: created, cribbed from setfilelabel function
	*/
	
	FSCatalogInfo catinfo;
	FSRefParam pb;
	register short flags;

	if ((ixlabel < 0) || (ixlabel > 7))
		return (false);
	
	if ( ! getmacfileinfo ( fs, &pb, &catinfo ) ) 
		return ( false );
	
	flags = ( ( FileInfo * ) pb.catInfo -> finderInfo ) -> finderFlags;
	
	flags &= 0xFFF1; // clear out old index
	
	if (flmapfromuserinterfaceindex) {
		short mapfromuserinterfaceindex [8] = {0, 6, 7, 5, 2, 4, 3, 1};

		ixlabel = mapfromuserinterfaceindex [ixlabel];
		}
	
	flags |= ixlabel << 1; // slam in new index
	
	( ( FileInfo * ) pb.catInfo -> finderInfo ) -> finderFlags = flags;
	
	return (setmacfileinfo (fs, &pb));
	} /* setfilelabelindex */



boolean fileparsevolname ( bigstring bspath, ptrfilespec fs ) {

	//
	// convert a full path, which might contain a volume name at the beginning
	// to a path with no volume name, and it's associated volume number in vnum.
	//
	// example: "Roverª:MORE Work" will return with bspath = "MORE Work" and
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
	
		
		FSRef fsrefvolume;
		bigstring bs, bsvolname;
		short ix = 1;
		
		clearbytes ( fs, sizeof ( *fs ) );
		
		copystring (bspath, bs); // work on a copy
		
		if (isemptystring (bs))
			return (false);
		
		if (!scanstring (chpathseparator, bs, &ix)) { // no colon, the whole thing is a volname
			
			copystring (bs, bsvolname);
			
			pushchar (chpathseparator, bsvolname);
			
			setemptystring (bs);
			}
		else {
			midstring (bs, 1, ix, bsvolname); // pick off the vol name and the colon
			
			deletestring (bs, 1, ix);
			}
		
		stringreplaceall ( ':', '/', bsvolname );
		
		insertstring ( BIGSTRING ( "\x09" "/Volumes/" ), bsvolname );
		
		/* convert from Mac Roman to UTF-8 */ {
		
			CFStringRef csr = CFStringCreateWithPascalString ( kCFAllocatorDefault, bsvolname,
				kCFStringEncodingMacRoman );
		
			CFStringGetCString ( csr, ( char * ) bsvolname, sizeof ( bsvolname ), kCFStringEncodingUTF8 );
			
			CFRelease ( csr );
	
			}
		
		if ( FSPathMakeRef ( bsvolname, &fsrefvolume, NULL ) != noErr )
			return ( false );
		
		return ( macmakefilespec ( &fsrefvolume, fs ) == noErr );
		
	
	
	return ( true );

	} // fileparsevolname



	boolean fileresolvealias (ptrfilespec fs) {
	
		//
		// 2006-09-18 creedon: FSRef-ized
		//
		
		FSRef fsref;
		boolean flfolder, flalias;
		
		setfserrorparam (fs);
		
		if (oserror (macgetfsref (fs, &fsref)))
			return (false);
		
		return (!oserror (FSResolveAliasFile ( &fsref, true, &flfolder, &flalias)));
		} // fileresolvealias
	


boolean filefrompath (bigstring path, bigstring fname) {
	
	/*
	return all the characters to the right of the colon in the path.
	
	example: "Work Disk #1:MORE Work:Status Center" returns "Status Center".
	*/

		return (lastword (path, chpathseparator, fname));

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

	
		//
		// moves a file or folder specified in fs to the folder specified by fsto.  make sure that the new path is really a folder.
		//
		// 2006-10-23 creedon: use fileisfolder function to determine if fs is folder
		//
		// 3.0.2 dmb: setoserrorparam to source file before catmove
		//
		// 1991-08-02 dmb: corrected error message params
		//
		
		boolean flfolder;
		FSRef src, dst;
		
		(void) fileisfolder (fs, &flfolder);
		
		if (flfolder) {
			oserror (errorParam); // not the best error message, but...
			return (false);
			}
		
		setfserrorparam (fs);
		
		if (oserror (macgetfsref (fs, &src)) || oserror (macgetfsref (fsto, &dst)))
			return (false);
		
		return (!oserror (FSMoveObject (&src, &dst, NULL)));
		
		
	
	} /*movefile*/


void filenotfounderror (bigstring bs) {
	
	/*
	generate a "file not found" or a "folder not found" error
	*/
	
	OSErr errcode;
	
	setoserrorparam (bs);
	
	
		if (lastchar (bs) == chpathseparator)
			errcode = errorDirNotFound;
		else
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
		
		FSRef fsref;
		tyfsname name;
		
		if (!surefile (fs)) /*file doesn't exist -- catch error*/
			return (false);
		
		if (oserror (macgetfsref (fs, &fsref)))
			return (false);
		
		filefrompath (bsnew, bsnew);
		
		setoserrorparam (bsnew); /*only likely errors from here on relate to new name*/

		bigstringtofsname (bsnew, &name);
		
		if (oserror (FSRenameUnicode (&fsref, name.length, name.unicode, kTextEncodingUnknown, NULL)))
			return (false);
		
		touchparentfolder (fs);
		
		return (true);

	} /*renamefile*/


boolean lockfile ( const ptrfilespec fs ) {

	//
	// 2006-06-13 creedon: for Mac, FSRef-ized
	//

	
		FSRef fsref;
		FSCatalogInfo catinfo;
		
		clearbytes (&catinfo, sizeof (catinfo));
	
		setfserrorparam (fs); // in case error message takes a filename parameter
		
		if (oserror (macgetfsref (fs, &fsref)))
			return (false);
		
		if (oserror (FSGetCatalogInfo (&fsref, kFSCatInfoNodeFlags, &catinfo, NULL, NULL, NULL)))
			return (false);
		
		catinfo.nodeFlags |= kFSNodeLockedMask;
		
		if (oserror (FSSetCatalogInfo (&fsref, kFSCatInfoNodeFlags, &catinfo)))
			return (false);
		
		return (true);
	

	
	} // lockfile


boolean unlockfile (const ptrfilespec fs) {

	//
	// 2006-06-13 creedon: for Mac, FSRef-ized
	//

	
		FSRef fsref;
		FSCatalogInfo catinfo;
		
		clearbytes (&catinfo, sizeof (catinfo));
	
		setfserrorparam (fs); // in case error message takes a filename parameter
		
		if (oserror (macgetfsref (fs, &fsref)))
			return (false);
			
		if (oserror (FSGetCatalogInfo (&fsref, kFSCatInfoNodeFlags, &catinfo, NULL, NULL, NULL)))
			return (false);
		
		catinfo.nodeFlags &= ~kFSNodeLockedMask;
		
		if (oserror (FSSetCatalogInfo (&fsref, kFSCatInfoNodeFlags, &catinfo)))
			return (false);
		
		return (true);
		

	
	} // unlockfile


boolean newfolder ( const ptrfilespec fs ) {

	//
	// 2006-06-18 creedon: for Mac, FSRef-ized
	//
	// 2.1b2 dmb: for Mac use filespecs.
	//
		
		
		setfserrorparam (fs);
		
		if (oserror (FSCreateDirectoryUnicode (&fs->ref, fs->name.length, fs->name.unicode, kFSCatInfoNone, NULL, NULL, NULL, NULL)))
			return (false);
		
		return (true);
				

	
	} // newfolder


boolean newfile ( const ptrfilespec fs, OSType creator, OSType filetype ) {

	//
	// 2009-09-03 aradke: switched tyfilespec.name field from CFStringRef to HFSUniStr255
	//
	// 2007-08-01 creedon: if path is NULL oserror
	//				   set err to bdNamErr by default
	//
	// 2006-06-18 creedon: for Mac, FSRef-ized
	//
	
		
		setfserrorparam (fs);

		return (!oserror (FSCreateFileUnicode (&fs->ref, fs->name.length, fs->name.unicode, kFSCatInfoNone, NULL, NULL, NULL)));
		
	
	
	} // newfile



	boolean getfullfilepath (bigstring bspath) {
	
		//
		// 2006-06-25 creedon: FSRef-ized
		//
		
		tyfilespec fs;
		FSRef fsref;
		
		setoserrorparam (bspath); /*in case error message takes a filename parameter*/
		
		insertstring (BIGSTRING ("\x09" "/Volumes/"), bspath);
		
		stringreplaceall (':', '/', bspath);
			
		convertpstring (bspath);
			
		if (oserror (FSPathMakeRef (bspath, &fsref, NULL)))
			return (false);
		
		if (oserror (macmakefilespec (&fsref, &fs)))
			return (false);
		
		return (filespectopath (&fs, bspath));
		
		} // getfullfilepath


	boolean filemakespec (short vnum, long dirid, bigstring fname, ptrfilespec pfs) {
	
		//
		// 2006-06-25 creedon: FSRef-ized
		//

		FSRef fsref;
		
		FSRefParam pb;
		pb.ioVRefNum = vnum;
		pb.ioDirID = dirid;
		pb.ioNamePtr = fname;
		pb.newRef = &fsref;
		
		OSErr result = PBMakeFSRefSync(&pb);
		
		if (noErr != result)
			return (false);
		
		return (macmakefilespec (&fsref, pfs) == noErr);
		
		} // filemakespec
		


boolean initfile (void) {

	/*
	call this before you do any setvol's.
	
	get me the volume id of the application that's running.
	
	useful for opening data files that are located in the same folder as
	the application itself.
	
	11/8/90 dmb: new implementation
	*/
	
	
	initfiledialog ();
	
	
	#ifdef flsystem6
	
	GetVol (nil, &applicationvolnum);
	
	#endif

	
	
	
	applicationresnum = CurResFile ();
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
	
		
		FSRef fsref;
	
		if ((UInt32) LSFindApplicationForInfo == (UInt32) kUnresolvedCFragSymbolAddress)
			return (false);
		
		if (LSFindApplicationForInfo (creator, NULL, NULL, &fsref, NULL) != noErr)
			return (false);
		
		return (macmakefilespec (&fsref, fsapp) == noErr);
		
	
	
	} // findapplication


void clearfilespec (ptrfilespec fs) {

	/*
	2009-09-04 aradke: init to nil filespec
	*/

	clearbytes (fs, sizeof (tyfilespec));
	
	} /*clearfilespec*/



	boolean macfilespecisvalid (const ptrfilespec fs) {
		
		/*
		2009-09-06 aradke: check if fs data is valid (though file may not exist)
		*/
		
		
		
		return (noErr == FSGetCatalogInfo(&fs->ref, kFSCatInfoNone, NULL, NULL, NULL, NULL) && (fsnamelength (&fs->name) > 0));

		} /*macfilespecisvalid*/
	
	
	boolean macfilespecisresolvable (const ptrfilespec fs) {
		
		FSRef fsref;
		
		return (macgetfsref (fs, &fsref) == noErr);
		} /*macfilespecisresolvable*/
	
	
	OSErr macgetfsref (const ptrfilespec fs, FSRef* fsref) {
		
		/*
		2009-09-05 aradke: obtain the actual FSRef corresponding to our extended filespec.
					if the file does not exist, we return an error.
		*/
		
		if (!macfilespecisvalid (fs))
			return (fnfErr);
		
		if (fs->flags.flvolume) {
		
			*fsref = fs->ref;
		
			return (noErr);
			}
		
		return (FSMakeFSRefUnicode (&fs->ref, fs->name.length, fs->name.unicode, kTextEncodingUnknown, fsref));
		} /*macgetfsref*/
	
	
	OSErr macmakefilespec (const FSRef *fsrefptr, ptrfilespec fs) {
		
		/*
		2009-09-06 aradke: obtain the filespec corresponding to the fsref.
		*/
		
		FSRef fsref = *fsrefptr;	// local copy in case fsrefptr points to fs->ref
		FSCatalogInfo catalogInfo;
		OSErr err;
		
		clearfilespec (fs);
		
		clearbytes (&catalogInfo, longsizeof(catalogInfo));

		err = FSGetCatalogInfo (&fsref, kFSCatInfoGettableInfo, &catalogInfo, NULL, NULL, NULL);
		
		if (err != noErr)	
			return (err);
		
		if (catalogInfo.parentDirID == fsRtParID) {
		
			fs->flags.flvolume = true;
			
			fs->ref = fsref;
			
			return (FSGetCatalogInfo (&fsref, kFSCatInfoNone, NULL, &fs->name, NULL, NULL));
			}

		return (FSGetCatalogInfo (&fsref, kFSCatInfoNone, NULL, &fs->name, NULL, &fs->ref));
		} /*macmakefilespec*/
	
	
	OSErr macgetfsspec (const ptrfilespec fs, FSSpec *fss) {
		
		/*
		2009-09-06 aradke: get an old-style FSSpec corresponding to our filespec.
					FSSpecs are seriously deprecated on OS X, but we still use them in a couple of places.
		*/
		
		FSRef fsref;
		FSCatalogInfo catalogInfo;
		OSErr err;
		
		if (macgetfsref (fs, &fsref) == noErr)	// file (already) exists
			return (FSGetCatalogInfo (&fsref, kFSCatInfoNone, NULL, NULL, fss, NULL));
		
		err = FSGetCatalogInfo (&fs->ref, kFSCatInfoVolume | kFSCatInfoNodeID, &catalogInfo, NULL, NULL, NULL);
		
		if (err != noErr)
			return (err);

		fss->vRefNum = catalogInfo.volume;
		fss->parID = catalogInfo.nodeID;
		fsnametobigstring (&fs->name, fss->name);	// FIXME: what if fs->name is longer than fss->name has room for?

		return (noErr);
		} /*macgetfsspec*/
		
		
	OSErr macgetfilespecparent (const ptrfilespec fs, ptrfilespec fsparent) {
		
		/*
		2009-09-05 aradke: obtain parent filespec. file does not have to exist.
					parent of volume or nil filespec is nil filespec.
		*/
		
		if (!macfilespecisvalid (fs) || fs->flags.flvolume) {

			clearfilespec (fsparent);	// default result

			return (noErr);
			}
		
		return (macmakefilespec (&fs->ref, fsparent));
		} /*macgetfilespecparent*/
	
	
	OSErr macgetfilespecchild (const ptrfilespec fs, tyfsnameptr name, ptrfilespec fschild) {
		
		/*
		2009-09-13 aradke: dive into a folder
		*/
		
		FSRef fsref;
		boolean flfolder;
		OSErr err;
		
		if (!fileisfolder (fs, &flfolder) || !flfolder)
			return (errFSNotAFolder);
		
		err = macgetfsref (fs, &fsref);
		
		if (err != noErr)
			return (err);
		
		clearfilespec (fschild);
		
		fschild->ref = fsref;
		
		copyfsname (name, &fschild->name);
		
		return (noErr);
		} /*macgetfilespecchild*/
	
	
	OSErr macgetfilespecchildfrombigstring (const ptrfilespec fs, bigstring bsname, ptrfilespec fschild) {
		
		/*
		2009-09-13 aradke: see macgetfilespecchild
		*/
		
		tyfsname name;
		
		bigstringtofsname (bsname, &name);
		
		return (macgetfilespecchild (fs, &name, fschild)); 
		} /*macgetfilespecchildfrombigstring*/
		
		
	boolean macgetfsrefname (const FSRef *fsref, tyfsnameptr fsname) {
		
		/*
		2009-09-05 aradke: obtain name of referenced file.
		*/

		return (FSGetCatalogInfo (fsref, kFSCatInfoNone, NULL, fsname, NULL, NULL) == noErr);

		} /*macgetfsrefname*/
	
	
	boolean macgetfsrefnameasbigstring (const FSRef *fsref, bigstring bs) {
		
		/*
		2009-09-05 aradke: obtain name of referenced file as pascal string.
		*/
		
		tyfsname fsname;
		
		if (!macgetfsrefname (fsref, &fsname))
			return (false);
		
		fsnametobigstring (&fsname, bs);
		
		return (true);
		} /*macgetfsrefnameasbigstring*/
	
	
	void macgetfilespecname (const ptrfilespec fs, tyfsnameptr fsname) {
		
		/*
		2009-09-05 aradke: obtain name of specified file.
		*/
		
		if (macfilespecisvalid (fs)) {
			copyfsname (&fs->name, fsname);
			}
		else {
			fsname->length = 0;
			}
		} /*macgetfilespecname*/
	
	
	void macgetfilespecnameasbigstring (const ptrfilespec fs, bigstring bs) {
		
		/*
		2009-09-05 aradke: obtain name of specified file as pascal string.
		*/
		
		tyfsname fsname;
		
		macgetfilespecname (fs, &fsname);
		
		fsnametobigstring (&fsname, bs);
		
		} /*macgetfilespecnameasbigstring*/
		
	
	void clearfsname (tyfsnameptr fsname) {
	
		clearbytes (fsname, sizeof (tyfsname));
		
		} /*clearfsname*/
	
	
	void copyfsname (const tyfsnameptr fssource, tyfsnameptr fsdest) {
		
		moveleft (fssource, fsdest, sizeof (tyfsname));
		
		} /*copyfsname*/
		
		
	unsigned short fsnamelength (const tyfsnameptr fsname) {
		
		return (fsname->length);
		
		} /*hfsnamelength*/

	
	void bigstringtofsname (const bigstring bs, tyfsnameptr fsname) {
	
		/*
		2006-07-06 creedon; created
		*/
	
		CFStringRef csr = CFStringCreateWithPascalString (kCFAllocatorDefault, bs, kCFStringEncodingMacRoman);
		
		fsname->length = CFStringGetLength (csr);
		
		CFStringGetCharacters (csr, CFRangeMake(0, fsname->length), fsname->unicode);
		
		CFRelease (csr);
		
		} /*bigstringtofsname*/
	
	
	void fsnametobigstring (const tyfsnameptr fsname, bigstring bs) {
		
		CFStringRef csr;
		
		csr = CFStringCreateWithCharacters (kCFAllocatorDefault, fsname->unicode, fsname->length);
		
		cfstringreftobigstring (csr, bs);

		CFRelease (csr);

		} /*fsnametobigstring*/

	
	boolean cfstringreftobigstring (CFStringRef input, StringPtr output) {
	
		/*
		2006-08-08 creedon: created, cribbed from < http://developer.apple.com/carbon/tipsandtricks.html#CFStringFromUnicode >
		*/
		
		CFIndex usedBufLen;
		
		CFStringGetBytes (input, CFRangeMake (0, MIN(CFStringGetLength(input), 255)),
					kCFStringEncodingMacRoman, '^', false, &(output[1]), 255, &usedBufLen);
		
		output[0] = usedBufLen;
		
		if (output[0] == 0)
			return (false);
		
		return (true);
		
		} //cfstringreftobigstring
	


