
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Folders.h>
#include "applet.h"
#include "appletfilesinternal.h"
#include "appletfileprefs.h"


boolean filegetprefsspec (bigstring fname, FSSpec *fsprefs) {
	
	short vnum;
	long dirid;
	OSErr ec;
	
	ec = FindFolder (kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &vnum, &dirid);
	
	if (fileerror (nil, ec)) 
		return (false);
	
	filemakespec (vnum, dirid, fname, fsprefs);
	
	return (true);
	} /*filegetprefsspec*/


