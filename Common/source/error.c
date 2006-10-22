
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

#include "memory.h"
#include "strings.h"
#include "dialogs.h"
#include "ops.h"
#include "resources.h"
#include "shell.h"
#include "shellhooks.h"
#include "error.h"


#ifdef WIN95VERSION
	#define memFullErr -20000
	#define userCanceledErr -20001
#endif

#define systemerrorlist 258 

#define ctsystemerrors 92


struct {

	short errornumber; /*an error result passed back by a Mac system routine*/
	
	/***short errorindex; #*index into the STR# resource for the appropriate message*/
	}

#ifdef MACVERSION

systemerrortable [ctsystemerrors] = {
	
	{-33},	//dirFulErr
	{-34},	//dskFulErr
	{-35},	//nsvErr
	{-36},	//ioErr
	{-37},	//bdNamErr
	{-38},	//fnOpnErr
	{-39},	//eofErr
	{-40},	//posErr
	{-41},	//mFulErr
	{-42},	//tmfoErr
	{-43},	//fnfErr
	{-44},	//wPrErr
	{-45},	//fLckdErr
	{-46},	//vLckdErr
	{-47},	//fBsyErr
	{-48},	//dupFNErr
	{-49},	//opWrErr
	{-50},
	{-51},	//rfNumErr
	{-52},	//gfpErr
	{-53},	//volOffLinErr
	{-54},	//permErr
	{-55},	//volOnLinErr
	{-56},	//nsDrvErr
	{-57},	//noMacDskErr
	{-58},	//extFSErr
	{-59},	//fsRnErr
	{-60},	//badMDBErr
	{-61},	//wrPermErr
	{-64},	//noDriveErr
	{-65},	//offLinErr
	{-66},	//noNybErr
	{-67},	//noAdrMkErr
	{-68},	//dataVerErr
	{-69},	//badCksmErr
	{-70},	//badBtSlpErr
	{-71},	//noDtaMkErr
	{-72},	//badDCksum
	{-73},
	{-74},
	{-75},
	{-76},
	{-77},
	{-78},
	{-79},
	{-80},
	{-81},
	{-120},	//dirNFErr
	{-121},
	{-122},	//badMovErr
	{-123},	//wrgVolTypErr
	{-192},	//resNotFound
	{-193},	//resFNotFound
	{-194},	//addResFailed
	{-196},	//rmvResFailed
	{-198},	//resAttrErr
	{-199},	//mapReadErr
	{-100},
	{-102},
	{-108},	//memFullErr
	{-109},	//nilHandleErr
	{-111},
	{-112},
	{-117},
	{-1703},
	{-1719},
	{-1704},
	{-1705},
	{-1708},
	{-1712},
	{-1711},
	{-906},
	{-1700},
	{-1701},
	{-1723},
	{-1728},
	{-5000},
	{-5016},
	{-5019},
	{-5023},
	{-5063},
	{-10000},
	{-10007},
	{-10011},
	{-10003},
	{-10014},

	{errAENewerVersion},
	{errOSAScriptError},
	{errAECantHandleClass},
	{connectionInvalid},
	{noUserInteractionAllowed},
	{errOSASourceNotAvailable}
	
	}; /*systemerrortable*/

#endif

#ifdef WIN95VERSION

systemerrortable [ctsystemerrors] = {
	
	{-33},	//dirFulErr
	{-34},	//dskFulErr
	{-35},	//nsvErr
	{-36},	//ioErr
	{-37},	//bdNamErr
	{-38},	//fnOpnErr
	{-39},	//eofErr
	{-40},	//posErr
	{-41},	//mFulErr
	{ERROR_TOO_MANY_OPEN_FILES},	//tmfoErr
	{ERROR_FILE_NOT_FOUND},	//fnfErr
	{-44},	//wPrErr
	{-45},	//fLckdErr
	{-46},	//vLckdErr
	{ERROR_SHARING_VIOLATION},	//fBsyErr
	{-48},	//dupFNErr
	{-49},	//opWrErr
	{-50},
	{-51},	//rfNumErr
	{-52},	//gfpErr
	{-53},	//volOffLinErr
	{-54},	//permErr
	{-55},	//volOnLinErr
	{-56},	//nsDrvErr
	{-57},	//noMacDskErr
	{-58},	//extFSErr
	{-59},	//fsRnErr
	{-60},	//badMDBErr
	{-61},	//wrPermErr
	{-64},	//noDriveErr
	{-65},	//offLinErr
	{-66},	//noNybErr
	{-67},	//noAdrMkErr
	{-68},	//dataVerErr
	{-69},	//badCksmErr
	{-70},	//badBtSlpErr
	{-71},	//noDtaMkErr
	{-72},	//badDCksum
	{-73},
	{-74},
	{-75},
	{-76},
	{-77},
	{-78},
	{-79},
	{-80},
	{-81},
	{ERROR_PATH_NOT_FOUND},	//dirNFErr
	{-121},
	{-122},	//badMovErr
	{-123},	//wrgVolTypErr
	{-192},	//resNotFound
	{-193},	//resFNotFound
	{-194},	//addResFailed
	{-196},	//rmvResFailed
	{-198},	//resAttrErr
	{-199},	//mapReadErr
	{-100},
	{-102},
	{ERROR_OUTOFMEMORY},	//memFullErr
	{-109},	//nilHandleErr
	{-111},
	{-112},
	{-117},
	{-1703},
	{-1719},
	{-1704},
	{-1705},
	{-1708},
	{-1712},
	{-1711},
	{-906},
	{-1700},
	{-1701},
	{-1723},
	{-1728},
	{ERROR_ACCESS_DENIED},
	{-5016},
	{-5019},
	{-5023},
	{-5063},
	{-10000},
	{-10007},
	{-10011},
	{-10003},
	{-10014},
	
	}; /*systemerrortable*/

