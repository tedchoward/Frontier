
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "applet.h"
#include "appletbits.h"
#include "appletfilesinternal.h"
#include "appletfileflags.h"




boolean filesetlocked (ptrfilespec pfs, boolean fllock) {
	
	OSErr ec;
	
	if (fllock) 
		ec = FSpSetFLock (pfs);
	else
		ec = FSpRstFLock (pfs);
		
	return (!fileerror (pfs, ec));
	} /*filesetlocked*/
	
	
boolean filesetfinderflag (ptrfilespec pfs, short flagnum, boolean flag) {

	OSErr ec;
	FInfo info;

	ec = FSpGetFInfo (pfs, &info);
	
	if (fileerror (pfs, ec))
		return (false);
	
	if (flag)
		info.fdFlags = setbit (info.fdFlags, flagnum);
	else
		info.fdFlags = clearbit (info.fdFlags, flagnum);
	
	ec = FSpSetFInfo (pfs, &info);	
	
	return (!fileerror (pfs, ec));
	} /*filesetfinderflag*/
	

boolean filesetsharedbit (ptrfilespec pfs, boolean flsharedbit) {
	
	return (filesetfinderflag (pfs, 6, flsharedbit));
	} /*filesetsharedbit*/
		

boolean filesetcustomiconbit (ptrfilespec pfs, boolean flcustomiconbit) {
	
	return (filesetfinderflag (pfs, 10, flcustomiconbit));
	} /*filesetcustomiconbit*/
		

boolean filesetstationery (ptrfilespec pfs, boolean flstationery) {
	
	return (filesetfinderflag (pfs, 11, flstationery));
	} /*filesetstationery*/
		

boolean filesetnamelocked (ptrfilespec pfs, boolean flnamelocked) {
	
	return (filesetfinderflag (pfs, 12, flnamelocked));
	} /*filesetnamelocked*/
		

boolean filesetbundlebit (ptrfilespec pfs, boolean flbundlebit) {
	
	return (filesetfinderflag (pfs, 13, flbundlebit));
	} /*filesetbundlebit*/
	

boolean filesetinvisible (ptrfilespec pfs, boolean flinvisible) {
	
	return (filesetfinderflag (pfs, 14, flinvisible));
	} /*filesetinvisible*/
	

boolean filesetaliasbit (ptrfilespec pfs, boolean flaliasbit) {
	
	return (filesetfinderflag (pfs, 15, flaliasbit));
	} /*filesetaliasbit*/
	
	

