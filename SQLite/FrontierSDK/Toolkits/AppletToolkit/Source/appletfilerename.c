
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"
#include "appletfilesinternal.h"
#include "appletfilemodified.h"
#include "appletfilerename.h"


boolean filerename (ptrfilespec pfs, bigstring newname) {
	
	/*
	dmb 9/21/93: don't set the file's mod date, just touch the parent 
	folder (and return true even if that part fails).
	*/
	
	OSErr ec;
	
	ec = FSpRename (pfs, newname);
	
	if (fileerror (pfs, ec)) 
		return (false);
	
	copystring (newname, (*pfs).name); /*stamp the renamed file's folder with the current time*/
	
	filetouchparentfolder (pfs);
	
	return (true);
	} /*filerename*/


