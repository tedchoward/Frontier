
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletmemory.h"
#include "appletstrings.h"
#include "appletsyserror.h"


#define systemerrorlist 258 

#define ctsystemerrors 86


typedef struct tyerrorrecord {

	short errornumber; /*an error result passed back by a Mac system routine*/
	
	short errorindex; /*index into the STR# resource for the appropriate message*/
	} tyerrorrecord;
	

tyerrorrecord systemerrortable [ctsystemerrors] = {
	-33, 	 1,
	-34, 	 2,
	-35, 	 3,
	-36, 	 4,
	-37, 	 5,
	-38, 	 6,
	-39, 	 7,
	-40, 	 8,
	-41, 	 9,
	-42, 	10,
	-43, 	11,
	-44, 	12,
	-45, 	13,
	-46, 	14,
	-47, 	15,
	-48, 	16,
	-49, 	17,
	-50, 	18,
	-51, 	19,
	-52, 	20,
	-53, 	21,
	-54, 	22,
	-55, 	23,
	-56, 	24,
	-57, 	25,
	-58, 	26,
	-59, 	27,
	-60, 	28,
	-61, 	29,
	-64, 	30,
	-65, 	31,
	-66, 	32,
	-67, 	33,
	-68, 	34,
	-69, 	35,
	-70, 	36,
	-71, 	37,
	-72, 	38,
	-73, 	39,
	-74, 	40,
	-75, 	41,
	-76, 	42,
	-77, 	43,
	-78, 	44,
	-79, 	45,
	-80, 	46,
	-81, 	47,
	-120, 	48,
	-121, 	49,
	-122, 	50,
	-123, 	51,
	-192, 	52,
	-193, 	53,
	-194, 	54,
	-196, 	55,
	-198, 	56,
	-199, 	57,
	-100, 	58,
	-102, 	59,
	-108, 	60,
	-109, 	61,
	-111, 	62,
	-112, 	63,
	-117, 	64,
	-1703,	65,
	-1719,	66,
	-1704,	67,
	-1705,	68,
	-1708,	69,
	-1712,	70,
	-1711,	71,
	-906,	72,
	-1700,	73,
	-1701,	74,
	-1723,	75,
	-1728,	76,
	-5000,	77,
	-5016,	78,
	-5019,	79,
	-5023,	80,
	-5063,	81,
	-10000,	82,
	-10007,	83,
	-10011,	84,
	-10003,	85,
	-10014,	86,
	}; /*systemerrortable*/


#define unknownsystemerror ctsystemerrors + 1

#define errorcushionsize 0x1800 /*6K*/

static Handle herrorcushion = nil; /*a buffer to allow memory error reporting*/

static bigstring syserrorparam = "";




void setsyserrorparam (bigstring bs) {
	
	copystring (bs, syserrorparam);
	} /*setsyserrorparam*/


static boolean getsystemerrorstring (OSErr ec, bigstring bs) {
	
	/*
	ec is a result code returned by a Macintosh system routine.
	
	we search the systemerrortable to see if we have a string for the 
	error, if so we load the string from the resource file and return
	true.
	
	10/28/91 dmb: return false any time we don't find a specific error string
	*/
	
	short i;
	
	for (i = 0; i < ctsystemerrors; i++) {
	
		if (systemerrortable [i].errornumber == ec) {
			
			GetIndString (bs, systemerrorlist, systemerrortable [i].errorindex);
			
			if (stringlength (bs) > 0)
				return (true);
			}
		} /*for*/
	
	copystring ("\pMacintosh system error #", bs); /*create error string*/
		
	pushint (ec, bs);
	
	return (true); /*loop satisfied, no message for this error*/
	} /*getsystemerrorstring*/


static boolean getsafetycushion (void) {
	
	if (herrorcushion == nil)
		herrorcushion = appnewhandle (errorcushionsize);
	
	return (herrorcushion != nil);
	} /*getsafetycushion*/


boolean syserror (OSErr ec) {
	
	/*
	return true if ec is not noErr, false otherwise.
	
	we display a text string explaining any errors we find.
	*/
	
	bigstring bs;
	extern void alertdialog (bigstring);
	
	if (ec == noErr) /*there wasn't an error*/
		return (false); 
		
	if (ec == userCanceledErr) /*user has already interacted with the OS, it's still an error*/
		return (true);
	
	getsystemerrorstring (ec, bs);
	
	parseparamstring (bs, syserrorparam, nil, nil, nil, bs);
	
	alertdialog (bs);
	
	setstringlength (syserrorparam, 0);
	
	return (true); /*there was an error*/
	} /*syserror*/


boolean sysmemoryerror (void) {

	boolean fl;
	
	DisposHandle (herrorcushion); /*make room*/
	
	herrorcushion = nil;
	
	setstringlength (syserrorparam, 0);
	
	syserror (memFullErr);
	
	fl = getsafetycushion (); 
	
	assert (fl); /*unable to reclaim the safety cushion*/
	
	return (true);
	} /*sysmemoryerror*/


boolean initsyserror (void) {
	
	return (getsafetycushion ());
	} /*initsyserror*/