#endif

#define unknownsystemerror ctsystemerrors + 1

#define errorcushionsize 0x1800 /*6K*/

static Handle herrorcushion = nil; /*a buffer to allow memory error reporting*/

static OSErr lasterror = noErr;

/*
static bigstring lasterrormessage = "";
*/


boolean setoserrorparam (bigstring bs) {
	
	return (setparseparams (bs, nil, nil, nil));
	} /*setoserrorparam*/


OSErr getoserror (void) {
	
	return (lasterror);
	} /*getoserror*/

/*
void getoserrormessage (bigstring bs) {
	
	copystring (lasterrormessage, bs);
	} #*getoserrormessage*/


boolean getsystemerrorstring (OSErr errcode, bigstring bs) {

	/*
	errcode is a result code returned by a Macintosh system routine.
	
	we search the systemerrortable to see if we have a string for the 
	error, if so we load the string from the resource file and return
	true.
	
	10/28/91 dmb: return false any time we don't find a specific error string
	*/
	
	register short i;
	register short errnum = errcode;
	bigstring bsos;

	for (i = 0; i < ctsystemerrors; i++) 
	
		if (systemerrortable [i].errornumber == errnum) {
			
			getstringlist (systemerrorlist, i + 1, bs);
			
			return (!isemptystring (bs));
			}
	
	getstringlist (systemerrorlist, unknownsystemerror, bs); /*loop satisfied, no message for this error*/
	
	if (isemptystring (bs))
		return (false);
		
	#ifdef MACVERSION
		shorttostring (errcode, bsos);
	#endif

	#ifdef WIN95VERSION
		if (getwinerrormessage (errcode, bsos)) {
			
			firstword (bsos, '.', bsos); //skip the cr
			}
		else {
		
			/* THe format failed, therefore we have no further info */
			wsprintf (stringbaseaddress(bsos), "error number %ld." , errcode);
			
			setstringlength (bsos, strlen (stringbaseaddress(bsos)));
			}
	#endif

	parsedialogstring (bs, bsos, nil, nil, nil, bs);

	return (true);
	} /*getsystemerrorstring*/


static boolean getsafetycushion (void) {
	
	if (herrorcushion == nil)
		herrorcushion = NewHandle (errorcushionsize);
	
	return (herrorcushion != nil);
	} /*getsafetycushion*/


boolean memoryerror (void) {

	/*
	returns true if there was a memory error, detected by calling the Mac
	routine MemError.
	
	wired off because we need a fancier solution, when we're out of memory,
	there's no room in the heap for a dialog box!  must pre-allocate this
	one...
	
	1/31/91 dmb: we'll deal with the heap space problem by using herrorcushion.
	CouldDialog is also used to ensure that the resources are already in memory.
	
	3/4/91 dmb: now always generate an error message, and always return true
	*/
	
	register OSErr errcode = MemError ();
	bigstring bs;
	#ifdef WIN95VERSION
		long ctbytes = longinfinity;
		
		shellcallmemoryhooks (&ctbytes);
	#endif
	
	if (errcode == noErr) /*specific error code has been overwritten*/
		errcode = memFullErr;
	
	DisposeHandle (herrorcushion); /*make room*/
	
	herrorcushion = nil;
	
	getsystemerrorstring (errcode, bs);
	
	lasterror = errcode;
	
	/*
	copystring (bs, lasterrormessage);
	*/
	
	shellerrormessage (bs);
	
	getsafetycushion (); /*hopefully, we should be able to get it back now*/
	
	return (true);
	} /*memoryerror*/


boolean oserror (OSErr errcode) {
	
	/*
	return true if there is a Macintosh OS error, false otherwise.
	
	we display a text string explaining any errors we find.
	
	1/31/91 dmb: now call shellerrormessage instead of alertdialog
	
	2.1b5: don't post a message for user cancelled
	*/
	
	register OSErr x = errcode;
 	bigstring bs;
	
	lasterror = x;
	
	if (x == noErr) /*there wasn't an error*/
		return (false);
	
	if (x != userCanceledErr) {

		getsystemerrorstring (x, bs);
		
		if (isemptystring (bs)) { /*resource missing*/
		
			copystring ((ptrstring) STR_Mac_OS_Error, bs); /*create error string*/
			
			pushint (x, bs);
			}
		
		parseparamstring (bs);
		
		/*
		copystring (bs, lasterrormessage);
		*/
		
		shellerrormessage (bs);
		}
	
	setparseparams (nil, nil, nil, nil);
	
	return (true); /*there was an error*/
	} /*oserror*/


#ifdef WIN95VERSION

boolean winerror (void) {
	
	/*
	5.0.1 dmb: the callerk has already detected an error. we're
	just reporting it.
	*/

	oserror (GetLastError ());

	return (true);
	} /*winerror*/


boolean getwinerrormessage (OSErr err, bigstring bserr) {

	LPVOID lpMsgBuf;

	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL) == 0) {
		
		setemptystring (bserr);
		
		return (false);
		}
	
	strcpy (stringbaseaddress (bserr), lpMsgBuf);
	
	setstringlength (bserr, strlen(lpMsgBuf));
	
	LocalFree (lpMsgBuf);
	
	return (true);
	} /*getwinerrormessage*/

#endif


boolean initerror (void) {
	
	return (getsafetycushion ());
	} /*initerror*/

