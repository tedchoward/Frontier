
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletmemory.h"
#include "appletresource.h"



boolean openresourcefile (ptrfilespec pfs, short *rnum) {
	
	short resourcerefnum = -1;
	OSErr ec;
	
	resourcerefnum = HOpenResFile ((*pfs).vRefNum, (*pfs).parID, (*pfs).name, fsCurPerm);

	ec = ResError ();
	
	if (ec == -39) { /*eof error, file has no resource fork, create one*/
		
		HCreateResFile ((*pfs).vRefNum, (*pfs).parID, (*pfs).name);
		
		if (ResError () != noErr) /*failed to create resource fork*/
			goto error;
			
		resourcerefnum = OpenResFile ((*pfs).name);
		}
	
	if (resourcerefnum != -1) /*it's open*/ {
		
		UseResFile (resourcerefnum); /*in case it was already open*/
		
		*rnum = resourcerefnum;
		
		return (true);
		}
	
	error:
	
	closeresourcefile (resourcerefnum); /*checks for -1*/
	
	*rnum = -1;
	
	return (false);
	} /*openresourcefile*/


boolean closeresourcefile (short rnum) {
	
	if (rnum != -1) { /*resource file was successfully opened*/
		
		if (rnum > 2) /*not System or ROM*/
			CloseResFile (rnum);
		else
			UpdateResFile (rnum); /*make sure map is consistent*/
		
		if (ResError () != noErr)
			return (false);
		}
	
	return (true);
	} /*closeresourcefile*/
	
	
boolean getresource (short rnum, OSType type, short id, long ct, void *p) {
	
	Handle h;
	
	h = Get1Resource (type, id);
	
	if (h == nil)
		return (false);

	moveleft (*h, p, ct);
		
	return (true);
	} /*getresource*/
	
	
boolean putresource (short rnum, OSType type, short id, long ct, void *p) {

	Handle h;
	
	h = Get1Resource (type, id);
	
	if (h != nil) { /*resource already exists*/
		
		if (!sethandlecontents (p, ct, h)) /*couldn't increase size of handle*/
			return (false);
		
		ChangedResource (h);
		}
	else {
		
		if (!newfilledhandle (p, ct, &h))
			return (false);
		
		AddResource (h, type, id, "\p");
		}
	
	return (ResError () == noErr);
	} /*putresource*/
	
	
boolean getresourcehandle (short rnum, OSType type, short id, Handle *hreturned) {
	
	Handle h;
	
	*hreturned = nil;
	
	h = Get1Resource (type, id);
	
	if (h == nil)
		return (false);
		
	return (copyhandle (h, hreturned));
	} /*getresource*/
	
	
boolean putresourcehandle (short rnum, OSType type, short id, Handle h) {
	
	boolean fl;
	
	lockhandle (h);
	
	fl = putresource (rnum, type, id, GetHandleSize (h), *h);
	
	unlockhandle (h);
	
	return (fl);
	} /*putresourcehandle*/
	
	
boolean getresourcestring (short index, bigstring bs) {
	
	GetIndString (bs, 131, index);
	
	return (ResError () == noErr);
	} /*getresourcestring*/
	
	
boolean getstringlistitem (short resnum, short index, bigstring bs) {

	GetIndString (bs, resnum, index);
	
	return (ResError () == noErr);
	} /*getstringlistitem*/
	
	
boolean deleteresource (short rnum, OSType type, short id) {
	
	boolean fl = true;
	Handle h;
	
	SetResLoad (false);
	
	h = Get1Resource (type, id);
	
	SetResLoad (true);
	
	if (h != nil) { /*got the indicated resource*/
		
		RmveResource (h);
		
		fl = !ResError ();
		
		disposehandle (h);
		}
	
	return (fl);
	} /*deleteresource*/



	
	
	