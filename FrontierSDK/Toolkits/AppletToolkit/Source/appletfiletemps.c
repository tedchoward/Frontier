
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Folders.h>
#include "applet.h"
#include "appletfilesinternal.h"
#include "appletfiletemps.h"


boolean filegettempspec (bigstring fname, FSSpec *fstemp) {
	
	short vnum;
	long dirid;
	OSErr ec;
	
	ec = FindFolder (kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &vnum, &dirid);
	
	if (fileerror (nil, ec)) 
		return (false);
	
	filemakespec (vnum, dirid, fname, fstemp);
	
	return (true);
	} /*filegettempspec*/


