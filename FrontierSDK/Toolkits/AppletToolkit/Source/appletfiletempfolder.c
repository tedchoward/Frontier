
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Script.h>
#include <Folders.h>
#include "applet.h"
#include "appletfilesinternal.h"
#include "appletfiletempfolder.h"


boolean filegettempfolder (short sourcevnum, bigstring foldername, short *vnum, long *dirid) {
	
	FSSpec fs;
	CInfoPBRec pb;
	OSErr ec;
	
	ec = FindFolder (sourcevnum, kTemporaryFolderType, kCreateFolder, &fs.vRefNum, &fs.parID);
	
	if (ec != noErr)
		ec = FindFolder (kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &fs.vRefNum, &fs.parID);
	
	if (fileerror (nil, ec)) 
		return (false);
	
	ec = FSMakeFSSpec (fs.vRefNum, fs.parID, foldername, &fs);
	
	if (ec == fnfErr)
		ec = FSpDirCreate (&fs, smSystemScript, dirid);
			
	if (fileerror (nil, ec)) 
		return (false);
	
	if (!getmacfileinfo (&fs, &pb))
		return (false);
	
	*vnum = pb.dirInfo.ioVRefNum;
	
	*dirid = pb.dirInfo.ioDrDirID;
	
	return (true);
	} /*filegettempfolder*/


