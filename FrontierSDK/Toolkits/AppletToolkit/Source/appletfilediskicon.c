
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"
#include "appletfilesinternal.h"
#include "appletfilediskicon.h"


boolean filegetdiskicon (short vnum, Handle *hdiskicon) {

	HParamBlockRec pb;
	ParamBlockRec cpb;
	OSErr ec;

	clearbytes (&pb, longsizeof (pb));
    
	pb.volumeParam.ioVRefNum = vnum;
    
	ec = PBHGetVInfoSync (&pb);
    
	if (fileerror (nil, ec))
		return (false);
    	
	cpb.cntrlParam.ioCRefNum = pb.volumeParam.ioVDRefNum;
    
	cpb.cntrlParam.ioVRefNum = pb.volumeParam.ioVDrvInfo;

	cpb.cntrlParam.csCode = 22;
    
	ec = PBControlSync (&cpb);

	if (ec != noErr) {
    
		cpb.cntrlParam.csCode = 21;
        
		ec = PBControlSync (&cpb);
		}
    	
	if (fileerror (nil, ec))
    	return (false);

	ec = PtrToHand (*(Ptr*) &cpb.cntrlParam.csParam, hdiskicon, kLargeIconSize);
	
	return (!fileerror (nil, ec));
	} /*filegetdiskicon*/
	
