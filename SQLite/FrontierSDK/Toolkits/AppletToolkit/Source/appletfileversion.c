
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"
#include "appletresource.h"
#include "appletfilesinternal.h"
#include "appletfileversion.h"


typedef struct lNumVersion {
	unsigned short majorRev1: 4; 		/*1st part of version number in BCD*/
	unsigned short majorRev2: 4; 		/*  2nd nibble of 1st part*/
	unsigned short minorRev : 4;		/*2nd part is 1 nibble in BCD*/
	unsigned short bugFixRev : 4; 		/*3rd part is 1 nibble in BCD*/
	unsigned short stage : 8;			/*stage code: dev, alpha, beta, final*/
	unsigned short nonRelRev1 : 4;		/*revision level of non-released version*/
	unsigned short nonRelRev2: 4;		/*  2nd nibble of revision level*/
	} lNumVersion;

typedef struct lVersRec {
	lNumVersion numericVersion;		/*encoded version number*/
	short countryCode;				/*country code from intl utilities*/
	Str255 shortVersion;			/*version number string - worst case*/
	Str255 reserved;				/*longMessage string packed after shortVersion*/
	} lVersRec, *lVersRecPtr, **lVersRecHndl;


static byte bsstages [] = "\pdab";	/*dev, alpha, beta*/

#define emptyversionsize ((long) sizeof (lNumVersion) + sizeof (short) + 2)


static boolean versionnumtostring (lNumVersion numvers, bigstring bs) {
	
	/*
	return the packed version number as a string, e.g. "1.0b2".  need 
	definitions above, which is mis-defined in the Think C headers
	*/
	
	setemptystring (bs);
	
	if (numvers.majorRev1 != 0)
		NumToString (numvers.majorRev1, bs);
	
	pushint (numvers.majorRev2, bs);
	
	pushchar ('.', bs);
	
	pushint (numvers.minorRev, bs);
	
	if (numvers.bugFixRev > 0) {
		
		pushchar ('.', bs);
		
		pushint (numvers.bugFixRev, bs);
		}
	
	if (numvers.stage < finalStage) {
		
		pushchar (bsstages [numvers.stage / developStage], bs);
		
		if (numvers.nonRelRev1 > 0)
			pushint (numvers.nonRelRev1, bs);
		
		pushint (numvers.nonRelRev2, bs);
		}
	
	return (true);
	} /*versionnumtostring*/
	
	
boolean filegetversion (FSSpec *pfs, bigstring version, bigstring position) {
	
	lNumVersion versionnumber;
	VersRecHndl hvers;
	short rnum;
	
	setstringlength (version, 0);
	
	setstringlength (position, 0);
	
	if (!openresourcefile (pfs, &rnum)) /*can't open resource fork of file*/
		return (false);
	
	if (getresourcehandle (rnum, 'vers', 2, (Handle *) &hvers)) {
	
		register byte *p;
		
		p = (**hvers).shortVersion;
		
		p += stringlength (p) + 1; /*skip to next contiguous string -- the long version*/
		
		copystring (p, position);
		
		disposehandle ((Handle) hvers);
		}
	
	if (getresource (rnum, 'vers', 1, longsizeof (versionnumber), &versionnumber))
		versionnumtostring (versionnumber, version);
		
	closeresourcefile (rnum);
	
	return (true);
	} /*filegetversion*/
	
	
