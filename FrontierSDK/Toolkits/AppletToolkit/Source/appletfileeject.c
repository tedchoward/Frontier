
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"
#include "appletfilesinternal.h"
#include "appletfileeject.h"


boolean fileejectdisk (ptrfilespec pfs) {
	
	short vnum = (*pfs).vRefNum;
	OSErr ec;
	
	ec = Eject (nil, vnum);

	if (fileerror (pfs, ec))
		return (false);
		
	ec = UnmountVol (nil, vnum);
	
	return (!fileerror (pfs, ec));
	} /*fileejectdisk*/
