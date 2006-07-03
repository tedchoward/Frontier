
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletprocess.h"
#include "appletmemory.h"
#include "appletfiles.h"
#include "appletfilesinternal.h"
#include "appletfiledesktop.h"



boolean hasdesktopmanager (short vnum) {
	
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


boolean findapplication (short vnum, OSType creator, boolean flappsonly, FSSpec *pfs) {
	
	/*
	dmb 9/21/93: keep looping until an APPL is found, or PBDTGetAPPLSync 
	returns an error. (it will eventually return afpItemNotFound.)
	
	dmb 9/24/93: added flappsonly parameter. a non-app, such as an extension 
	or desk accessory, can own a document type.
	*/
	
	DTPBRec dt;
	bigstring appname;
	tyfileinfo info;
	
	if (!hasdesktopmanager (vnum))
		return (false);
		
	clearbytes (&dt, longsizeof (dt));
		
	dt.ioVRefNum = vnum;
	
	if (PBDTGetPath (&dt) != noErr)
		return (false);
		
	dt.ioNamePtr = (StringPtr) &appname;
	
	dt.ioFileCreator = creator;
	
	for (dt.ioIndex = 0; ; ++dt.ioIndex) {
		
		if (PBDTGetAPPLSync (&dt) != noErr) 
			return (false);
		
		if (!filemakespec (vnum, dt.ioAPPLParID, appname, pfs))
			continue;
		
		if (!fileexists (pfs))
			continue;
		
		if (!filegetinfo (pfs, &info))
			continue;
		
		if (!flappsonly) /*dmb 9/24/93: some callers don't require apps*/
			return (true);
		
		if (info.filetype == 'APPL') /*DW 9/16/93: desktop db can contain references to non-apps*/
			return (true);
		}
	} /*findapplication*/


boolean findapponanydisk (OSType appid, FSSpec *fs) {
	
	/*
	find the app whose creator id is appid. return true if we found it, 
	false otherwise.
	*/
	
	DTPBRec dt;
	ParamBlockRec pb;
	Str255 appfname;
	
	clearbytes (&pb, (long) sizeof (pb));
	
	while (true) {
		
		++pb.volumeParam.ioVolIndex;
		
		if (PBGetVInfoSync (&pb) != noErr)
			return (false);
		
		if (!hasdesktopmanager (pb.volumeParam.ioVRefNum))
			continue;
		
		dt.ioNamePtr = NULL;
		
		dt.ioVRefNum = pb.volumeParam.ioVRefNum;
		
		if (PBDTGetPath (&dt) != noErr)
			return (false);
		
		dt.ioNamePtr = (StringPtr) &appfname;
		
		dt.ioIndex = 0;
		
		dt.ioFileCreator = appid;
		
		if (PBDTGetAPPLSync (&dt) == noErr) {
		
			if (FSMakeFSSpec (pb.volumeParam.ioVRefNum, dt.ioAPPLParID, appfname, fs) == noErr)
				return (true);
			}
		} /*while*/
		
	return (false);
	} /*findapponanydisk*/


boolean assureappisrunning (OSType serverid, boolean flbringtofront) {
	
	/*
	if the application whose creator id is serverid is running, return true.
	
	if not, we look for the application and try to launch it. we wait until it's 
	actually running and ready to receive Apple Events.
	*/
	
	typrocessinfo info;
	FSSpec fs;
	
	if (findrunningapp (serverid, &info)) /*already running, nothing to do*/
		return (true);
		
	if (!findapponanydisk (serverid, &fs))
		return (false);
		
	if (!launchappwithdoc (&fs, nil, flbringtofront))
		return (false);
		
	return (true);
	} /*assureappisrunning*/
	
	
boolean getfilecomment (FSSpec *pfs, bigstring bscomment) {
	
	DTPBRec dt;
	
	clearbytes (&dt, longsizeof (dt));
	
	setemptystring (bscomment); /*default return*/
	
	if (!hasdesktopmanager ((*pfs).vRefNum))
		return (false);
	
	dt.ioVRefNum = (*pfs).vRefNum;
	
	if (PBDTGetPath (&dt) != noErr)
		return (false);
	
	dt.ioNamePtr = (*pfs).name;
	
	dt.ioDirID = (*pfs).parID;
	
	dt.ioDTBuffer = (Ptr) bscomment + 1;
	
	dt.ioDTReqCount = lenbigstring;
	
	if (PBDTGetCommentSync (&dt) != noErr)
		return (false);
	
	setstringlength (bscomment, dt.ioDTActCount);
	
	return (true);
	} /*getfilecomment*/


boolean setfilecomment (FSSpec *pfs, bigstring bscomment) {
	
	DTPBRec dt;
	
	clearbytes (&dt, longsizeof (dt));
	
	if (!hasdesktopmanager ((*pfs).vRefNum))
		return (false);
	
	dt.ioVRefNum = (*pfs).vRefNum;
	
	if (PBDTGetPath (&dt) != noErr)
		return (false);
	
	dt.ioNamePtr = (*pfs).name;
	
	dt.ioDirID = (*pfs).parID;
	
	dt.ioDTBuffer = (Ptr) bscomment + 1;
	
	dt.ioDTReqCount = stringlength (bscomment);
	
	if (PBDTSetCommentSync (&dt) != noErr)
		return (false);
	
	PBDTFlushSync (&dt);
	
	return (true);
	} /*setfilecomment*/


boolean filecopycomment (FSSpec *fssource, FSSpec *fsdest) {

	bigstring bscomment;
	
	if (!getfilecomment (fssource, bscomment))
		return (false);
	
	return (setfilecomment (fsdest, bscomment));
	} /*filecopycomment*/


