
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletstrings.h"
#include "appletmenuops.h"
#include "appletfolder.h"
#include "appletfoldermenu.h"



static boolean foldermenucallback (bigstring fname, tyfileinfo *info, long refcon) {
	
	MenuHandle hmenu = (MenuHandle) refcon;
	
	return (pushmenuitem (hmenu, fname, true));
	} /*foldermenucallback*/
	
	
boolean getfoldermenu (ptrfilespec pfolderspec, MenuHandle *hmenu) {
	
	*hmenu = NewMenu (25000, "\px");
	
	folderloop (pfolderspec, false, &foldermenucallback, (long) *hmenu);
	
	return (true);
	} /*getfoldermenu*/
	
	
