
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Icons.h>
#include "appletdefs.h"
#include "appletmemory.h"
#include "appleticons.h"

unsigned short ttLabel [8] = {0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700};

#ifndef __ICONS__
#define svAllAvailableData 0xFFFFFFFF


pascal OSErr NewIconSuite (Handle *iconsuite) = {
	
	0x303C, 0x0207, 0xABC9};
	/*NewIconSuite*/
	
	
pascal OSErr AddIconToSuite (Handle iconbits, Handle iconsuite, ResType resid) = {

	0x303C, 0x0608, 0xABC9};
	/*AddIconToSuite*/
	
	
pascal OSErr DisposeIconSuite (Handle iconsuite, Boolean disposeData) = {
	
	0x303C, 0x0302, 0xABC9};
	/*DisposeIconSuite*/
	
	
pascal OSErr PlotIconSuite (const Rect *r, short align, short transform, Handle iconsuite) = {

	0x303C, 0x0603, 0xABC9};
	/*PlotIconSuite*/
	
	
pascal OSErr GetIconFromSuite (Handle *iconbits, Handle iconsuite, ResType resid) = {
	
	0x303C, 0x0609, 0xABC9};
	/*GetIconFromSuite*/


pascal OSErr PlotIconID (Rect *r, short align, short transform, short resid) = {

	0x303C, 0x0500, 0xABC9}; 
	/*PlotIconID*/


pascal OSErr GetIconSuite (Handle *theSuite, short theID, long theSelector) = {

	0x303C, 0x0501, 0xABC9}; 
	/*GetIconSuite*/
#endif

boolean geticonsuite (short id, Handle *iconsuite) {

	return (GetIconSuite (iconsuite, id, svAllAvailableData) == noErr);
	} /*geticonsuite*/
	
	
boolean newiconsuite (Handle *iconsuite) {

	return (NewIconSuite (iconsuite) == noErr);
	} /*newiconsuite*/
	
	
boolean addicontosuite (Handle iconbits, Handle iconsuite, ResType resid) {

	return (AddIconToSuite (iconbits, iconsuite, resid) == noErr);
	} /*addicontosuite*/
	
	
boolean disposeiconsuite (Handle iconsuite, Boolean disposeData) {
	
	if (iconsuite == nil)
		return (true);
		
	return (DisposeIconSuite (iconsuite, disposeData) == noErr);
	} /*disposeiconsuite*/
	
	
boolean ploticonsuite (Rect *r, short align, short transform, Handle iconsuite) {

	return (PlotIconSuite (r, align, transform, iconsuite) == noErr);
	} /*ploticonsuite*/
	
	
boolean geticonfromsuite (Handle *iconbits, Handle iconsuite, ResType resid) {
	
	return (GetIconFromSuite (iconbits, iconsuite, resid) == noErr);
	} /*geticonfromsuite*/


boolean ploticonresource (Rect *r, short align, short transform, short resid) {
	
	/*
	1.0b20 dmb: try plotting cicn if icon family isn't found. This 
	allows all of the stand system icons to be used.
	
	1.0b21 dmb: try geting normal b&w icon if all else fails
	*/
	
	OSErr ec;
	CIconHandle hcicon;
	Handle hicon;
	
	ec = PlotIconID (r, align, transform, resid);
	
	if (ec == noErr)
		return (true);
	
	hcicon = GetCIcon (resid);
	
	if (hcicon != nil) {
		
		PlotCIcon (r, hcicon);
		
		DisposeCIcon (hcicon);
		
		return (true);
		}
	
	hicon = GetIcon (resid);
	
	if (hicon != nil) {
		
		PlotIcon (r, hicon);
		
		/*ReleaseResource (hicon);*/ /*dmb 1.0b21 - don't need to*/
		
		return (true);
		}
	
	return (false);
	} /*ploticonresource*/


ResType icontypetorestype (short icontype) { 
	
	ResType restype = 0;
	
	switch (icontype) {
	
    	case kLargeIcon:
    		restype = 'ICN#'; break;
    		
  	 	case kLarge4BitIcon:
    		restype = 'icl4'; break;
  	 		
    	case kLarge8BitIcon:
    		restype = 'icl8'; break;
    		
    	case kSmallIcon:
    		restype = 'ics#'; break;
    		
    	case kSmall4BitIcon:
    		restype = 'ics4'; break;
    		
    	case kSmall8BitIcon:
    		restype = 'ics8'; break;
		} /*switch*/
		
	return (restype);
	} /*icontypetorestype*/
	
	
#ifndef coderesource /*DW 11/16/93: let the standalone IOA component build without link errors*/
	

	static boolean packone (Handle iconsuite, ResType type, Handle hpacked) {
		
		Handle iconbits;
		long handlesize;
		
		if (!geticonfromsuite (&iconbits, iconsuite, type))
			return (true);
			
		handlesize = GetHandleSize (iconbits);
			
		if (!enlargehandle (hpacked, longsizeof (type), &type))
			return (false);
			
		if (!enlargehandle (hpacked, longsizeof (handlesize), &handlesize))
			return (false);
			
		return (pushhandleonhandle (iconbits, hpacked));
		} /*packone*/
		
		
	boolean packiconsuite (Handle iconsuite, Handle *hpacked) {
		
		Handle h;
		
		if (!newclearhandle (0, &h))
			return (false);
		
		if (!packone (iconsuite, 'ICN#', h))
			goto error;
		
		if (!packone (iconsuite, 'icl4', h))
			goto error;
		
		if (!packone (iconsuite, 'icl8', h))
			goto error;
		
		if (!packone (iconsuite, 'ics#', h))
			goto error;
		
		if (!packone (iconsuite, 'ics4', h))
			goto error;
		
		if (!packone (iconsuite, 'ics8', h))
			goto error;
		
		*hpacked = h;
		
		return (true);
		
		error:
		
		disposehandle (h);
		
		return (false);
		} /*packiconsuite*/
		
		
	boolean unpackiconsuite (Handle hpacked, Handle *iconsuite) {
		
		Handle hsuite;
		long packedhandlesize;
		long ctbytes = 0;
		char *p;
		OSType type;
		long handlesize;
		Handle iconbits;
		
		if (!newiconsuite (&hsuite))
			return (false);
		
		packedhandlesize = GetHandleSize (hpacked);
		
		lockhandle (hpacked);
		
		p = *hpacked;
		
		while (true) {
			
			if (ctbytes >= packedhandlesize)
				break;
				
			BlockMove (p, &type, longsizeof (type));
			
			p += longsizeof (type);
			
			BlockMove (p, &handlesize, longsizeof (handlesize));
			
			p += longsizeof (handlesize);
			
			if (!newfilledhandle (p, handlesize, &iconbits))
				goto error;
			
			if (!addicontosuite (iconbits, hsuite, type))
				goto error;
			
			p += handlesize;
			
			ctbytes += longsizeof (type) + longsizeof (handlesize) + handlesize;
			} /*while*/
		
		unlockhandle (hpacked);
		
		*iconsuite = hsuite;
		
		return (true);
		
		error:
		
		disposeiconsuite (hsuite, true);
		
		return (false);
		} /*unpackiconsuite*/
		
#endif
	
	
			
	