
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletfiles.h"
#include "appletfilesinternal.h"
#include "appletfolder.h"
#include "appletfiledelete.h"



boolean filedeletevisit (bigstring bsitem, tyfileinfo *info, long refcon) {
	
	FSSpec fs;
	
	if (stringlength (fs.name) == 0) /*defensive driving -- if you turn this off -- it could delete C source code in the program you're testing*/
		return (false);
	
	filemakespec ((*info).vnum, (*info).dirid, bsitem, &fs);
	
	return (filedelete (&fs));
	} /*filedeletevisit*/


boolean filedelete (ptrfilespec pfs) {
	
	OSErr ec;
	
	ec = FSpDelete (pfs);
	
	if (ec == fBsyErr) { /*could be a folder with items in it*/
		
		tyfileinfo info;
		
		filegetinfo (pfs, &info);
		
		if (!info.flfolder) /*not a folder, it's an error*/
			goto error;
			
		if (!folderloop (pfs, true, &filedeletevisit, 0))
			return (false);
			
		ec = FSpDelete (pfs); /*delete the now-empty folder*/
		}
		
	if (ec == noErr)
		return (true);
	
	error:
	
	fileerror (pfs, ec);
	
	return (false);
	} /*filedelete*/
	
	
