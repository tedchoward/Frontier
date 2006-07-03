
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define appletresource


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif

#ifndef filesinclude

	#include "appletfiles.h"
	
#endif


#define cancelbuttonstring 1 /*these are indexes used for getresourcestring calls*/


boolean getresourcestring (short, bigstring);

boolean openresourcefile (ptrfilespec, short *, short); /* 2005-09-02 creedon - added short for forktype*/

boolean closeresourcefile (short);

boolean getresource (short, OSType, short, long, void *);

boolean putresource (short, OSType, short, long, void *);

boolean getresourcehandle (short, OSType, short, Handle *);

boolean putresourcehandle (short, OSType, short, Handle);

boolean deleteresource (short, OSType, short);

boolean getstringlistitem (short, short, bigstring);
